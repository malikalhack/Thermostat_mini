 /*
 ************************************
 * ��������	: Therm23.h				*
 * �����	: Malik					*
 * ���� 	: 20.07.2015 10:32:11	*
 ************************************
 */
 //��������� ��, ���������� � ��������
 #define BIT _BV
 #define pwire1 BIT(PINB0)
 #define ds1 0
 #define ten1 1
 #define led_sost 2
 #define gist 5
 //��������� ����������
 unsigned char out_1wire; //�������� ��������
 unsigned char step_oper; //���� ���������� ���������
 unsigned char time_wire; //����� ��������
 unsigned char temperLS,temperMS; //������� � ������� ���� �������� �����������
 unsigned char result=125; //�������� ����������� � 10 �������
 unsigned char control; //������� ���������
 //���������� ��� ������ � ���������
 volatile unsigned char time,time_flag;