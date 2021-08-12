
 /* 
 * File:   stanok_txt.h
 * Author: Aleksey Mamontov
 
 
 */
//////////////////////////////////////////////////////////////////////////
//				Текст для термостата матрицы					       ///																  |
//////////////////////////////////////////////////////////////////////////


// Строим внешний вид блока термостата на экране
uint8_t TEN_matrix_init[]={ SET_font18pt,   				// font 18pt
							SET_cursorX,18,
							SET_cursorY,0,   				// set X_cusror 18
							SET_background,0x04,0x00,		// Set background  GREEN
							0x20,0x20,0xCC,0xE0,0xF2,0xF0,0xE8,0xF6,0xE0,0x20,0x20,0x20, // Матрица
							
							//0x20,0xd2,0xe5,0xec,0xef,0xe5,0xf0,0xe0,0xf2,0xf3,0xf0,0xe0, //txt Температура
							SET_background,0x00,0x00, 
							SET_cursorX,28, //
							SET_cursorY,2,
							0xad,0xd1,
							SET_cursorX,18, //
							SET_cursorY,4,  //
							0xc2,0xea,0xeb,0x3a,0x20,//Вкл:
							SET_cursorX,28, //
							SET_cursorY,4,
							0xad,0xd1,	
							SET_cursorX,18,
							SET_cursorY,5,
							0xc2,0xfb,0xea,0xeb,0x3a, //Выкл:
							SET_cursorX,28, //
							SET_cursorY,5,
							0xad,0xd1,	
							0};

// Отображение текущей температуры
uint8_t TEN_matrix_temperature[]={SET_fontNumber,
								  SET_background,0,0,
								  SET_font,0xff,0xff,
								  SET_cursorX,23, 
								  SET_cursorY,2,0};								  
uint8_t TEN_matrix_temperature_txt[6]= {0x20,0x20,0x20,0x20,0x30,0};


// Отображение температуры ON 
uint8_t TEN_matrix_on[]={	SET_font18pt,
							SET_cursorX,23, //
							SET_cursorY,4,0};							
uint8_t TEN_matrix_on_txt[6]= {0x20,0x20,0x20,0x20,0x30,0};
uint8_t TEN_new_matrix_on_txt[6]= {0x20,0x20,0x20,0x20,0x20,0};


// Отображение температуры OFF 
uint8_t TEN_matrix_off[]={	SET_font18pt,
							SET_cursorX,23, //
							SET_cursorY,5,0};
							
uint8_t TEN_matrix_off_txt[6]= {0x20,0x20,0x20,0x20,0x30,0};
uint8_t TEN_new_matrix_off_txt[6]= {0x20,0x20,0x20,0x20,0x20,0};

uint8_t TEN_matrix_temperature_visible = 1;
uint8_t TEN_matrix_on_visible = 0;
uint8_t TEN_matrix_off_visible = 0;



////////////////////////////////////////////////////////////////////////////
//				Определяем текст для Термостата пуансона				  //
///////////////////////////////////////////////////////////////////////////

uint8_t TEN_punch_init[]= { SET_font18pt,   				// font 18pt
							SET_cursorX,18,
							SET_cursorY,07,   				// set X_cusror 18
							SET_background,0x04,0x00,		// Set background  GREEN
							0x20,0x20,0xCF,0xF3,0xE0,0xED,0xF1,0xEE,0xED,0x20,0x20,0x20,
							SET_background,0x00,0x00, 
							SET_cursorX,28, //
							SET_cursorY,9,
							0xad,0xd1,
							SET_cursorX,18, //
							SET_cursorY,11,  //
							0xc2,0xea,0xeb,0x3a,0x20,//Вкл:
							SET_cursorX,28, //
							SET_cursorY,11,
							0xad,0xd1,							
							SET_cursorX,18,
							SET_cursorY,12,
							0xc2,0xfb,0xea,0xeb,0x3a,
							SET_cursorX,28, //
							SET_cursorY,12,
							0xad,0xd1,					//Выкл:
							0};
							
// Отображения текучей температуры пуансона							 
uint8_t TEN_punch_temperature[]={SET_fontNumber,
							 SET_background,0,0,
							 SET_font,0xff,0xff,
							 SET_cursorX,23, //
							 SET_cursorY,9,
							 0};							 
