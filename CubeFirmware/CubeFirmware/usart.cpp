#include "prj_defs.h"
#include "usart.h"
#include "Cube.h"
#include <avr/interrupt.h>

bool connected = false;
bool recentlyReceived = false;
uint8_t tempByte = 0x00;
uint8_t bitNumber = 0;
uint8_t serviceBit; // indicator of which cube state is transmitted, odd or even

extern Cube tempState;
extern Cube cubeState;

void processReceivedByte(uint8_t byte) {
    // first bit of the received byte is service
	if ((byte & 0x01) != serviceBit) {
		tempState.clear();
		tempByte = 0x00;
		bitNumber = 0;
		serviceBit = (byte & 0x01);
	}

    // last 7 bits are data bits
	tempByte |= ((byte >> 1) << bitNumber) & 0xFF;
	
    if (bitNumber) { // if tempByte is finished
		if (tempState.set(tempByte)) {
			cubeState = tempState;
		    tempByte = 0x00;
		    bitNumber = 0;
		    serviceBit ^= 0x01;
        }
		tempByte = 0x00;
		tempByte |= (byte >> (9 - bitNumber));
	}
	
    bitNumber += 7;
	bitNumber %= 8;
}

ISR(USART_RXC_vect) {
	if (!connected)
		connected = true;
	
	recentlyReceived = true;
	connected = true;
	
	processReceivedByte(UDR);
}

ISR(TIMER1_OVF_vect) {
	if (!recentlyReceived)
		connected = false;
	
	recentlyReceived = false;
	
	// set the start value for the timer
    TCNT1H = (char) ((0xffff - TIMER_DELAY) >> 8);
	TCNT1L = (char) (0xffff - TIMER_DELAY);
}

void initUSART() {
	UCSRA = 0x00;
	UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
	UBRRH = 0;
	UBRRL = (F_CPU / (B_RATE * 16UL)) - 1;
}

bool isUsartConnected() {
    return connected;
}
