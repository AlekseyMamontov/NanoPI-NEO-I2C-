
 /* 
 * Author: Aleksey Mamontov
 * Created on 2-10-2018

 */
#include <pthread.h>
#include "stanok.h"
#include "ttp229l/TTP229.h"
#include <unistd.h>
#include <sched.h>


//#include <time.h>
//#include <signal.h>
 
int cycle2 = 0;


void test_txt();
int ret;

void tft_command(struct TFT_screen_panel *panel, uint8_t n){
	panel->command = n;
	while (panel->status_command != n){};
	while (panel->status_command == n){};
};

// TEST STANOK

int main(int argc, char** argv) {

struct TFT_screen_panel *panel = &TFT_screen_1;

if (mcp23017_open_i2c (&panel->tft_model->chip) < 0)  { perror("I2C erorr open() "); return -1;};
if (mcp23017_open_i2c (&chip1) < 0) {perror("I2C erorr open MAX6675");return -1;};


mcp23017_restore_default(&chip1);
mcp23017_restore_default(&TFT_screen_1.tft_model->chip);


Stanok_init_device();


pthread_start_tft_panel (panel);

tft_command (panel,TFT_INIT);
tft_command (panel,TFT_FAST_CLEAR);
tft_command (panel,TFT_BUILD_INFO_BLOCK);
tft_command (panel,TFT_UPDATE_CHANGE_BLOCK);

panel->command = TFT_CYCLE;


pthread_start_thermosensors(&MAX6675);


while(1){

	
	mcp23017_scan_sensors_chip(&chip1); 
	Counter_product.check_event(&Counter_product);
	TEN_matrix.check_temperature(&TEN_matrix);

	if(cycle2) Test_operation.launch_transition(&Test_operation);

	 TTP229L_read_key(&chip_keys);
/*	    
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
	
	if(chip_keys.buffer[0] == 16) {	if (!panel->status_command) 	
									{panel->atrrib_command = (void *) &Stanok_Menu;
									panel->command = TFT_MENU_DOWN;}
		};
	if(chip_keys.buffer[0] == 12) {	if (!panel->status_command) 	
									{panel->atrrib_command = (void *) &Stanok_Menu;
									panel->command = TFT_MENU_UP;}
		};
	
	if(chip_keys.buffer[0] == 15) {	if (!panel->status_command) 	
									{panel->atrrib_command = (void *) &Stanok_Menu;
									panel->command = TFT_MENU_ENTER;}
		};
	


usleep(100000);



};


printf ("test end \n");
return (EXIT_SUCCESS);
};








 


















