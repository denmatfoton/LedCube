#include "CubeConnection.h"

DWORD WINAPI connect(LPVOID n) {
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus = FT_Open(0, &ftHandle);
	DWORD BytesReceived;
	while (ftStatus != FT_OK) {
		Sleep(500);
		FT_STATUS ftStatus = FT_Open(0, &ftHandle);
	}
	FT_SetBaudRate(ftHandle, 230400);
	FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
	char serviceBit = 0x00;
	while (true) {
		unsigned char tempByte = serviceBit, bitNumber = 0, byte;

		for (register short i = 0; i < 64; ++i) {
			byte = state.getRow(i & 0x07, (i & 0xf8) >> 3);
			tempByte |= byte << (1 + bitNumber);
			ftStatus = FT_Write(ftHandle, &tempByte, 1, &BytesReceived);
			tempByte = serviceBit | ((byte >> (7 - bitNumber)) << 1);
			++bitNumber;
			bitNumber %= 7;
		
			if (!bitNumber) {
				ftStatus = FT_Write(ftHandle, &tempByte, 1, &BytesReceived);
				tempByte = serviceBit;
			}
		}
		ftStatus = FT_Write(ftHandle, &tempByte, 1, &BytesReceived);
		serviceBit ^= 0x01;
		Sleep(50);
	}
	return 0;
}