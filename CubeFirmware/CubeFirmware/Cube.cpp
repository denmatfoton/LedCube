#include "prj_defs.h"
#include "Cube.h"
#include <avr/interrupt.h>
#include <util/delay.h>


uint8_t Cube::swapByte(uint8_t n) {
    static const uint8_t inv[] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};
	uint8_t temp = inv[(n & 0xf0) >> 4];
	temp |= inv[n & 0x0f] << 4;
	return temp;
}

Cube::Cube(void) : curLevel(0), setPosition(0) {
	
}

uint8_t Cube::getRow(uint8_t y, uint8_t z) {
	return state[y][z];
}

void Cube::clear() {
	for (unsigned char i = 0; i < 0x40; ++i)
        state[LSB_VALUE(i)][MSB_VALUE(i)] = 0;
	setPosition = 0;
}

void Cube::set(uint8_t x, uint8_t y, uint8_t z) {
	state[y][z] |= 1 << x;
}

bool Cube::set(uint8_t c) {
    state[LSB_VALUE(setPosition)][MSB_VALUE(setPosition)] = c;

	if (++setPosition == 0x40) {
		setPosition = 0;
		return true;
	}
	
    return false;
}

void Cube::displayLevel() { // layer initialization
	PORTC = 0;
	_delay_us(10);

	// row initialization
	for (unsigned char j = 0; j < 8;) {
		PORTA = swapByte(state[j][curLevel]);
		PORTB = ++j & 7;
	}

	PORTC = 1 << curLevel++;
	curLevel &= 0x07;
}