uint8_t TEN_punch_temperature_txt[6]={0x20,0x20,0x20,0x20,0x30,0};	

// Отображение температуры ON для пуансона
uint8_t TEN_punch_on[]={	SET_font18pt,
							SET_cursorX,23, //
							SET_cursorY,11,0};							
uint8_t TEN_punch_on_txt[6]= {0x20,0x20,0x20,0x20,0x30,0};
uint8_t TEN_new_punch_on_txt[6]= {0x20,0x20,0x20,0x20,0x20,0};
// Отображение температуры OFF пуансона
uint8_t TEN_punch_off[]={	SET_font18pt,
							SET_cursorX,23, //
							SET_cursorY,12,0};							
uint8_t TEN_punch_off_txt[6]= {0x20,0x20,0x20,0x20,0x30,0};
uint8_t TEN_new_punch_off_txt[6]= {0x20,0x20,0x20,0x20,0x20,0};

uint8_t TEN_punch_temperature_visible = 1;
uint8_t TEN_punch_on_visible = 0;
uint8_t TEN_punch_off_visible = 0;

//////////////////////////////////////////////////////////////////////////////////
//				Создаем блок счетчика готовой продукции					       //
////////////////////////////////////////////////////////////////////////////////

//Иницилизируем массивы для отображения  счетчика	

uint8_t Counter_product_init[]={
						SET_font18pt,
						SET_cursorX,0,   // set X_cusror 
						SET_cursorY,0,
						//SET_background,0x00,0x1f,
						SET_background,0xd0,0x21, // Set background
						0x20,0x20,0x20,0x20,0x20,0xC8,0xe7,0xe4,0xe5,0xeb,0xe8,0xe9,0x20,0x20,0x20,0x20,0x20, //txt  Изделий
						SET_background,0x00,0x0,
						SET_cursorX,14,   // set X_cusror 
						SET_cursorY,2,
						0xd8,0xf2,0x2e,
						0,0};

uint8_t Counter_product_change[]={
						SET_fontNumber,
						SET_cursorX,3,   // set X_cusror 
						SET_cursorY,2,
						SET_background,0,0, // Set background
						0
						};
	
uint8_t Counter_product_change_txt[]={
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x30,0,0 //txt  Изделий
	};	
uint8_t Counter_product_new_change_txt[]={
	0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x30,0,0 //txt  Изделий
	};	

uint8_t Counter_product_visible = 0x1;

//////////////////////////////////////////////////////////////////////////////
//	Индикатор включения и выключения устройств, значения входных датчиков   //
//////////////////////////////////////////////////////////////////////////////


// Иницилизатор строки индикаторов
uint8_t init_indikator_devices[]={ SET_font8pt,
						SET_cursorX,0,   // set X_cusror 
						SET_cursorY,8,
						SET_background,0x04,0x00,
						//SET_background,0xFc,0x00, // Set background
						SET_font,0xFF,0xFF,
//						0x20,0x20,0x20,0xc8,0xed,0xe4,0xe8,0xea,0xe0,0xf2,0xee,0xf0,0xfb,0x20,0x20,0x20, // Индикаторы
						0x20,0x20,0x20,0xd3,0xf1,0xf2,0xf0,0xee,0xe9,0xf1,0xf2,0xe2,0xe0,0x20,0x20,0x20,
						SET_background,0x00,0x0,
						0};						

//Отвечает за отображение информацию от ТЭНа 1
uint8_t TEN_punch_led[] = {SET_font8pt,
					   SET_cursorX,0,   // set X_cusror 
					   SET_cursorY,10,0};

//изменяя данные два байта мы будем менять фон под текстом (для отображения оn - off)
// arr [0] - изменить фон , arr[1] -> color msb, arr[2] -> color lsb; 
uint8_t TEN_punch_led_txt[] ={ 
					   //SET_background,0x00,0x0,
					   0x20,0xD2,0xDD,0xCD,0x20,0xeF,0xF3,0xE0,0xED,0xF1,0xEE,0xED,0xe0, // ТЭН пуансон
					   0};
					   
