
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 2-10-2018
 
 * 
 */

 #include "stanok.h"


int  init_mcp23017_chip_tft();
void test();
void display_test();
struct MCP23017_PIN *Lampa = &mcp_gpio[0];


//////TESTY----------



int main(int argc, char** argv) {
	
MCP23017 *chip = &TFT_screen_1.tft_model->chip;

mcp23017_restore_default(&chip1);
mcp23017_restore_default(&chip2);
mcp23017_restore_default(chip);

mcp23017_init_device (Lampa);

























init_mcp23017_chip_tft();







printf ("test end \n");
return (EXIT_SUCCESS);
};


int init_mcp23017_chip_tft(){
	
	
	
MCP23017 *chip = &TFT_screen_1.tft_model->chip;

mcp23017_write_byte (chip,IODIRA,0xff);
mcp23017_write_byte (chip,IODIRB,0xff);


mcp23017_write_byte (chip,OLATA, 0x00);
mcp23017_write_byte (chip,OLATB, 0xff);

mcp23017_write_byte (chip,IODIRA,0);
mcp23017_write_byte (chip,IODIRB,0);



//mcp23017_write_byte (&chip1,IODIRA,0);
//mcp23017_set_bit (&chip1,2,1);

mcp23017_enable_device (Lampa);



mcp23017_write_byte (chip, OLATB, 0b00011111); // сброс 
mcp23017_write_byte (chip, OLATB, 0b00010111); // reset
mcp23017_write_byte (chip, OLATB, 0b00011111);

sleep(1);		// ЭТА Звдержка Обязательна или будут артефакты
mcp23017_write_byte (chip, OLATB, 0b00011101);  //включить экран CS = 0
mcp23017_write_byte (chip, IOCON, 0b00100000);  // Включить чтоб байты писались по кругу



	if (mcp23017_open_i2c (chip) < 0)  { perror("I2C erorr open() "); return -1;};
 
		init_TFT_panel(&TFT_screen_1);
	    tft_setWindow(&TFT_screen_1,0,0,480,320);

 
 display_test();



	if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};
	mcp23017_write_byte (chip, OLATB, 0b00011111);	//выключить экран CS = 1	




mcp23017_disable_device(Lampa);

	
//mcp23017_set_bit (&chip1,2,0);	
	
return 0;	};















void display_test(){
	
write_tft_clear_fast(&TFT_screen_1,0x00);

tft_printf(&TFT_screen_1,temperatura);


printf("cursor_x = %d \n cursor_y = %d \n  x = %d \n",
    TFT_screen_1.window->cursor_x, 
    TFT_screen_1.window->cursor_y,
    TFT_screen_1.window->image_x0
    );




};


























