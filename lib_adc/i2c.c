//------------------------------------------------------------------------------------------------------------
//
// I2C Library
//
// 2022.03.16 (charles.park@hardkernel.com)
//
//------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "i2c.h"

//------------------------------------------------------------------------------------------------------------
int i2c_read        (int fd);
int i2c_read_byte   (int fd, int reg);
int i2c_read_word   (int fd, int reg);
int i2c_read_block  (int fd, int reg, unsigned char *buff, int buff_size);
int i2c_write       (int fd, int data);
int i2c_write_byte  (int fd, int reg, int value);
int i2c_write_word  (int fd, int reg, int value);
int i2c_write_block (int fd, int reg, unsigned char *buff, int buff_size);
int i2c_device_check(int fd, int device_addr);
int i2c_open_device (const char *device_node, int device_addr);
int i2c_close       (int fd);
int i2c_open        (const char *device_node);

//------------------------------------------------------------------------------------------------------------
static inline int i2c_smbus_access (int fd, char rw, uint8_t command, int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args ;

	args.read_write = rw ;
	args.command    = command ;
	args.size       = size ;
	args.data       = data ;
	return ioctl (fd, I2C_SMBUS, &args) ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_read (int fd)
{
    union i2c_smbus_data data ;

    if (i2c_smbus_access (fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data))
        return -1 ;
    else
        return data.byte & 0xFF ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_read_byte (int fd, int reg)
{
    union i2c_smbus_data data;

    if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data))
        return -1 ;
    else
        return data.byte & 0xFF ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_read_word (int fd, int reg)
{
    union i2c_smbus_data data;

    if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_WORD_DATA, &data))
        return -1 ;
    else
        return data.word & 0xFFFF ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_write (int fd, int data)
{
    return i2c_smbus_access (fd, I2C_SMBUS_WRITE, data, I2C_SMBUS_BYTE, NULL) ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_write_byte (int fd, int reg, int value)
{
    union i2c_smbus_data data ;

    data.byte = value ;
    return i2c_smbus_access (fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, &data) ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_write_word (int fd, int reg, int value)
{
  union i2c_smbus_data data ;

  data.word = value ;
  return i2c_smbus_access (fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_WORD_DATA, &data) ;
}

//------------------------------------------------------------------------------------------------------------
int i2c_set_addr (int fd, int device_addr)
{
    usleep(1000);
    if(ioctl (fd, I2C_SLAVE, device_addr) < 0)  {
        fprintf (stderr, "Can't setup device : device adddr is 0x%02x\n", device_addr);
        return -1;
    }
	return 0;
}

//------------------------------------------------------------------------------------------------------------
int i2c_open_device (const char *device_node, int device_addr)
{
    int fd;

    if ((fd = open (device_node, O_RDWR)) < 0)
        return -1;

    if (i2c_set_addr (fd, device_addr)) {
        i2c_close (fd);
        return -1;
    }

    return fd;
}

//------------------------------------------------------------------------------------------------------------
int i2c_close (int fd)
{
    if (fd)
        close (fd);

    return 0;
}

//------------------------------------------------------------------------------------------------------------
int i2c_open (const char *device_node)
{
	int fd;
	if ((fd = open (device_node, O_RDWR)) < 0) {
        fprintf (stderr, "Unable to open I2C device : %s\n", strerror(errno));
        return -1;
	}
    return fd;
}

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------



