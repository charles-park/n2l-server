//------------------------------------------------------------------------------
/**
 * @file lib_ui.c
 * @author charles-park (charles.park@hardkernel.com)
 * @brief User interface library (include parser)
 * @version 0.1
 * @date 2022-09-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
#include "../typedefs.h"
#include "../lib_fb/lib_fb.h"

//------------------------------------------------------------------------------
#include "lib_ui.h"

//------------------------------------------------------------------------------
// Function prototype.
//------------------------------------------------------------------------------
static   int  _my_strlen         (char *str);
static   int  _ui_str_scale      (int w, int h, int lw, int slen);
static   void _ui_clr_str        (fb_info_t *fb, rect_item_t *r_item, string_item_t *s_item);
static   void _ui_update_r       (fb_info_t *fb, rect_item_t *r_item);
static   void _ui_update_s       (fb_info_t *fb, string_item_t *s_item, int x, int y);
static   void _ui_parser_cmd_C   (char *buf, fb_info_t *fb, ui_grp_t *ui_grp);
static   void _ui_parser_cmd_B   (char *buf, fb_info_t *fb, ui_grp_t *ui_grp);
static   void _ui_parser_cmd_I   (char *buf, fb_info_t *fb, ui_grp_t *ui_grp);
static   void _ui_str_pos_xy     (rect_item_t *r_item, string_item_t *s_item);
static   void *_ui_find_item     (ui_grp_t *ui_grp, int fid);
static   void _ui_update         (fb_info_t *fb, ui_grp_t *ui_grp, int id);

         void ui_set_ritem       (fb_info_t *fb, ui_grp_t *ui_grp, int f_id, int bc, int lc);
         void ui_set_sitem       (fb_info_t *fb, ui_grp_t *ui_grp, int f_1d, int fc, int bc, char *str);
         void ui_set_str         (fb_info_t *fb, ui_grp_t *ui_grp,
                                    int f_id, int x, int y, int scale, int font, char *fmt, ...);
         void ui_set_printf      (fb_info_t *fb, ui_grp_t *ui_grp, int id, char *fmt, ...);
         void ui_update          (fb_info_t *fb, ui_grp_t *ui_grp, int id);
         void ui_update_group    (fb_info_t *fb, ui_grp_t *ui_grp, int gid);
         void ui_close           (ui_grp_t *ui_grp);
         ui_grp_t *ui_init       (fb_info_t *fb, const char *cfg_filename);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/*
   UI Config file 형식

   [ type ]
   '#' : commant
   'R' : Rect data
   'S' : string data
   'C' : default config data
   'L' : Line data
   'G' : Rect group data

   Rect data x, y, w, h는 fb의 비율값 (0%~100%), 모든 컬러값은 32bits rgb data.

   ui.cfg file 참조
*/

//------------------------------------------------------------------------------
static int _my_strlen(char *str)
{
   int cnt = 0, err = 512;

   /* utf-8 에서 한글표현은 3바이트 */
   while ((*str != 0x00) && err--) {
      if (*str & 0x80) {
         str += 3;   cnt += 2;
      } else {
         str += 1;   cnt++;
      }
   }
   return err ? cnt : 0;
}

//------------------------------------------------------------------------------
static int _ui_str_scale (int w, int h, int lw, int slen)
{
   int as, w_len, h_len;

   /* auto scaling */
   /* 배율이 설정되어진 최대치 보다 큰 경우 종료한다. */
   for (as = 1; as < ITEM_SCALE_MAX; as++) {
      w_len = FONT_ASCII_WIDTH * as * slen + lw * 2;
      h_len = FONT_HEIGHT      * as        + lw * 2;
      /*
         만약 배율이 1인 경우에도 화면에 표시되지 않는 경우 scale은 0값이 되고
         문자열은 화면상의 표시가 되지 않는다.
      */
      if ((w_len > w) || (h_len > h)) {
         if (as == 1)
            err("String length too big. String can't display(scale = 0).\n");
         return (as -1);
      }
   }
   return ITEM_SCALE_MAX;
}

