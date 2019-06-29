
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
struct Tech_pins{
	uint8_t *pin;
	uint8_t *value;
	uint8_t num_pin;
};


struct Tech_transition{

	
 	uint8_t pin_on[MAX_tech_pin];  
 	// future add
	//struct Tech_pins 
	//*depends_pin_on[MAX_tech_pin];
 	uint8_t  N_pin_on; 
 	
	uint8_t pin_off[MAX_tech_pin];
	//future add
	//struct Tech_pins 
	//*depends_pin_off[MAX_tech_pin];
	uint8_t  N_pin_off;
	
	
	uint8_t pin_waiting_event[MAX_tech_pin]; 
	uint8_t pin_waiting_value[MAX_tech_pin];
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

struct Tech_transition *init_transition;
struct Tech_transition *first_transition;
struct Tech_transition *current_transition;	
void (*launch_transition)(struct Tech_operation *operation);
	
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

/*****************************************************************************************
 * Основной тех процесс		   								 							 * 
 *****************************************************************************************/

struct Tech_process{

		struct 
		Tech_operation *Operation;
		uint8_t n_cycle_operation;


		pthread_t pthread_id;
		void (*pthread_Tech_process)(void *);	

		// буфер	
		//uint8_t status_command;
		
		pthread_mutex_t lock_command;	
_Atomic	uint8_t position_buffer;
_Atomic uint8_t num_commands;	
		uint8_t buffer_command[256];
		void *buffer_data[256];	

void (*run_operation)();

};
/*****************************************************************************************
 * Кольцевой буфер команд и данных 													 	 *
 * Cтруктура которая передается в поток должна быть static, 							 *
 * иначе не успеет скопироватся в буфер и будет  Segmentation fault 					 *															 *
 *****************************************************************************************/
 
int Tech_set_command(struct Tech_process *Tech, uint8_t command, void *data){

   uint8_t res;
    
	if(pthread_mutex_lock(&Tech->lock_command)) perror("error mutex lock buffer_command \n");
	
	 res = Tech->position_buffer + (Tech->num_commands++);
	 Tech->buffer_command[res] = command;
	 Tech->buffer_data[res] = data;
  
	if(pthread_mutex_unlock(&Tech->lock_command)) perror("error mutex unlock buffer_command \n");
};

struct Tech_data{
	
	
	
	
	uint8_t status;
	};




/*****************************************************************************************
 * Поток который работает с тех.процессом												 * 
 *****************************************************************************************/

#define START_PROCESS 		1
#define PAUSE_PROCESS 		2
#define COUNTINUE_PROCESS 	3
#define END_PROCESS			4
#define PAUSE_KEY_TO_COUNTINUE 5
 
 
void pthread_Tech_process(void *per_tech){
	
 struct Tech_process *Tech = (struct Tech_process*)per_tech;
 struct Tech_transition *transition;
 struct Tech_operation 	 *operation;
 uint8_t command;
 void      *data;
 uint8_t status_work_flow = 1;
 uint8_t status_process   =	0;
 
 
 
 while (status_work_flow){
	
	if(Tech->num_commands){
	if(pthread_mutex_trylock(&Tech->lock_command)) {
		command = 0;
	}else{	
	
		data = &Tech->buffer_data[Tech->position_buffer];
		command = Tech->buffer_command[(Tech->position_buffer)++];
		(Tech->num_commands)--;						
		pthread_mutex_unlock(&Tech->lock_command);
	};		 
 }else command = 0;
	
 switch(command){
	 
	 case 0:
	 break;
	 
	 case START_PROCESS:

/* 
 * сбросить счетчик
 * Выставить температуру на термостатах..
 * Возможно рассмотреть  запуск процесса только
 * когда температура достигнет заданных значений
 */  

	 status_process = COUNTINUE_PROCESS;		
	 break;
	 	 
	 case PAUSE_PROCESS:
/*
 * остановить процесс, в режим паузы
*/
	 status_process = PAUSE_PROCESS;
	 break;
	  		
     case COUNTINUE_PROCESS: 
/*
 * продолжить процесс, в режиме паузы
*/     
     status_process = COUNTINUE_PROCESS;
     break;
     
	 case END_PROCESS:
/*
 * остановить процесс и не продолжать:
 * обнулить все данные и выставить значение 
 * по умолчанию.
*/	 
	 status_process = 0;		
	 break;
	 
	 case PAUSE_KEY_TO_COUNTINUE:
/* остановится и дождаться нажатия 
 * клавиши и выполнить один переход,
 * и обратно дождаться клавиши.
*/ 	 
	 break;
	 

	default:	
	break;
};
	
command =0;

switch(status_process){
		
		case COUNTINUE_PROCESS:
		operation->launch_transition(operation);
		break;
		
		
		
		
		
		
		
		
		};
	
	
	
	
	
	
	
	
	
 };//while	
pthread_exit(NULL);		
};	
/*****************************************************************************************
 * Cтарт потока Техпроцесс	   								 		 * 
 *****************************************************************************************/

void pthread_start_Tech_process(struct Tech_process *Tech){
 
 int potok = pthread_create(&Tech->pthread_id,
							NULL,
							(void*) pthread_Tech_process,
							(void *) Tech);
 if (potok){printf("ERROR; return code from pthread_create() is %d\n", potok);
	 
	 }else printf("enabled Tech_process \n");
 
};



















