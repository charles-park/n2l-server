//------------------------------------------------------------------------------
/**
 * @file lib_adc.h
 * @author charles-park (charles.park@hardkernel.com)
 * @brief User interface library (include parser)
 * @version 0.1
 * @date 2022-10-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
//------------------------------------------------------------------------------
#ifndef __LIB_ADC_H__
#define __LIB_ADC_H__

//------------------------------------------------------------------------------
extern	bool	adc_read_pin 	(int fd, const char *name, unsigned int *read_value, unsigned int *cnt);
extern  int     adc_board_init  (const char *i2c_fname);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#endif  // __LIB_ADC_H__
//------------------------------------------------------------------------------
