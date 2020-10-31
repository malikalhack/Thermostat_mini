 /*
 ************************************
 * Название	: Therm23.h				*
 * Автор	: Malik					*
 * Дата 	: 20.07.2015 10:32:11	*
 ************************************
 */
 //Настройка МК, индикатора и датчиком
 #define BIT _BV
 #define pwire1 BIT(PINB0)
 #define ds1 0
 #define ten1 1
 #define led_sost 2
 #define gist 5
 //Различные переменные
 unsigned char out_1wire; //Контроль датчиков
 unsigned char step_oper; //Шаги выполнения программы
 unsigned char time_wire; //Время ожидания
 unsigned char temperLS,temperMS; //Младший и старший байт значения температуры
 unsigned char result=125; //Значение температуры в 10 формате
 unsigned char control; //Регистр состояния
 //Переменные для работы с счётчиком
 volatile unsigned char time,time_flag;