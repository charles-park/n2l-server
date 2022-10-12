//------------------------------------------------------------------------------
/**
 * @file typedefs.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief 자주 사용되는 typedef 정의 모음.
 * @version 0.1
 * @date 2022-05-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

//------------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
// #define	dbg(fmt, args...)
// #define	err(fmt, args...)
#define	dbg(fmt, args...)	fprintf(stdout,"[DBG] %s(%d) : " fmt, __func__, __LINE__, ##args)
#define	err(fmt, args...)	fprintf(stderr,"[ERR] %s (%s - %d)] : " fmt, __FILE__, __func__, __LINE__, ##args)
#define	info(fmt, args...)	fprintf(stdout,"[INFO] : " fmt, ##args)

//------------------------------------------------------------------------------
typedef unsigned char   __u8;
typedef unsigned short  __u16;
typedef unsigned int    __u32;
typedef unsigned long   __ul32;

typedef signed char     __s8;
typedef signed short    __s16;
typedef signed int      __s32;
typedef signed long     __sl32;

typedef enum {false, true}  bool;

//------------------------------------------------------------------------------
typedef struct bit8__t {
    __u8    b0  :1;
    __u8    b1  :1;
    __u8    b2  :1;
    __u8    b3  :1;
    __u8    b4  :1;
    __u8    b5  :1;
    __u8    b6  :1;
    __u8    b7  :1;
}   bit8_t;

typedef union bit8__u {
    __u8    uc;
    bit8_t  bits;
}   bit8_u;

//------------------------------------------------------------------------------
typedef struct bit16__t {
    __u16   b0  :1;
    __u16   b1  :1;
    __u16   b2  :1;
    __u16   b3  :1;
    __u16   b4  :1;
    __u16   b5  :1;
    __u16   b6  :1;
    __u16   b7  :1;

    __u16   b8  :1;
    __u16   b9  :1;
    __u16   b10 :1;
    __u16   b11 :1;
    __u16   b12 :1;
    __u16   b13 :1;
    __u16   b14 :1;
    __u16   b15 :1;
}   bit16_t;

typedef union bit16__u {
    __u8        u8[2];
    __u16       u16;
    bit16_t     bits;
}   bit16_u;

//------------------------------------------------------------------------------
typedef struct bit32__t {
    __u32   b0  :1;
    __u32   b1  :1;
    __u32   b2  :1;
    __u32   b3  :1;
    __u32   b4  :1;
    __u32   b5  :1;
    __u32   b6  :1;
    __u32   b7  :1;

    __u32   b8  :1;
    __u32   b9  :1;
    __u32   b10 :1;
    __u32   b11 :1;
    __u32   b12 :1;
    __u32   b13 :1;
    __u32   b14 :1;
    __u32   b15 :1;

    __u32   b16 :1;
    __u32   b17 :1;
    __u32   b18 :1;
    __u32   b19 :1;
    __u32   b20 :1;
    __u32   b21 :1;
    __u32   b22 :1;
    __u32   b23 :1;

    __u32   b24 :1;
    __u32   b25 :1;
    __u32   b26 :1;
    __u32   b27 :1;
    __u32   b28 :1;
    __u32   b29 :1;
    __u32   b30 :1;
    __u32   b31 :1;
}   bit32_t;

typedef union bit32__u {
    __u8        u8[4];
    __u16       u16[2];
    __u32       ui;
    __ul32      ul;
    bit32_t     bits;
}   bit32_u;

//------------------------------------------------------------------------------
#endif  // #define __TYPEDEFS_H__

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
