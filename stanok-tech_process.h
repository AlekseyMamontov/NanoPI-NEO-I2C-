
 /* 
 * Author: Aleksey Mamontov
 * mamontov1@gmail.com
 
 * 
 */
// Таймеры Posix , при компиляции gcc ...... -lrt
#include <signal.h>
#include <time.h>                                                                                   


// Технологические операции и переходы

#define MAX_tech_pin 5


/* 
 pin_on -блоки что включаются 
 pin_off - блоки что выключатся на данном переходе
 N_pin - 0 нет блоков на переключение
 pin_waiting_event - ожидания события от пина
 transition_delay -задержка при переходе к следующему переходу
 next - следующий переход
*/

struct Tech_transition{

	
 	uint8_t pin_on[MAX_tech_pin];  
	uint8_t pin_off[MAX_tech_pin];
	uint8_t pin_waiting_event[MAX_tech_pin]; 
		 
	uint8_t  N_pin_on; 		     
	uint8_t  N_pin_off;	 
	uint8_t  N_pin_waiting;
	
	uint8_t  hide_initial;
	 
    long transition_delay; 

/* 1 -  перехода в другому переходу, 
 * 0 -крутимся в этом переходе и ждем события от таймера 
 * или ждем события от концевика 
 */ 
	uint8_t move_to_next_transition;  

	struct Tech_transition *next;
				
	void (*activation_transition)(struct Tech_transition *);
	void (*pin_waiting)(struct Tech_transition *);
};


// Технологическая  операция


struct Tech_operation{

struct Tech_transition *first_transition;
struct Tech_transition *current_transition;	
void (*launch_transition)(struct Tech_operation *operation);	
	};


// Структура главного цикла 


struct Tech_process {

struct Tech_operation *Operation;

void (*cycle_operation[10])();

uint8_t * n_cycle_operation;
};

/* таймер выдержки 
   timer_create(CLOCK_REALTIME,&signal_timer_tech_process,&set_pause_tech_process);
   не забыть создать данный таймер
*/

timer_t timer_tech_process;  


struct sigevent signal_timer_tech_process = {
	 .sigev_notify = SIGEV_SIGNAL,
     .sigev_signo = SIGUSR2,
     .sigev_value = {
		 .sival_ptr = &timer_tech_process,
		 },	 	 
};

struct itimerspec set_pause_tech_process ={
	.it_value={		
		.tv_sec = 1,
		.tv_nsec = 0,
		},
		
	.it_interval ={
		.tv_sec = 0,
		.tv_nsec = 0,		
		}
};



// покаместь заворачивать в  
//activation_signal(timer_finished_transition(struct Tech_operation *operation));

void timer_finished_transition (struct Tech_operation *operation){
	
	operation->current_transition->move_to_next_transition = 1;
	
	};

//////////////// Инициализация при первом запуске перехода ////////

void inital_transition(struct Tech_transition *transition){
	
	if (transition->N_pin_off){
		for (int i=0;i<transition->N_pin_off;i++){
				Stanok_disabled_device(transition->pin_off[i]);};	
		};
	if (transition->N_pin_on){
		for (int i=0;i<transition->N_pin_on;i++){
				Stanok_enabled_device(transition->pin_on[i]);};
		};				

	// старт таймера задержки		
	if(transition->transition_delay){
		set_pause_tech_process.it_value.tv_sec = transition->transition_delay;
		if(timer_settime(timer_tech_process,0,&set_pause_tech_process,NULL)) perror ("timer Tech_transition  no start");
};
	if(!transition->N_pin_waiting && !transition->transition_delay) transition->move_to_next_transition = 1;

	transition->hide_initial = 1;
	
//	printf("Иницилизация блока %d ---- %d ----%ld \n", transition->hide_initial,transition->move_to_next_transition,transition->transition_delay);
	
};
////////////////////// Ожидания пина от концевика для перехода///////////////////// 
   
void pin_waiting_transition(struct Tech_transition *transition){

uint8_t set_waiting = 1;	

	for (int i=0;i<transition->N_pin_waiting;i++){
		//set_waiting=set_waiting&mcp23017_read_sensor(transition->pin_waiting_event[i]);	
		};	
		
transition->move_to_next_transition = set_waiting;		
};

//////////////////////// Обработка переходов ////////////////////

void launch_transition(struct Tech_operation *operation){

 struct Tech_transition *transition = operation->current_transition;

 if(transition->hide_initial ==0 ) transition->activation_transition(transition); 
 if(transition->N_pin_waiting) transition->pin_waiting(transition);
	 
 if(transition->move_to_next_transition){	 
	 transition->hide_initial = 0;
	 transition->move_to_next_transition = 0; 
	 operation->current_transition = transition->next;
	// printf("Таймер привысил %d \n", timer_getoverrun(timer_tech_process)); 
	 };

};

//////////////////////////////////////////////////////////////////////////





















