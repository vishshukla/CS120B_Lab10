/*	Author: nnguy099
 *  Partner(s) Name: Vishwas Shukla
 *	Lab Section: 23
 *	Assignment: Lab 10  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;


void set_PWM(double f) {
    static double current_f;
    if (f != current_f) {
        if(!f) TCCR3B &= 0x08;
        else TCCR3B |= 0x03;
        
        if(f < 0.954) OCR3A = 0xFFFF;
        else if (f > 31250) OCR3A = 0x0000;
        else OCR3A = (short)(8000000 / (128 * f)) - 1;

        TCNT3 = 0;
        current_f = f;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);

    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	
	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;
}

void TimerOff () {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
} 

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char three_temp = 0x00;
unsigned char blinking_temp = 0x01;

enum ThreeLEDsSM {three_start, three_s0} Three_state;

void ThreeLEDsSM_Tick() {
	switch(Three_state) {
		case three_start: Three_state = three_s0; break;
		case three_s0:
		default: break;
	}

	switch(Three_state) { // actions
		case three_start: break;
		case three_s0: 
			if (three_temp == 0x00) three_temp = 0x01;
			else if (three_temp == 0x01) three_temp = 0x02;
			else if (three_temp == 0x02) three_temp = 0x04;
			else if (three_temp == 0x04) three_temp = 0x01;
			break;
		default: break;
	}
}

enum BlinkingLEDSM {blinking_start, blinking_s0} Blinking_state;

void BlinkingLEDSM_Tick() {
	switch(Blinking_state) {
		case blinking_start: Blinking_state = blinking_s0; break;
		case blinking_s0:
		default: break;
	}
	
	switch(Blinking_state) { // actions
		case blinking_start: break;
		case blinking_s0: blinking_temp = !blinking_temp; break;
	}
}

unsigned char speaker_temp = 0x00;

enum SpeakerSM {speaker_start, speaker_s0, speaker_s1} Speaker_state;

void SpeakerSM_Tick() {
    switch(Speaker_state) {
        case speaker_start:
            Speaker_state = speaker_s0;
            break;
        case speaker_s0:
            if ((~PINA & 0x04) == 0x04) Speaker_state = speaker_s1;
            break;
        case speaker_s1:
            if ((~PINA & 0x04) != 0x00) Speaker_state = speaker_s0;
            break;
        default: break;
    }

    switch(Speaker_state) {
        case speaker_start:
        case speaker_s0: speaker_temp = 0x00;   break;
        case speaker_s1:
                         speaker_temp = 0x01;   break;
        default: break;
    }
}

enum CombineLEDsSM {combine_start} Combine_state;

void CombineLEDsSM_Tick() {
	switch(Combine_state) { // transitions
		case combine_start: 
		default: break;
	}

	switch(Combine_state) { // actions
		case combine_start: PORTB = three_temp | (blinking_temp << 3) | (speaker_temp << 4);
                 break;
		default: break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;	
	DDRA = 0x00; PORTA = 0xFF;
	unsigned long Three_elapsedTime = 0;
	unsigned long Blinking_elapsedTime = 0;
    unsigned long Speaker_elapsedTime = 0;
	const unsigned long timerPeriod = 2;	

	Three_state = three_start;
	Blinking_state = blinking_start;
	Combine_state = combine_start;

	TimerSet(2);
	TimerOn();

    /* Insert your solution below */
    while (1) {
	if (Three_elapsedTime >= 300) { 
		ThreeLEDsSM_Tick();
		Three_elapsedTime = 0;
	}
	
	if (Blinking_elapsedTime >= 1000) {
		BlinkingLEDSM_Tick();
		Blinking_elapsedTime = 0;
	}
    
    if(Speaker_elapsedTime >= 2) {
        SpeakerSM_Tick();
        Speaker_elapsedTime = 0;
    }    
	CombineLEDsSM_Tick();
	while (!TimerFlag);
	TimerFlag = 0;
	Three_elapsedTime += timerPeriod;
	Blinking_elapsedTime += timerPeriod;	
    Speaker_elapsedTime += timerPeriod;
    }
    return 1;
}
