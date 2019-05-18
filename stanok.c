
 /* 
 * Author: Aleksey Mamontov
 * Created on 2-10-2018

 */
#include <pthread.h>
#include "stanok.h"
#include "ttp229l/TTP229.h"
#include <unistd.h>
//#include <time.h>
//#include <signal.h>
 
int cycle2 = 0;

int  example_mcp23017_chip_tft();
void test_txt();
int ret,tft_potok,max_potok; 	
pthread_t tft_threads, max_threads;

// TEST STANOK



int main(int argc, char** argv) {

if (mcp23017_open_i2c (&TFT_screen_1.tft_model->chip) < 0)  { perror("I2C erorr open() "); return -1;};
if (mcp23017_open_i2c (&chip1) < 0) {perror("I2C erorr open MAX6675");return -1;};


	
mcp23017_restore_default(&chip1);
mcp23017_restore_default(&TFT_screen_1.tft_model->chip);



example_mcp23017_chip_tft();

printf ("test end \n");
return (EXIT_SUCCESS);
};




int example_mcp23017_chip_tft(){
	

// init chip for thermostat

//mcp23017_write_byte (&chip1,IODIRB,0b00111111);
//mcp23017_write_byte (&chip1,OLATB, 0b11000000);
//mcp23017_write_byte (&chip1,IOCON, 0b00000000); // ne pomogaty

Stanok_init_device();




// tft init


MCP23017 *chip = &TFT_screen_1.tft_model->chip;

mcp23017_write_byte (chip,OLATA, 0x00);
mcp23017_write_byte (chip,OLATB, 0xff);

mcp23017_write_byte (chip,IODIRA,0);
mcp23017_write_byte (chip,IODIRB,0);


mcp23017_write_byte (chip, OLATB, 0b00011111); // сброс 
mcp23017_write_byte (chip, OLATB, 0b00010111); // reset
mcp23017_write_byte (chip, OLATB, 0b00011111);

sleep(1);		// ЭТА Звдержка Обязательна или будут артефакты

mcp23017_write_byte (chip, OLATB, 0b00011101);  //включить экран CS = 0
mcp23017_write_byte (chip, IOCON, 0b00100000);  // Включить чтоб байты писались по кругу (для экрана)



 
		init_TFT_panel(&TFT_screen_1);
	    tft_setWindow(&TFT_screen_1,0,0,480,320);

		//write_tft_command(&TFT_screen_1,0x28);// off - display
			write_tft_clear_fast(&TFT_screen_1,0x00);
			build_info_block_to_display (&TFT_screen_1);
		//write_tft_command(&TFT_screen_1,0x29);// on - display
		
		update_change_full_block_to_display(&TFT_screen_1);
		


mcp23017_write_byte (chip, OLATB, 0b00011111);	//выключить экран CS = 1

//MAX6675.init_timer(&MAX6675);

	tft_potok = pthread_create(&tft_threads, NULL, tft_panel, (void *) &TFT_screen_1);
		if (tft_potok) {printf("ERROR; return code from pthread_create() is %d\n", tft_potok);exit(-1);};


	pthread_start_thermosensors(&MAX6675);


test1:

	
	mcp23017_scan_sensors_chip(&chip1); 
	Counter_product.check_event(&Counter_product);
	TEN_matrix.check_temperature(&TEN_matrix);




 if(cycle2) Test_operation.launch_transition(&Test_operation);



	 TTP229L_read_key(&chip_keys);
/*	
    if(chip_keys.buffer[0] == 1) ;
	if(chip_keys.buffer[0] == 2) ;
	
	if(chip_keys.buffer[0] == 5) ;
	if(chip_keys.buffer[0] == 6) ;
	
	if(chip_keys.buffer[0] == 9) ;
	if(chip_keys.buffer[0] == 10);
	
	
	if(chip_keys.buffer[0] == 13 && !cycle2) {		
								   cycle2 = 1; 
						           Test_operation.current_transition = Test_operation.first_transition;
						           
						           };

	if(chip_keys.buffer[0] == 14 && cycle2) {cycle2 = 0;
									//set_pause_tech_process.it_value.tv.sec = 0;
									//timer_settime(timer_tech_process,0,&set_pause_tech_process,NULL);
									};
*/
	
	if(chip_keys.buffer[0] == 16) Stanok_Menu.tft_menu_down(&Stanok_Menu);
	if(chip_keys.buffer[0] == 12) Stanok_Menu.tft_menu_up(&Stanok_Menu);
	if(chip_keys.buffer[0] == 15) Stanok_Menu.tft_menu_enter(&Stanok_Menu);




	 
  //fast_change_block_to_display(&TFT_screen_1);
	 
 //if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};

//usleep(100000);



goto test1;










};








 


















