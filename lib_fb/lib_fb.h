//-----------------------------------------------------------------------------
/**
 * @file lib_fb.h
 * @author charles-park (charles-park@hardkernel.com)
 * @brief framebuffer library header file.
 * @version 0.1
 * @date 2022-05-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef __LIB_FB_H__
#define __LIB_FB_H__

//-----------------------------------------------------------------------------
// Color table & convert macro
//-----------------------------------------------------------------------------
#define RGB_TO_UINT(r,g,b)  (((r << 16) | (g << 8) | b) & 0xFFFFFF)
#define UINT_TO_R(i)        ((i >> 16) & 0xFF)
#define UINT_TO_G(i)        ((i >>  8) & 0xFF)
#define UINT_TO_B(i)        ((i      ) & 0xFF)

/*
    https://www.rapidtables.com/web/color/RGB_Color.html
*/
#define COLOR_BLACK         RGB_TO_UINT(0, 0, 0)
#define COLOR_WHITE         RGB_TO_UINT(255,255,255)
#define COLOR_RED	        RGB_TO_UINT(255,0,0)
#define COLOR_LIME	        RGB_TO_UINT(0,255,0)
#define COLOR_BLUE	        RGB_TO_UINT(0,0,255)
#define COLOR_YELLOW        RGB_TO_UINT(255,255,0)
#define COLOR_GREEN	        RGB_TO_UINT(0,128,0)
#define COLOR_CYAN          RGB_TO_UINT(0,255,255)
#define COLOR_MAGENTA       RGB_TO_UINT(255,0,255)
#define COLOR_SILVER        RGB_TO_UINT(192,192,192)
#define COLOR_DIM_GRAY      RGB_TO_UINT(105,105,105)
#define COLOR_GRAY          RGB_TO_UINT(128,128,128)
#define COLOR_DARK_GRAY     RGB_TO_UINT(169,169,169)
#define COLOR_LIGHT_GRAY    RGB_TO_UINT(211,211,211)
#define COLOR_MAROON        RGB_TO_UINT(128,0,0)
#define COLOR_OLIVE         RGB_TO_UINT(128,128,0)
#define COLOR_PURPLE        RGB_TO_UINT(128,0,128)
#define COLOR_TEAL          RGB_TO_UINT(0,128,128)
#define COLOR_NAVY          RGB_TO_UINT(0,0,128)

//-----------------------------------------------------------------------------
// Frame buffer struct
//-----------------------------------------------------------------------------
typedef union fb_color__u {
    struct {
        unsigned int    b:8;    // lsb
        unsigned int    g:8;
        unsigned int    r:8;
        unsigned int    a:8;
    } bits;
    unsigned int uint;
}	fb_color_u;

typedef struct fb_info__t {
	int			fd;
	int			w;
	int			h;
	int			stride;
	int			bpp;
	bool		is_bgr;
	char		*base;
	char		*data;
}	fb_info_t;

//-----------------------------------------------------------------------------
#define FONT_HANGUL_WIDTH   16
#define FONT_ASCII_WIDTH    8
#define FONT_HEIGHT         16

enum eFONTS_HANGUL {
    eFONT_HAN_DEFAULT = 0,
    eFONT_HANBOOT,
    eFONT_HANGODIC,
    eFONT_HANPIL,
    eFONT_HANSOFT,
    eFONT_END
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern void         put_pixel 	(fb_info_t *fb, int x, int y, int color);
extern void         draw_text 	(fb_info_t *fb, int x, int y,
									int f_color, int b_color, int scale, char *fmt, ...);
extern void         draw_line 	(fb_info_t *fb, int x, int y, int w, int color);
extern void         draw_rect 	(fb_info_t *fb, int x, int y, int w, int h, int lw, int color);
extern void         draw_fill_rect (fb_info_t *fb, int x, int y, int w, int h, int color);
extern void         set_font	(enum eFONTS_HANGUL s_font);
extern void         fb_clear 	(fb_info_t *fb);
extern void         fb_close 	(fb_info_t *fb);
extern fb_info_t    *fb_init 	(const char *DEVICE_NAME);

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
#endif  // #define __LIB_FB_H__
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------
