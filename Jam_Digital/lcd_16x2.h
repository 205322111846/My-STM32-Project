/*
 * lcd_16x2.h
 *
 *  Created on: Dec 22, 2020
 *      Author: Dell
 */

#ifndef INC_LCD_16X2_H_
#define INC_LCD_16X2_H_
void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_put_cur(int row, int col);  // put cursor at the entered position row (0 or 1), col (0-15);

void lcd_clear (void);




#endif /* INC_LCD_16X2_H_ */
