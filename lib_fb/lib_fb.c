//-----------------------------------------------------------------------------
/**
 * @file lib_fb.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief Framebuffer control library
 * @version 0.1
 * @date 2022-05-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>

#include "../typedefs.h"
#include "lib_fb.h"
//-----------------------------------------------------------------------------
// Fonts
//-----------------------------------------------------------------------------
#include "fonts/FontHangul.h"
#include "fonts/FontHansoft.h"
#include "fonts/FontHanboot.h"
#include "fonts/FontHangodic.h"
#include "fonts/FontHanpil.h"
#include "fonts/FontHangodic.h"
#include "fonts/FontAscii_8x16.h"


//-----------------------------------------------------------------------------
// Function prototype define.
//-----------------------------------------------------------------------------
static void make_image  (unsigned char is_first,
                        unsigned char *dest,
                        unsigned char *src);
static unsigned char *get_hangul_image( unsigned char HAN1,
                                        unsigned char HAN2,
                                        unsigned char HAN3);
static void draw_hangul_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale);
static void draw_ascii_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale);
static void _draw_text (fb_info_t *fb, int x, int y, char *p_str,
                        int f_color, int b_color, int scale);
void         put_pixel (fb_info_t *fb, int x, int y, int color);
void         draw_text (fb_info_t *fb, int x, int y,
                     int f_color, int b_color, int scale, char *fmt, ...);
void         draw_line (fb_info_t *fb, int x, int y, int w, int color);
void         draw_rect (fb_info_t *fb, int x, int y, int w, int h, int lw, int color);
void         draw_fill_rect (fb_info_t *fb, int x, int y, int w, int h, int color);
void         set_font(enum eFONTS_HANGUL s_font);
void         fb_clear (fb_info_t *fb);
void         fb_close (fb_info_t *fb);
fb_info_t    *fb_init (const char *DEVICE_NAME);

//-----------------------------------------------------------------------------
// hangul image base 16x16
//-----------------------------------------------------------------------------
static unsigned char HANFontImage[32] = {0,};

const char D_ML[22] = { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 																	};
const char D_FM[40] = { 1, 3, 0, 2, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 1, 3, 0, 2, 1, 3, 1, 3, 1, 3 			};
const char D_MF[44] = { 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 1, 6, 3, 7, 3, 7, 3, 7, 1, 6, 2, 6, 4, 7, 4, 7, 4, 7, 2, 6, 1, 6, 3, 7, 0, 5 };

static unsigned char *HANFONT1 = (unsigned char *)FONT_HANGUL1;
static unsigned char *HANFONT2 = (unsigned char *)FONT_HANGUL2;
static unsigned char *HANFONT3 = (unsigned char *)FONT_HANGUL3;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void make_image  (unsigned char is_first,
                        unsigned char *dest,
                        unsigned char *src)
{
    int i;
    if (is_first)   for (i = 0; i < 32; i++)    dest[i]  = src[i];
    else            for (i = 0; i < 32; i++)    dest[i] |= src[i];
}

//-----------------------------------------------------------------------------
static unsigned char *get_hangul_image( unsigned char HAN1,
                                        unsigned char HAN2,
                                        unsigned char HAN3)
{
    unsigned char f, m, l;
    unsigned char f1, f2, f3;
    unsigned char first_flag = 1;
    unsigned short utf16 = 0;

    /*------------------------------
    UTF-8 을 UTF-16으로 변환한다.

    UTF-8 1110xxxx 10xxxxxx 10xxxxxx
    ------------------------------*/
    utf16 = ((unsigned short)HAN1 & 0x000f) << 12 |
            ((unsigned short)HAN2 & 0x003f) << 6  |
            ((unsigned short)HAN3 & 0x003f);
    utf16 -= 0xAC00;

    /* 초성 / 중성 / 종성 분리 */
    l = (utf16 % 28);
    utf16 /= 28;
    m = (utf16 % 21) +1;
    f = (utf16 / 21) +1;

    /* 초성 / 중성 / 종성 형태에 따른 이미지 선택 */
    f3 = D_ML[m];
    f2 = D_FM[(f * 2) + (l != 0)];
    f1 = D_MF[(m * 2) + (l != 0)];

    memset(HANFontImage, 0, sizeof(HANFontImage));
    if (f)  {   make_image(         1, HANFontImage, HANFONT1 + (f1*16 + f1 *4 + f) * 32);    first_flag = 0; }
    if (m)  {   make_image(first_flag, HANFontImage, HANFONT2 + (        f2*22 + m) * 32);    first_flag = 0; }
    if (l)  {   make_image(first_flag, HANFontImage, HANFONT3 + (f3*32 - f3 *4 + l) * 32);    first_flag = 0; }

    return HANFontImage;
}

