#include "CubeState.h"

CubeState::CubeState(void) {
	stateMutex = CreateMutex(0, FALSE, 0);
	position = 0;
	clear();
}

CubeState::~CubeState(void) {
	CloseHandle(stateMutex);
}

unsigned char CubeState::getRow(unsigned char y, unsigned char z) {
	WaitForSingleObject(stateMutex, INFINITE);
	if ((y | z) > 7 || (y | z) < 0) {
		ReleaseMutex(stateMutex);
		return 0;
	}
	unsigned char temp = mas[y][z];
	ReleaseMutex(stateMutex);
	return temp;
}

void CubeState::clear() {
	WaitForSingleObject(stateMutex, INFINITE);
	for (unsigned char i = 0; i < 0x40; ++i)
		mas[i & 0x07][(i & 0xf8) >> 3] = 0;
	position = 0;
	ReleaseMutex(stateMutex);
}

void CubeState::set(unsigned char x, unsigned char y, unsigned char z) {
	WaitForSingleObject(stateMutex, INFINITE);
	if ((x | y | z) > 7) {
		ReleaseMutex(stateMutex);
		return;
	}
	mas[y][z] |= 1 << x;
	ReleaseMutex(stateMutex);
}

void CubeState::setRow(unsigned char y, unsigned char z, unsigned char value) {
	WaitForSingleObject(stateMutex, INFINITE);
	if ((y | z) > 7) {
		ReleaseMutex(stateMutex);
		return;
	}
	mas[y][z] = value;
	ReleaseMutex(stateMutex);
}

bool CubeState::set(unsigned char c) {
	WaitForSingleObject(stateMutex, INFINITE);
	mas[position & 0x07][(position & 0xf8) >> 3] = c;
	
	if (++position == 0x40) {
		position = 0;
		ReleaseMutex(stateMutex);
		return true;
	}
	ReleaseMutex(stateMutex);
	return false;
}

void CubeState::moveValues() {
	WaitForSingleObject(stateMutex, INFINITE);
	for (unsigned char i = 0; i < 0x40; ++i)
		mas[i & 0x07][(i & 0xf8) >> 3] <<= 1;
	ReleaseMutex(stateMutex);
}