#pragma once
#include <windows.h>

class CubeState {
private:
	unsigned char position;
	unsigned char mas[8][8];
public:
	CubeState();
	~CubeState();
	unsigned char getRow(unsigned char y, unsigned char z);
	void clear();
	void set(unsigned char x, unsigned char y, unsigned char z);
	void moveValues();
	void setRow(unsigned char, unsigned char, unsigned char);
	bool set(unsigned char c);
	HANDLE stateMutex;
};