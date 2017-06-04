#ifndef CUBE_H
#define CUBE_H
#include <avr/io.h>

#define CUBE_SIZE      8
#define POW_2          3
#define MSB_VALUE(x)  ((x & (~(CUBE_SIZE - 1))) >> POW_2)
#define LSB_VALUE(x)  (x & (CUBE_SIZE - 1))

class Cube
{
public:
	Cube();
    uint8_t getRow(uint8_t y, uint8_t z);
	void clear();
	void set(uint8_t x, uint8_t y, uint8_t z);
	bool set(uint8_t c);
    void displayLevel();
private:
    static uint8_t swapByte(uint8_t n);
	uint8_t setPosition;
    uint8_t curLevel;
	uint8_t state[CUBE_SIZE][CUBE_SIZE];
};

#endif // CUBE_H
