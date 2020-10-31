/********************************
* Название: Thermostat_mini.c	*
* Автор	: Malik					*
* Дата 	: 20.07.2015 10:31:21	*
* Для AVR: ATtiny13A			*
* Тактовая частота: 4.8МГц (int)*
********************************/ 

#include <avr/io.h>
#define F_CPU 4800000UL
#include <util/delay.h>
#include "Therm23.h"
#include <avr/interrupt.h>

ISR (TIM0_OVF_vect) { if (--time==0) time_flag=1; } //64uS

    //Подпрограмма инициализации датчика
    void ds_init (void) {
	    cli();
	    register unsigned char i;
	    out_1wire=pwire1;
	    DDRB|=out_1wire;
	    _delay_ms(0.48);
	    DDRB&=~out_1wire;
	    _delay_us(65);
	    i=PINB;
	    sei();
	    if (i&pwire1) out_1wire&=~BIT(ds1);
	    if(out_1wire) {
		    _delay_ms(0.42);
		    i=PINB;
		    if (!(i&pwire1)) out_1wire&=~BIT(ds1);
	    }
	    if (out_1wire==0) {
		    step_oper=0;
		    control|=BIT(0);
	    }
	    return;
    }

    //Подпрограммы записи битов
    void writebit0(void) {
	    cli();
	    DDRB|=out_1wire;
	    _delay_us(60);
	    DDRB&=~out_1wire;
	    sei();
	    return;
    }

    void writebit1(void) {
	    cli();
	    DDRB|=out_1wire;
	    _delay_us(3);
	    DDRB&=~out_1wire;
	    sei();
	    _delay_us(58);
	    return;
    }

    //Подпрограмма записи байта в датчик
    void writebyte (unsigned char command) {
	    for (register unsigned char i=0;i<8;i++) {
		    if (command&BIT(0))	writebit1();
		    else writebit0();
		    command>>=1;
	    }
	    return;
    }

    //Подпрограмма чтения битов
    unsigned char readbit(void) {
	    unsigned char i;
	    cli();
	    DDRB|=out_1wire;
	    _delay_us(3);
	    DDRB&=~out_1wire;
	    _delay_us(10);
	    i=PINB;
	    sei();
	    _delay_us(47);
	    return i;
    }

    //Подпрограмма чтения байта из датчика
    void readbyte (unsigned char *pw1) {
	    register unsigned char p,r;
	    *pw1=0;
	    r=1;
	    for (register unsigned char i=0;i<8;i++) {
		    p=readbit();
		    if(p&pwire1) *pw1|=r;
		    r<<=1;
	    }
	    return;
    }

//Подпрограмма инициализации MK
void avr_init (void) {
	//PB0-выход/вход датчика (1) 
	//PB1-выход (нагреватель) (1)
	//PB2-выход (индикатор состояния) (1)
	//PB3-не используется (1)
	//PB4-вход (регулятор)(0)
	//PB5-вход (сброс) (0)
	DDRB=0x0e;
	PORTB=0x00;
	ADMUX=0b00100010; //Vcc,Upper,PB4
	ADCSRA=0b10000100; //ADC-enable, auto,interrupt-disable, CK/128
	TCCR0B=BIT(CS02);//CK/256
	TIMSK0=BIT(TOIE0);
	sei();
	return;
}

 //Работа с датчиком
 void receive_t (void) {
	 if (step_oper==0) {
		 step_oper=1;
		 ds_init();
		 return;
	 }
	 if (step_oper==1) {
		 step_oper=2;
		 writebyte(0xCC); //Skip ROM [CCh]
		 writebyte(0x44); //Convert T [44h]
		 ADCSRA|=BIT(ADSC); //Старт АЦП
		 time_wire=50; //~188uS
		 return;
	 }
	 if (step_oper==2) {
		 if (!time_wire)	step_oper=3;
		 return;
	 }
	 if (step_oper==3) {
		 step_oper=4;
		 ds_init();
		 return;
	 }
	 if (step_oper==4) {
		 step_oper=5;
		 writebyte(0xCC); //Skip ROM [CCh]
		 writebyte(0xBE); //Read Scratchpad [BEh]
		 return;
	 }
	 if (step_oper==5) {
		 step_oper=0;
		 readbyte(&temperLS);//младший байт
		 readbyte(&temperMS);//старший байт
		 result=(temperMS<<4)|(temperLS>>4);
	     if ((!(pwire1&out_1wire))||((result>85)&&result<=125)) {
			 control|=BIT(0);
		     return;
	     }
	     if (result>=128) {
			 result=~result;
		 }
		 return;
	 }
 }
 
  //Логика работы
void logica (void) {
	if (control&BIT(0)) {
		control&=BIT(0);
		PORTB&=~BIT(ten1);
		PORTB|=BIT(led_sost);
		return;
	}
	register unsigned char adc_res,i;
	i=PINB;
	adc_res=ADCH/3;
	if (i&BIT(ten1)) {
		if (result>=adc_res) {
			PORTB&=~BIT(ten1);
			control|=BIT(7);
		}
	}
	else {
		if (control&BIT(7)) {
			if (result+gist<adc_res) {
				PORTB|=BIT(ten1);
			}
		}
		else {
			if (result<adc_res) {
				PORTB|=BIT(ten1);
			}
		}
	}	
	return;
}
 	
int main(void) {
	avr_init();
	time=70;
	while(1) {
		ds_init();
		do {
			if (out_1wire) {
				writebyte(0xCC); //Skip ROM [CCh]
				writebyte(0x4E); //Write scratchpad [4Eh]
				writebyte(0x5A); //Write TH = 90
				writebyte(0x00); //Write TL = 0
				writebyte(0x1F); //Write config	9bit
				ds_init();
				writebyte(0xCC); //Skip ROM [CCh]
				writebyte(0x48); //Copy scratchpad [48h]
			}
			if (time_flag) {
				time_flag=0;
				time=70;
				ds_init();
				logica();
			}
		}
		while (out_1wire==0);
		while(out_1wire) {
			if (time_flag) {
				time_flag=0;
				time=70;
				logica();
				--time_wire;
				receive_t();
			}
		}
	}
}