//-----------------------------------------------------------------------------
void put_pixel (fb_info_t *fb, int x, int y, int color)
{
    fb_color_u c;
    int offset = (y * fb->stride) + (x * (fb->bpp >> 3));

    if ((x < fb->w) && (y < fb->h)) {
        c.uint = color;
        if (fb->is_bgr) {
            *(fb->data + offset) = c.bits.b;  offset++;
            *(fb->data + offset) = c.bits.g;  offset++;
            *(fb->data + offset) = c.bits.r;  offset++;
        } else {
            *(fb->data + offset) = c.bits.r;  offset++;
            *(fb->data + offset) = c.bits.g;  offset++;
            *(fb->data + offset) = c.bits.b;  offset++;
        }
        if (fb->bpp == 32)
            *(fb->data + offset) = 0xFF;
    } else {
        dbg("Out of range.(width = %d, x = %d, height = %d, y = %d)\n", 
            fb->w, x, fb->h, y);
    }
}

//-----------------------------------------------------------------------------
static void draw_hangul_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale)
{
    int pos, i, j, mask, x_off, y_off, scale_y, scale_x;

    for (i = 0, y_off = 0, pos = 0; i < 16; i++) {
        for (scale_y = 0; scale_y < scale; scale_y++) {
            if (scale_y)
                pos -= 2;
            for (x_off = 0, j = 0; j < 2; j++) {
                for (mask = 0x80; mask > 0; mask >>= 1) {
                    for (scale_x = 0; scale_x < scale; scale_x++) {
                        int c;
                        c = (p_img[pos] & mask) ? f_color : b_color;

                        put_pixel(fb, x + x_off, y + y_off, c);
                        x_off++;
                    }
                }
                pos++;
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void draw_ascii_bitmap (fb_info_t *fb,
                    int x, int y, unsigned char *p_img,
                    int f_color, int b_color, int scale)
{
    int pos, mask, x_off, y_off, scale_y, scale_x;

    for (pos = 0, y_off = 0; pos < 16; pos++) {
        for (scale_y = 0; scale_y < scale; scale_y++) {
            for (x_off = 0, mask = 0x80; mask > 0; mask >>= 1) {
                for (scale_x = 0; scale_x < scale; scale_x++) {
                    int c;
                    c = (p_img[pos] & mask) ? f_color : b_color;

                    put_pixel(fb, x + x_off, y + y_off, c);
                    x_off++;
                }
            }
            y_off++;
        }
    }
}

//-----------------------------------------------------------------------------
static void _draw_text (fb_info_t *fb, int x, int y, char *p_str,
                        int f_color, int b_color, int scale)
{
    unsigned char *p_img;
    unsigned char c1, c2, c3;

    while(*p_str) { 
        c1 = *(unsigned char *)p_str++;

        //---------- 한글 ---------
        /* 모든 문자는 기본적으로 UTF-8형태로 저장되며 한글은 3바이트를 가진다. */
        /* 한글은 3바이트를 일어 UTF8 to UTF16으로 변환후 초/중/종성을 분리하여 조합형으로 표시한다. */
        if (c1 >= 0x80){
            c2 = *(unsigned char *)p_str++;
            c3 = *(unsigned char *)p_str++;

            p_img = get_hangul_image(c1, c2, c3);
            draw_hangul_bitmap(fb, x, y, p_img, f_color, b_color, scale);
            x = x + FONT_HANGUL_WIDTH * scale;
        }
        //---------- ASCII ---------
        else {
            p_img = (unsigned char *)FONT_ASCII[c1];
            draw_ascii_bitmap(fb, x, y, p_img, f_color, b_color, scale);
            x = x + FONT_ASCII_WIDTH * scale;
        }
    }  
}

//-----------------------------------------------------------------------------
void draw_text (fb_info_t *fb, int x, int y,
                int f_color, int b_color, int scale, char *fmt, ...)
{
    char buf[256];
    va_list va;

    memset(buf, 0x00, sizeof(buf));

    va_start(va, fmt);
    vsprintf(buf, fmt, va);
    va_end(va);

    _draw_text(fb, x, y, buf, f_color, b_color, scale);
}

//-----------------------------------------------------------------------------
void draw_line (fb_info_t *fb, int x, int y, int w, int color)
{
    int dx;

    for (dx = 0; dx < w; dx++)
        put_pixel(fb, x + dx, y, color);
}

//-----------------------------------------------------------------------------
void draw_rect (fb_info_t *fb, int x, int y, int w, int h, int lw, int color)
{
	int dy, i;

	for (dy = 0; dy < h; dy++) {
        if (dy < lw || (dy > (h - lw -1)))
            draw_line (fb, x, y + dy, w, color);
        else {
            for (i = 0; i < lw; i++) {
                put_pixel (fb, x + 0    +i, y + dy, color);
                put_pixel (fb, x + w -1 -i, y + dy, color);
            }
        }
	}
}

//-----------------------------------------------------------------------------
void draw_fill_rect (fb_info_t *fb, int x, int y, int w, int h, int color)
{
	int dy;

	for (dy = 0; dy < h; dy++)
        draw_line(fb, x, y + dy, w, color);
}

//-----------------------------------------------------------------------------
void set_font(enum eFONTS_HANGUL s_font)
{
    switch(s_font)
    {
        case    eFONT_HANBOOT:
            HANFONT1 = (unsigned char *)FONT_HANBOOT1;
            HANFONT2 = (unsigned char *)FONT_HANBOOT2;
            HANFONT3 = (unsigned char *)FONT_HANBOOT3;
        break;
        case    eFONT_HANGODIC:
            HANFONT1 = (unsigned char *)FONT_HANGODIC1;
            HANFONT2 = (unsigned char *)FONT_HANGODIC2;
            HANFONT3 = (unsigned char *)FONT_HANGODIC3;
        break;
        case    eFONT_HANPIL:
            HANFONT1 = (unsigned char *)FONT_HANPIL1;
            HANFONT2 = (unsigned char *)FONT_HANPIL2;
            HANFONT3 = (unsigned char *)FONT_HANPIL3;
        break;
        case    eFONT_HANSOFT:
            HANFONT1 = (unsigned char *)FONT_HANSOFT1;
            HANFONT2 = (unsigned char *)FONT_HANSOFT2;
            HANFONT3 = (unsigned char *)FONT_HANSOFT3;
        break;
        case    eFONT_HAN_DEFAULT:
        default :
            HANFONT1 = (unsigned char *)FONT_HANGUL1;
            HANFONT2 = (unsigned char *)FONT_HANGUL2;
            HANFONT3 = (unsigned char *)FONT_HANGUL3;
        break;
    }
}

//-----------------------------------------------------------------------------
void fb_clear (fb_info_t *fb)
{
    memset(fb->data, 0x00, (fb->w * fb->h * fb->bpp) / 8);
}

//-----------------------------------------------------------------------------
void fb_close (fb_info_t *fb)
{
    if (fb) {
        if (fb->fd)
            close (fb->fd);
        free (fb);
    }
}

//-----------------------------------------------------------------------------
#define FB_CURSOR_CONTROL   "/sys/class/graphics/fbcon/cursor_blink"

void fb_cursor (bool status)
{
    FILE *fp = fopen (FB_CURSOR_CONTROL, "w");

    if (fp != NULL) {
        fputc (status ? '1' : '0', fp);
        fclose (fp);
    }
}

//-----------------------------------------------------------------------------
fb_info_t *fb_init (const char *DEVICE_NAME)
{
	struct fb_var_screeninfo fvsi;
	struct fb_fix_screeninfo ffsi;
    
    fb_info_t   *fb = (fb_info_t *)malloc(sizeof(fb_info_t));

    if (fb == NULL) {
        err("framebuffer malloc error!\n");
        return NULL;
    }
	memset(fb, 0, sizeof(fb_info_t));

	if ((fb->fd = open(DEVICE_NAME, O_RDWR)) < 0) {
		err("open");
        return NULL;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fvsi) < 0) {
		err("ioctl(FBIOGET_VSCREENINFO)");
        goto out;
	}
	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &ffsi) < 0) {
		err("ioctl(FBIOGET_FSCREENINFO)");
        goto out;
	}

	fb->w       = fvsi.xres;
	fb->h       = fvsi.yres;
	fb->bpp     = fvsi.bits_per_pixel;
	fb->stride  = ffsi.line_length;

    info ("[ %s : %s ] fb_x_res : %d, fb_y_res : %d\n",
            __FILE__, __func__, fb->w, fb->h);

	if (fvsi.red.length != 8 || fvsi.green.length != 8 || fvsi.blue.length != 8) {
		err("mmap");
        goto out;
	}

	fb->base = (char *)mmap((caddr_t) NULL, ffsi.smem_len,
                        PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

	if (fb->base == (char *)-1) {
		err("mmap");
        goto out;
	}

    fb->data = fb->base + ((unsigned long) ffsi.smem_start % (unsigned long) getpagesize());
    fb_cursor(false);
    fb_clear (fb);
    return  fb;
out:
    fb_close(fb);
    return  NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