//Отвечает за отображение информации от ТЭНа 2					   
uint8_t TEN_matrix_led[] = {SET_font8pt,
					   SET_cursorX,0,   // set X_cusror 
					   SET_cursorY,11,0};
uint8_t TEN_matrix_led_txt[] ={
					   //SET_background,0x00,0x0,					   
					   0x20,0xD2,0xDD,0xCD,0x20,0xeC,0xE0,0xF2,0xF0,0xE8,0xF6,0xfb,0x20, // ТЭН матрицы
					   0};						   
//Отвечает за отображения состояние Пневмоцилиндра включен или выключен					   				   
uint8_t pneumatic_cylinder1_led[] = {
					   SET_font8pt,
					   SET_cursorX,0,   
					   SET_cursorY,12,0};
uint8_t pneumatic_cylinder1_led_txt[] ={
					   //SET_background,0x00,0x0,					   
					   0x20,0xCF,0xED,0xE5,0xE2,0xEC,0xEE,0xF6,0xE8,0xEB,0xE8,0xED,0xE4,0xF0, //Пневмоцилиндр
					   0};	
					   
//Отвечает за Вакуумный клапан 					   
uint8_t pneumatic_klapan1_led[] = {
					   SET_font8pt,
					   SET_cursorX,0,   
					   SET_cursorY,13,0};
uint8_t pneumatic_klapan1_led_txt[] ={
					   //SET_background,0x00,0x0,
					   0x20,0xCA,0xEB,0xE0,0xEF,0xE0,0xED,0x20,0xe2,0xe0,0xea,0xea,0xf3,0xec,0xe0, //  Клапан 1
					   0};		

uint8_t TEN_punch_led_visible = 0xff;
uint8_t TEN_matrix_led_visible = 0xff;
uint8_t pneumatic_cylinder1_visible = 0xff;
uint8_t pneumatic_klapan1_led_visible = 0xff;



////////////////////////////////// Indikator Sensors //////////////////////////////////////


// Cтроим визуальное не изменяемое иззображение данного блока  
uint8_t init_indikator_sensors[]={ 
						SET_font8pt,
						SET_cursorX,19,   
						SET_cursorY,8,
						//SET_background,0x04,0x00,
						SET_background,0xFc,0x00, // Set background
						SET_font,0xFF,0xFF,
						0x20,0x20,0x20,0xc4,0xe0,0xf2,0xf7,0xe8,0xea,0xe8,0x20,0x20,0x20,0x20, //txt Датчики
						SET_background,0x00,0x0,
						0};

/// Датчик выпадения заготовки из штампа, он же влияет на счетчик - я о датчике )).
uint8_t sensors_1_led[] = {SET_font8pt,
					   SET_cursorX,19,   
					   SET_cursorY,10,
					   0};
uint8_t sensors_1_led_txt[] ={ 
					   0x20,0xc3,0xee,0xf2,0x2e,0xef,0xf0,0xee,0xe4,0xf3,0xea,0xf6,0xe8,0xe8,0x20,  
					   0};

//Концевик (геркон) на возрат пневмоциллиндра
uint8_t sensors_2_led[] = {SET_font8pt,
					   SET_cursorX,19,   
					   SET_cursorY,11,0};
uint8_t sensors_2_led_txt[] ={						   
					   0x20,0xca,0xee,0xed,0xf6,0xe5,0xe2,0xe8,0xea,0x20,0x31, 
					   0};	
					   					   				   
//Концевик (геркон) на зажим - срабатывает когда пневмоцилиндр придавил заготовку
uint8_t sensors_3_led[] = {SET_font8pt,
					   SET_cursorX,19,   
					   SET_cursorY,12,0};
uint8_t sensors_3_led_txt[] ={						   
					   0x20,0xca,0xee,0xed,0xf6,0xe5,0xe2,0xe8,0xea,0x20,0x32,
					   0};	
					   
// Наличие напряжения 220В. ))  	
uint8_t sensors_4_led[] = {SET_font8pt,
					   SET_cursorX,19,   
					   SET_cursorY,13,0};
uint8_t sensors_4_led_txt[] ={					   
					   0x20,0xca,0xee,0xed,0xf6,0xe5,0xe2,0xe8,0xea,0x20,0x33, // 
					   0};					   