//------------------------------------------------------------------------------
static void _ui_clr_str (fb_info_t *fb, rect_item_t *r_item, string_item_t *s_item)
{
   int color = s_item->fc.uint;

   /* 기존 String을 배경색으로 다시 그림(텍스트 지움) */
   /* string x, y 좌표 연산 */
   s_item->fc.uint = s_item->bc.uint;
   _ui_str_pos_xy(r_item, s_item);
   _ui_update_s (fb, s_item, r_item->x, r_item->y);
   s_item->fc.uint = color;
   memset (s_item->str, 0x00, ITEM_STR_MAX);
}

//------------------------------------------------------------------------------
static void _ui_update_r (fb_info_t *fb, rect_item_t *r_item)
{
   draw_fill_rect (fb, r_item->x, r_item->y, r_item->w, r_item->h,
                     r_item->bc.uint);
   if (r_item->lw)
      draw_rect (fb, r_item->x, r_item->y, r_item->w, r_item->h, r_item->lw,
                     r_item->lc.uint);
}

//------------------------------------------------------------------------------
static void _ui_update_s (fb_info_t *fb, string_item_t *s_item, int x, int y)
{
   draw_text (fb, x + s_item->x, y + s_item->y, s_item->fc.uint, s_item->bc.uint,
               s_item->scale, s_item->str);
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_C (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   char *ptr = strtok (buf, ",");

   ptr = strtok (NULL, ",");     fb->is_bgr        = (atoi(ptr) != 0) ? 1: 0;
   ptr = strtok (NULL, ",");     ui_grp->fc.uint   = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->bc.uint   = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->lc.uint   = strtol(ptr, NULL, 16);
   ptr = strtok (NULL, ",");     ui_grp->f_type    = atoi(ptr);

   set_font(ui_grp->f_type);
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_B (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   int item_cnt = ui_grp->b_item_cnt;
   char *ptr = strtok (buf, ",");
   rect_item_t   *r = &ui_grp->b_item[item_cnt].r;
   string_item_t *s = &ui_grp->b_item[item_cnt].s;

   ptr = strtok (NULL, ",");     ui_grp->b_item[item_cnt].id = atoi(ptr);
   ptr = strtok (NULL, ",");     r->x  = atoi(ptr);
   ptr = strtok (NULL, ",");     r->y  = atoi(ptr);
   ptr = strtok (NULL, ",");     r->w  = atoi(ptr);
   ptr = strtok (NULL, ",");     r->h  = atoi(ptr);
   ptr = strtok (NULL, ",");     r->lw = atoi(ptr);

   r->x = (r->x * fb->w / 100);  r->y = (r->y * fb->h / 100);
   r->w = (r->w * fb->w / 100);  r->h = (r->h * fb->h / 100);
   
   r->bc.uint = ui_grp->bc.uint;
   r->lc.uint = ui_grp->lc.uint;

   ptr = strtok (NULL, ",");     s->scale = atoi(ptr);
   ptr = strtok (NULL, ",");     ui_grp->b_item[item_cnt].s_align = atoi(ptr);

   /* 문자열이 없거나 앞부분의 공백이 있는 경우 제거 */
   if ((ptr = strtok (NULL, ",")) != NULL) {
      int slen = strlen(ptr);

      while ((*ptr == 0x20) && slen--)
         ptr++;

      s->len = slen;
      strncpy(s->str, ptr, slen-1);
      // default string for ui_reset
      strncpy(ui_grp->b_item[item_cnt].s_dfl, ptr, slen-1);
   }

   ptr = strtok (NULL, ",");
   ui_grp->b_item[item_cnt].gid = atoi(ptr);

   s->f_type  = ui_grp->f_type;   s->fc.uint = ui_grp->fc.uint;
   s->bc.uint = ui_grp->bc.uint;

   switch (ui_grp->b_item[item_cnt].s_align) {
      case STR_ALIGN_L:
      case STR_ALIGN_R:
      default :   case STR_ALIGN_C:
         s->x = -1;  s->y = -1;
      break;
   }

   item_cnt++;
   ui_grp->b_item_cnt = item_cnt;
}

//------------------------------------------------------------------------------
static void _ui_parser_cmd_I (char *buf, fb_info_t *fb, ui_grp_t *ui_grp)
{
   int item_cnt = ui_grp->i_item_cnt;
   char *ptr = strtok (buf, ",");

   if ((ptr = strtok (NULL, ",")) != NULL)
      ui_grp->i_item[item_cnt].uid = atoi(ptr);

   /* 문자열이 없거나 앞부분의 공백이 있는 경우 제거 */
   if ((ptr = strtok (NULL, ",")) != NULL) {
      int slen = strlen(ptr);

      while ((*ptr == 0x20) && slen--)
         ptr++;

      strncpy(ui_grp->i_item[item_cnt].group, ptr, slen);
   }
   
   /* 문자열이 없거나 앞부분의 공백이 있는 경우 제거 */
   if ((ptr = strtok (NULL, ",")) != NULL) {
      int slen = strlen(ptr);

      while ((*ptr == 0x20) && slen--)
         ptr++;

      strncpy(ui_grp->i_item[item_cnt].item, ptr, slen);
   }

   ui_grp->i_item[item_cnt].is_info = true;
   if ((ptr = strtok (NULL, ",")) != NULL)
      ui_grp->i_item[item_cnt].is_info = (atoi(ptr) == 1) ? true : false;

   item_cnt++;
   ui_grp->i_item_cnt = item_cnt;
}

//------------------------------------------------------------------------------
static void _ui_str_pos_xy (rect_item_t *r_item, string_item_t *s_item)
{
   int slen = _my_strlen(s_item->str);

   if (s_item->x < 0) {
      slen = slen * FONT_ASCII_WIDTH * s_item->scale;
      s_item->x = ((r_item->w - slen) / 2);
   }
   if (s_item->y < 0)
      s_item->y = ((r_item->h - FONT_HEIGHT * s_item->scale)) / 2;
}

//------------------------------------------------------------------------------
static void *_ui_find_item (ui_grp_t *ui_grp, int fid)
{
   int i;
   for (i = 0; i < ui_grp->b_item_cnt; i++)
      if (fid == ui_grp->b_item[i].id)
         return &ui_grp->b_item[i];
   return NULL;
}

//------------------------------------------------------------------------------
void ui_set_ritem (fb_info_t *fb, ui_grp_t *ui_grp, int f_id, int bc, int lc)
{
   b_item_t *pitem = _ui_find_item(ui_grp, f_id);

   if ((f_id < ITEM_COUNT_MAX) && (pitem != NULL)) {
      if (bc != -1)  pitem->r.bc.uint = bc;   
      if (lc != -1)  pitem->r.lc.uint = lc;
      ui_set_sitem (fb, ui_grp, f_id, -1, bc, NULL);
      ui_update (fb, ui_grp, f_id);
   }
}

//------------------------------------------------------------------------------
void ui_set_sitem (fb_info_t *fb, ui_grp_t *ui_grp, int f_id, int fc, int bc, char *str)
{
   b_item_t *pitem = _ui_find_item(ui_grp, f_id);

   if ((f_id < ITEM_COUNT_MAX) && (pitem != NULL)) {
      /* font color 변경 */
      if (fc != -1)
         pitem->s.fc.uint = fc;
      if (bc != -1)
         pitem->s.bc.uint = bc;

      /* 받아온 string을 buf에 저장 */
      if (str != NULL)  {
         char buf[ITEM_STR_MAX];

         memset (buf, 0x00, sizeof(buf));
         sprintf(buf, "%s", str);
         /*
         기존 문자열 보다 새로운 문자열이 더 작은 경우
         기존 문자열을 배경색으로 덮어 씌운다.
         */
         if ((strlen(pitem->s.str) > strlen(buf)))
            _ui_clr_str (fb, &pitem->r, &pitem->s);

         /* 새로운 string 복사 */
         strncpy(pitem->s.str, buf, strlen(buf));
      }
      _ui_str_pos_xy(&pitem->r, &pitem->s);
      _ui_update_s (fb, &pitem->s, pitem->r.x, pitem->r.y);
   }
}

//------------------------------------------------------------------------------
void ui_set_str (fb_info_t *fb, ui_grp_t *ui_grp,
                  int f_id, int x, int y, int scale, int font, char *fmt, ...)
{
   b_item_t *pitem = _ui_find_item (ui_grp, f_id);

   if ((f_id < ITEM_COUNT_MAX) && (pitem != NULL)) {

      va_list va;
      char buf[ITEM_STR_MAX];
      int n_scale = pitem->s.scale;

      /* 받아온 가변인자를 string 형태로 변환 하여 buf에 저장 */
      memset(buf, 0x00, sizeof(buf));
      va_start(va, fmt);   vsprintf(buf, fmt, va); va_end(va);

      if (scale) {
         /* scale = -1 이면 최대 스케일을 구하여 표시한다 */
         if (scale < 0)
            n_scale = _ui_str_scale (pitem->r.w, pitem->r.h, pitem->r.lw,
                                       _my_strlen(buf));
         else
            n_scale = scale;

         if (pitem->s.scale > n_scale)
            _ui_clr_str (fb, &pitem->r, &pitem->s);
      }

      if (font) {
      pitem->s.f_type = (font < 0) ? ui_grp->f_type : font;
      set_font(pitem->s.f_type);
      }

      /*
      기존 문자열 보다 새로운 문자열이 더 작은 경우
      기존 문자열을 배경색으로 덮어 씌운다.
      */
      if ((strlen(pitem->s.str) > strlen(buf)) || n_scale != pitem->s.scale) {
      _ui_clr_str (fb, &pitem->r, &pitem->s);
      pitem->s.scale = n_scale;
      }
      pitem->s.x = (x != 0) ? x : pitem->s.x;
      pitem->s.y = (y != 0) ? y : pitem->s.y;

      /* 새로운 string 복사 */
      strncpy(pitem->s.str, buf, strlen(buf));

      _ui_str_pos_xy(&pitem->r, &pitem->s);
      _ui_update_s (fb, &pitem->s, pitem->r.x, pitem->r.y);
   }
}

//------------------------------------------------------------------------------
static void _ui_update (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   b_item_t *pitem = _ui_find_item(ui_grp, id); 

   if ((id < ITEM_COUNT_MAX) && (pitem != NULL)) {
      pitem->s.f_type = ui_grp->f_type;

      if ((signed)pitem->s.bc.uint < 0)
         pitem->s.bc.uint = pitem->r.bc.uint;

      set_font(pitem->s.f_type);

      if (pitem->s.scale < 0)
         pitem->s.scale = _ui_str_scale (pitem->r.w, pitem->r.h, pitem->r.lw,
                                       _my_strlen(pitem->s.str));

      _ui_str_pos_xy(&pitem->r, &pitem->s);
      _ui_update_r (fb, &pitem->r);
      _ui_update_s (fb, &pitem->s, pitem->r.x, pitem->r.y);
   }
}

//------------------------------------------------------------------------------
void ui_set_printf (fb_info_t *fb, ui_grp_t *ui_grp, int id, char *fmt, ...)
{
   va_list va;
   char buf[ITEM_STR_MAX];

   /* 받아온 가변인자를 string 형태로 변환 하여 buf에 저장 */
   memset(buf, 0x00, sizeof(buf));
   va_start(va, fmt);   vsprintf(buf, fmt, va); va_end(va);

   ui_set_str (fb, ui_grp, id, -1, -1, -1, -1, buf);
}

//------------------------------------------------------------------------------
void ui_update (fb_info_t *fb, ui_grp_t *ui_grp, int id)
{
   int i, sid, fid;
   r_item_t *r_item;

   /* ui_grp에 등록되어있는 모든 item에 대하여 화면 업데이트 함 */
   if (id < 0) {
      /* 초기화 값으로 전환 */
      for (i = 0; i < ui_grp->b_item_cnt; i++) {
         ui_grp->b_item[i].r.bc.uint = ui_grp->bc.uint;
         ui_grp->b_item[i].r.lc.uint = ui_grp->lc.uint;
         ui_grp->b_item[i].s.bc.uint = ui_grp->bc.uint;
         ui_grp->b_item[i].s.fc.uint = ui_grp->fc.uint;
         memset  (ui_grp->b_item[i].s.str, 0x00, ITEM_STR_MAX);
         strncpy (ui_grp->b_item[i].s.str, ui_grp->b_item[i].s_dfl, strlen (ui_grp->b_item[i].s_dfl));
      }
      /* 모든 item에 대한 화면 업데이트 */
      for (i = 0; i < ITEM_COUNT_MAX; i++)
         _ui_update (fb, ui_grp, i);
   }
   else
      /* id값으로 설정된 1 개의 item에 대한 화면 업데이트 */
      _ui_update (fb, ui_grp, id);
}

//------------------------------------------------------------------------------
void ui_update_group (fb_info_t *fb, ui_grp_t *ui_grp, int gid)
{
   int i;
   for (i = 0; i < ui_grp->b_item_cnt; i++) {
      if (ui_grp->b_item[i].gid == gid)
         _ui_update (fb, ui_grp, ui_grp->b_item[i].id);
   }
}

//------------------------------------------------------------------------------
void ui_close (ui_grp_t *ui_grp)
{
   /* 할당받은 메모리가 있다면 시스템으로 반환한다. */
   if (ui_grp)
      free (ui_grp);
}

//------------------------------------------------------------------------------
ui_grp_t *ui_init (fb_info_t *fb, const char *cfg_filename)
{
   ui_grp_t	*ui_grp;
   FILE *pfd;
   char buf[256], r_cnt = 0, s_cnt = 0, *ptr, is_cfg_file = 0;

   if ((pfd = fopen(cfg_filename, "r")) == NULL)
      return   NULL;

	if ((ui_grp = (ui_grp_t *)malloc(sizeof(ui_grp_t))) == NULL)
      return   NULL;

   memset (ui_grp, 0x00, sizeof(ui_grp_t));
   memset (buf,    0x00, sizeof(buf));

   while(fgets(buf, sizeof(buf), pfd) != NULL) {
      if (!is_cfg_file) {
         is_cfg_file = strncmp ("ODROID-UI-CONFIG", buf, strlen(buf)-1) == 0 ? 1 : 0;
         memset (buf, 0x00, sizeof(buf));
         continue;
      }
      switch(buf[0]) {
         case  'C':  _ui_parser_cmd_C (buf, fb, ui_grp); break;
         case  'B':  _ui_parser_cmd_B (buf, fb, ui_grp); break;
         case  'I':  _ui_parser_cmd_I (buf, fb, ui_grp); break;
         default :
            err("Unknown parser command! cmd = %c\n", buf[0]);
         case  '#':  case  '\n':
         break;
      }
      memset (buf, 0x00, sizeof(buf));
   }

   if (!is_cfg_file) {
      err("UI Config File not found! (filename = %s)\n", cfg_filename);
      free (ui_grp);
      return NULL;
   }

   /* all item update */
   if (ui_grp->b_item_cnt)
      ui_update (fb, ui_grp, -1);

   if (pfd)
      fclose (pfd);

	// file parser
	return	ui_grp;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
