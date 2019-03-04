
  /*
 * Author: Aleksey Mamontov
 * Created on 24 июня 2018 г., 23:05
 * ukrfoil@gmail.com
 */

 #include "stanok.h"
 #include "ttp229l/TTP229.h"
 //#include <time.h>
//#include <signal.h>
#include <unistd.h>
//#include <time.h>
 
int cycle = 0; 
int cycle2 = 0;

// -------- init timer - temperature ---------//

timer_t timer_temperature;  
void delay_temp(){cycle = 1;};
	

struct sigevent evp = {
	 .sigev_notify = SIGEV_SIGNAL,
     .sigev_signo = SIGUSR1,
     .sigev_value = {
		 .sival_ptr = &timer_temperature,
		 },	 	 
};

struct itimerspec ts ={
	.it_value={		
		.tv_sec = 1,
		.tv_nsec = 0,
		},
		
	.it_interval ={
		.tv_sec = 0,
		.tv_nsec = 0,		
		}
};

//-------- timer - end ---------------//





int  example_mcp23017_chip_tft();
void test_txt();
// display_test();
int ret; 	
	
 
	
	
 
 
 


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

		write_tft_command(&TFT_screen_1,0x28);// off - display
			write_tft_clear_fast(&TFT_screen_1,0x00);
			build_info_block_to_display (&TFT_screen_1);
		write_tft_command(&TFT_screen_1,0x29);// on - display
		
		update_change_full_block_to_display(&TFT_screen_1);
		
if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};

mcp23017_write_byte (chip, OLATB, 0b00011111);	//выключить экран CS = 1



/// timer
signal(SIGUSR2,timer_operations);
if(timer_create(CLOCK_REALTIME,&signal_timer_tech_process,&timer_tech_process)) perror ("timer no_create"); 		

signal(SIGUSR1,delay_temp);
if(timer_create(CLOCK_REALTIME,&evp,&timer_temperature)) perror ("timer no_create"); 
if(timer_settime(timer_temperature,0,&ts,NULL)) perror ("timer no start");


/// cycle

test1:


   if(cycle == 1){
	
	Read_raw_MAX6675(&MAX6675);
	max6675_data_to_char(&MAX6675);
		cycle = 0;
		if(timer_settime(timer_temperature,0,&ts,NULL)) perror ("timer no start");
};

  if(cycle2) Test_operation.launch_transition(&Test_operation);



	TTP229L_read_key(&chip_keys);
	
    if(chip_keys.buffer[0] == 1) mcp23017_enable_device (Lampa);
	if(chip_keys.buffer[0] == 2) mcp23017_disable_device(Lampa);
	
	if(chip_keys.buffer[0] == 5) mcp23017_enable_device (Rele1);
	if(chip_keys.buffer[0] == 6) mcp23017_disable_device(Rele1);
	
	if(chip_keys.buffer[0] == 9) mcp23017_enable_device (Rele2);
	if(chip_keys.buffer[0] == 10)mcp23017_disable_device(Rele2);
	
	
	if(chip_keys.buffer[0] == 13 && !cycle2) {		
								   cycle2 = 1; 
						           Test_operation.current_transition = Test_operation.first_transition;
						           
						           };

	if(chip_keys.buffer[0] == 14 && cycle) {cycle2 = 0;
									//set_pause_tech_process.it_value.tv.sec = 0;
									//timer_settime(timer_tech_process,0,&set_pause_tech_process,NULL);
									};

	
	if(chip_keys.buffer[0] == 16) Stanok_Menu.tft_menu_down(&Stanok_Menu);
	if(chip_keys.buffer[0] == 12) Stanok_Menu.tft_menu_up(&Stanok_Menu);
	if(chip_keys.buffer[0] == 15) Stanok_Menu.tft_menu_enter(&Stanok_Menu);


if (mcp23017_open_i2c (chip) < 0)  { perror("I2C erorr open() "); return -1;};
	 
  fast_change_block_to_display(&TFT_screen_1);
	 
 if ( mcp23017_close_i2c (chip) < 0){ perror("I2C erorr close() "); return -1;};





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
* signal(SIGALRM,delay_temp);

 cycle = 0;
 
 test_delay.it_value.tv_sec = 1;
 test_delay.it_value.tv_usec = 0;
 
 test_delay.it_interval.tv_sec = 0;
 test_delay.it_interval.tv_usec = 0;

 ret = setitimer(ITIMER_REAL,&test_delay,NULL);
 
 if (ret) perror ("setitimer");
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/







 


