uint8_t sensors_1_led_visible=0xff;
uint8_t sensors_2_led_visible=0xff;
uint8_t sensors_3_led_visible=0xff;
uint8_t sensors_4_led_visible=0xff;

////////////////////////////////////////////////////////////////////////////
//                    menu block
////////////////////////////////////////////////////////////////////////////

#define MAX_line_char 18 // максимальное количество символов в строке вместе с 0 (окончания текста) 

uint8_t TFT_menu_init []={  SET_font18pt, 
							SET_cursorX,0,   // set X_cusror 
							SET_cursorY,8,
							SET_background,0x00,0x1f, // Set background
							SET_font,0xFF,0xFF,
							0x20,0x20,0x20,0xCC,0xE5,0xED,0xFE,0x20,0xf1,0xf2,0xe0,0xed,0xea,0xe0,0x20,0x20,0x20, //txt  Изделий
							SET_background,0x00,0x0,
							0};
							
//-------------------start block visible  menu --------------// 							

// 1 строка меню

uint8_t menu_line_1_off[]={SET_font18pt,
					   SET_background,0x00,0x0,
					   SET_font,0xFF,0xFF,
					   SET_cursorX,0,   
					   SET_cursorY,9,
					   0};
uint8_t menu_line_1_on[]={SET_font18pt,
					   SET_background,0xFF,0xFF,
					   SET_font,0,0,
					   SET_cursorX,0,   
					   SET_cursorY,9,
					   0};					   
					   								   
// 2 строка меню

uint8_t menu_line_2_off[]={SET_font18pt,
					   SET_background,0x00,0x0,
					   SET_font,0xFF,0xFF,
					   SET_cursorX,0,   
					   SET_cursorY,10,
					   0};
uint8_t menu_line_2_on[]={SET_font18pt,
					   SET_background,0xFF,0xFF,
					   SET_font,0,0,
					   SET_cursorX,0,   
					   SET_cursorY,10,
					   0};
// 3 строка меню
uint8_t menu_line_3_off[]={SET_font18pt,
					   SET_background,0x00,0x0,
					   SET_font,0xFF,0xFF,
					   SET_cursorX,0,   
					   SET_cursorY,11,
					   0};
uint8_t menu_line_3_on[]={SET_font18pt,
					   SET_background,0xFF,0xFF,
					   SET_font,0,0,
					   SET_cursorX,0,   
					   SET_cursorY,11,
					   0};
// 4 строка меню
uint8_t menu_line_4_off[]={SET_font18pt,
					   SET_background,0x00,0x0,
					   SET_font,0xFF,0xFF,
					   SET_cursorX,0,   
					   SET_cursorY,12,
					   0};
uint8_t menu_line_4_on[]={SET_font18pt,
					   SET_background,0xFF,0xFF,
					   SET_font,0,0,
					   SET_cursorX,0,   
					   SET_cursorY,12,
					   0};


