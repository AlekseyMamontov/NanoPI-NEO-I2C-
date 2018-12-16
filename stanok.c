
 /* To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 * File:   main.c
 * Author: Aleksey Mamontov
 * Created on 2-10-2018
 
 * 
 */

 #include "stanok.h"
 #include "ttp229l/TTP229.h"
 #include <time.h>

int  example_mcp23017_chip_tft();
void test_txt();
// display_test();



//////TESTY----------



int main(int argc, char** argv) {
	

mcp23017_restore_default(&chip1);
mcp23017_restore_default(&chip2);

mcp23017_restore_default(&TFT_screen_1.tft_model->chip);

mcp23017_init_device (Lampa);
mcp23017_init_device (Rele1);
mcp23017_init_device (Rele2);

example_mcp23017_chip_tft();

printf ("test end \n");
return (EXIT_SUCCESS);
};




int example_mcp23017_chip_tft(){
	

	
MCP23017 *chip = &TFT_screen_1.tft_model->chip;

mcp23017_write_byte (chip,OLATA, 0x00);
mcp23017_write_byte (chip,OLATB, 0xff);

mcp23017_write_byte (chip,IODIRA,0);
mcp23017_write_byte (chip,IODIRB,0);

mcp23017_write_byte (&chip2,IODIRA,0b11111100);
mcp23017_write_byte (&chip2,OLATA, 0b00000011);


//mcp23017_write_byte (&chip1,IODIRA,0);
//mcp23017_set_bit (&chip1,2,1);



mcp23017_write_byte (chip, OLATB, 0b00011111); // сброс 
mcp23017_write_byte (chip, OLATB, 0b00010111); // reset
mcp23017_write_byte (chip, OLATB, 0b00011111);

sleep(1);		// ЭТА Звдержка Обязательна или будут артефакты

mcp23017_write_byte (chip, OLATB, 0b00011101);  //включить экран CS = 0
mcp23017_write_byte (chip, IOCON, 0b00100000);  // Включить чтоб байты писались по кругу (для экрана)


if (mcp23017_open_i2c (chip) < 0)  { perror("I2C erorr open() "); return -1;};
 
		init_TFT_panel(&TFT_screen_1);
	    tft_setWindow(&TFT_screen_1,0,0,480,320);

		//write_tft_command(&TFT_screen_1,0x28);
		write_tft_clear_fast(&TFT_screen_1,0x00);
		//write_tft_command(&TFT_screen_1,0x29);
	
		build_info_block_to_display (&TFT_screen_1);
		
		
		update_change_full_block_to_display(&TFT_screen_1);
		
if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};

mcp23017_write_byte (chip, OLATB, 0b00011111);	//выключить экран CS = 1
		







int n_cycle = 0;


test1:


   if(n_cycle == 0){
	
	Read_raw_MAX6675(&MAX6675);
	max6675_data_to_char(&MAX6675);
	n_cycle = 0;
};
	TTP229L_read_key(&chip_keys);

	

    if(chip_keys.buffer[0] == 1) mcp23017_enable_device (Lampa);
	if(chip_keys.buffer[0] == 2) mcp23017_disable_device(Lampa);
	
	if(chip_keys.buffer[0] == 5) mcp23017_enable_device (Rele1);
	if(chip_keys.buffer[0] == 6) mcp23017_disable_device(Rele1);
	
	if(chip_keys.buffer[0] == 9) mcp23017_enable_device (Rele2);
	if(chip_keys.buffer[0] == 10)mcp23017_disable_device(Rele2);

	if(chip_keys.buffer[0] == 16) Stanok_Menu.tft_menu_down(&Stanok_Menu);
	if(chip_keys.buffer[0] == 12) Stanok_Menu.tft_menu_up(&Stanok_Menu);
	if(chip_keys.buffer[0] == 15) Stanok_Menu.tft_menu_enter(&Stanok_Menu);


if (mcp23017_open_i2c (chip) < 0)  { perror("I2C erorr open() "); return -1;};
	 
  fast_change_block_to_display(&TFT_screen_1);
	 
 if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};

//n_cycle --;

sleep(1);

goto test1;










};











//tft_setWindow(&TFT_screen_1,0,50,250,320);
		
		//test_txt(&TFT_screen_1);
		//write_tft_command(&TFT_screen_1,0x37);
		
		//write_tft_data(&TFT_screen_1,0x00);
		//write_tft_data(&TFT_screen_1,0x40);













//printf("--> %d ----%d\n",MAX6675.thermo_data[0],MAX6675.thermo_data[1]);


/*
void test_txt(struct TFT_screen_panel *panel){
	
	for(int i=0x20;i<127;i++){
		
		print_char_tft(panel,i);
		
		};
	for(int i=0;i<500;i++){
		
		write_tft_command(&TFT_screen_1,0x37);
		
		write_tft_data(&TFT_screen_1,0x00);
		write_tft_data(&TFT_screen_1,i);
		
		};
};
*/







 


















