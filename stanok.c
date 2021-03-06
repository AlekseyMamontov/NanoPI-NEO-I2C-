
 /* 
 * Author: Aleksey Mamontov
 * Created on 2-10-2018

 */
 
#include <unistd.h>
#include <sys/reboot.h>
#include <sched.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

//#include <stdatomic.h>

#include "stanok.h"
#include "ttp229l/TTP229.h"


int ret;


int main(int argc, char** argv) {

// запретить сигналы в потоках
sigset_t sig_mask;
sigemptyset(&sig_mask); 				// обнулить всю маску
//sigfillset(&sig_techprocess); 		// записать в маску - все сигналы 
sigaddset(&sig_mask,SIGALRM); 
sigaddset(&sig_mask,SIGHUP);  		// добавить сигнал в маску
sigprocmask(SIG_BLOCK,&sig_mask,NULL); 	// заблокировать сигналы


if (mcp23017_open_i2c (&chip1) < 0){
	perror("I2C erorr open MAX6675");
	return -1;
	};
	
mcp23017_restore_default(&chip1);
pthread_start_nGPIO (&Stanok_gpio);
pthread_start_thermoblock(&Stanok_termoblock);

struct TFT_screen_panel *panel = &TFT_screen_1;
pthread_start_tft_panel (panel);
pthread_start_counters(&Stanok_counters);
pthread_start_Tech_process(&Stanok_process);


uint8_t old_status = 0;
while(1){


//if(cycle2) Test_operation.launch_transition(&Test_operation);

	 TTP229L_read_key(&chip_keys);

	
	if(chip_keys.buffer[0] == 16 && old_status !=16) {		
		tft_set_command(panel,TFT_MENU_DOWN,&Stanok_Menu);
		old_status = 16;};
	if(chip_keys.buffer[0] == 12 && old_status !=12) {		
		tft_set_command(panel,TFT_MENU_UP,&Stanok_Menu);
		old_status = 12;};	
	if(chip_keys.buffer[0] == 15 && old_status !=15) {		
		tft_set_command(panel,TFT_MENU_ENTER,&Stanok_Menu);
		old_status = 15;};	
	if(chip_keys.buffer[0] == 14 && old_status !=14) {		
		tft_set_command(panel,TFT_FAST_CLEAR,NULL);
		old_status = 14;};
		
	if(chip_keys.buffer[0] != old_status) old_status = 0;


usleep(10000);

};


printf ("test end \n");
return (EXIT_SUCCESS);
};








 


















