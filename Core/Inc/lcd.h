/*
 * lcd.h
 *
 *  Created on: 07.06.2014
 *      Author: Florian
 *  Modified by: Jeremy Overesch (10/29/2025) for STM32H750
 */


#ifndef LCD_H_
#define LCD_H_

/*---------------------------------------------------------------------*
 *  additional includes                                                *
 *---------------------------------------------------------------------*/
#include <stdbool.h>
/*---------------------------------------------------------------------*
 *  global definitions                                                 *
 *---------------------------------------------------------------------*/

/* rotate display by 180 degrees */
//#define	LCD_UPSIDE_DOWN

#define LCD_MAX_X					800	// LCD width (NHD-7.0-800480EF-ASXV)
#define LCD_MAX_Y					480 // LCD height (NHD-7.0-800480EF-ASXV)

/*---------------------------------------------------------------------*
 *  type declarations                                                  *
 *---------------------------------------------------------------------*/

typedef enum
{
	LCD_BACKGROUND,
	LCD_FOREGROUND
} lcd_layers_t;

/*---------------------------------------------------------------------*
 *  function prototypes                                                *
 *---------------------------------------------------------------------*/

void lcd_init (void);

void lcd_set_layer (lcd_layers_t layer);

void lcd_refresh (void);

void lcd_set_transparency (lcd_layers_t layer, uint8_t transparency);

/*---------------------------------------------------------------------*
 *  global data                                                        *
 *---------------------------------------------------------------------*/
extern uint32_t lcd_frame_buffer;

extern lcd_layers_t lcd_layer;

extern bool lcd_vsync;

/*---------------------------------------------------------------------*
 *  inline functions and function-like macros                          *
 *---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*
 *  eof                                                                *
 *---------------------------------------------------------------------*/

#endif /* LCD_H_ */
