//------------------------------------------------------------------------------------------------------------
//
// I2C Library
//
// 2022.03.16 (charles.park@hardkernel.com)
//
//------------------------------------------------------------------------------------------------------------
#ifndef __I2C_H__
#define __I2C_H__

//------------------------------------------------------------------------------------------------------------
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

//------------------------------------------------------------------------------------------------------------
extern int i2c_read        (int fd);
extern int i2c_read_byte   (int fd, int reg);
extern int i2c_read_word   (int fd, int reg);
extern int i2c_write       (int fd, int data);
extern int i2c_write_byte  (int fd, int reg, int value);
extern int i2c_write_word  (int fd, int reg, int value);
extern int i2c_set_addr    (int fd, int device_addr);
extern int i2c_open_device (const char *device_node, int device_addr);
extern int i2c_close       (int fd);
extern int i2c_open        (const char *device_node);

//------------------------------------------------------------------------------------------------------------
#endif  // __I2C_H__
//------------------------------------------------------------------------------------------------------------