uint8_t menu_line_1_txt[MAX_line_char] =  {0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t menu_line_2_txt[MAX_line_char] =  {0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t menu_line_3_txt[MAX_line_char] =  {0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};					   					   					   
uint8_t menu_line_4_txt[MAX_line_char] =  {0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};					   

uint8_t menu_line_1_visible = 1;
uint8_t menu_line_2_visible = 1;
uint8_t menu_line_3_visible = 1;
uint8_t menu_line_4_visible = 1;


//------------------end block visible menu ----------------//					   
					   
					   
//---------------- TXT - menu -----------------------------//
					   
uint8_t space_txt_menu[] ={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t prev_txt_menu[] = {0x20,0x2e,0x2e,0x2e,0x2e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};


// ----------------start process ------------------------//

uint8_t start_process_txt_menu[] ={  
0x20,0xd1,0xf2,0xe0,0xf0,0xf2,0x20,0xf8,0xf2,0xe0,0xEC,0xef,0xee,0xe2,0xea,0xe8,0x20,0};

// ------------  Start submenu ------------//

uint8_t exit_operation_txt_menu[]={
0x20,0xc7,0xe0,0xea,0xee,0xed,0xf7,0xe8,0xf2,0xfc,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t pause_transition_txt_menu[]={
0x20,0xce,0xf1,0xf2,0xe0,0xed,0xee,0xe2,0xe8,0xf2,0xfc,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t continue_transition_txt_menu[]={
0x20,0xcf,0xf0,0xee,0xe4,0xee,0xeb,0xe6,0xe8,0xf2,0xfc,0x20,0x20,0x20,0x20,0x20,0x20,0};
uint8_t one_pass_transition_txt_menu[]={
0x20,0xcf,0xee,0xf8,0xe0,0xe3,0xee,0xe2,0xee,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};





//------- programm -----------------------------------------//
uint8_t program_process_txt_menu[] ={
0x20,0xcf,0xf0,0xee,0xe3,0xf0,0xe0,0xec,0xec,0xfb,0x20,0xf1,0xf2,0xe0,0xed,0xea,0xe0,0};			
//---------------submenu programm.......//

uint8_t Stanok_number_operation[] ={0,1,2,3,4,5,6,7};
	
uint8_t programm_160x120_txt_menu[] ={
SET_background,0x3a,0x07,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0x31,0x36,0x30,0xf5,0x31,0x32,0x30,0x20,0};	
uint8_t programm_140x200_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0x32,0x30,0x30,0xf5,0x31,0x34,0x30,0x20,0};	
uint8_t programm_D200_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0xc4,0x32,0x30,0x30,0x20,0x20,0x20,0x20,0};	
uint8_t programm_D370_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0xc4,0x33,0x37,0x30,0x20,0x20,0x20,0x20,0};	
uint8_t programm_D300_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0xc4,0x33,0x30,0x30,0x20,0x20,0x20,0x20,0};
uint8_t programm_D275_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xd2,0xe0,0xf0,0xe5,0xeb,0xea,0xe0,0x20,0xc4,0x32,0x37,0x35,0x20,0x20,0x20,0x20,0};
uint8_t program_test_txt_menu[] ={
0x1f,0x1f,0x1f,0x20,0xf2,0xe5,0xf1,0xf2,0xee,0xe2,0xfb,0xe9,0x20,0xf0,0xe5,0xe6,0xe8,0xec,0x20,0x20,0};


// ------- setting-----------//
uint8_t setting_device_txt_menu[] ={  
0x20,0xCD,0xe0,0xf1,0xf2,0xf0,0xee,0xe9,0xea,0xe0,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};

//-----submenu setting-------//
uint8_t Thermostat_punch_txt_menu[] ={ 
0x20,0xd2,0x2d,0xf1,0xf2,0xe0,0xf2,0x20,0xeF,0xF3,0xE0,0xED,0xF1,0xEE,0xED,0xe0,0x20,0};					  
uint8_t Thermostat_matrix_txt_menu[] ={ 
0x20,0xd2,0x2d,0xf1,0xf2,0xe0,0xf2,0x20,0xeC,0xE0,0xF2,0xF0,0xE8,0xF6,0xfb,0x20,0x20,0};
uint8_t Counter_txt_menu[] ={ 
0x20,0xd1,0xf7,0xe5,0xf2,0xf7,0xe8,0xea,0x20,0xe8,0xe7,0xe4,0xe5,0xeb,0xe8,0xe9,0x20,0};


// ----- exit programm stanok ---//
uint8_t exit_process_txt_menu[] ={ 
0x20,0xC2,0xfb,0xe9,0xf2,0xe8,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0};
//-----submenu setting-------//
uint8_t restart_stanok_txt_menu[] ={ 
0x20,0xd0,0xe5,0xf1,0xf2,0xe0,0xf0,0xf2,0x20,0xf1,0xf2,0xe0,0xed,0xea,0xe0,0x20,0x20,0};
uint8_t power_off_stanok_txt_menu[] ={ 
0x20,0xC2,0xfb,0xea,0xeb,0xfe,0xf7,0xe8,0xf2,0xfc,0x20,0xf1,0xf2,0xe0,0xed,0xee,0xea,0};
uint8_t exit_programm_txt_menu[] ={ 
0x20,0xC2,0xfb,0xe9,0xf2,0xe8,0x20,0xe2,0x20,0x4c,0x69,0x6e,0x75,0x78,0x20,0x20,0x20,0};




