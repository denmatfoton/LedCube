#include <avr/interrupt.h>
#include <util/delay.h>
#include "prj_defs.h"
#include "Cube.h"
#include "usart.h"
#include <math.h>


Cube tempState;
Cube cubeState;

void countCurSin() {
    static float curOffset = 0.0;
    static float curAngel = 1.0;
	int i, j;

	tempState.clear();
	float c = cos(curAngel) * UOFFSET;
    float s = sin(curAngel) * UOFFSET;
	float h = curOffset - 3.5 * c;

    for (i = 0; i < 8; ++i, h += c - 8 * s)
		for (j = 0; j < 8; ++j, h += s)
			tempState.set(i, j, (int)floor(3.5 * sin(h) + 4));

	curOffset += UOFFSET;
	if (curOffset > twoPi)
		curOffset -= twoPi;
	curAngel += pi / 10 * (fabs(sin(curAngel / 10)) + 0.1);
	if (curAngel > twoPi)
		curAngel -= twoPi;
	if (curAngel < -twoPi)
		curAngel += twoPi;
	cubeState = tempState;
}

void initTimer(void) {
	//set the prescaler
	TCCR1B = PRESCALER;
	TCCR0 = PRESCALER;
	
	//Set the Start Value for the Timer
	TCNT1H = (char) ((0xffff - TIMER_DELAY) >> 8);
	TCNT1L = (char) (0xffff - TIMER_DELAY);
	TCNT0 = 0xff - IMAGE_REFRESH_DELAY;
	
	//Start the Timer
	TIMSK |= (1 << TOIE0) | (1 << TOIE1);
}

ISR(TIMER0_OVF_vect) {
    cubeState.displayLevel();
    TCNT0 = 0xff - IMAGE_REFRESH_DELAY;
}

int main(void) {
	// starting initialization
	DDRA = 0xff;
	DDRB = 0x07;
	DDRC = 0xff;
	initUSART();
	initTimer();
	sei();
	cubeState.clear();
	
	while(true) {
        if (!isUsartConnected())
			countCurSin();
		_delay_ms(1000.0 / F_FUNC);
    }
	return 0;
}
