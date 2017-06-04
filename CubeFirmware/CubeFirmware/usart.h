#ifndef USART_H
#define USART_H
#include <avr/io.h>

void processReceivedByte(uint8_t byte);
void initUSART();
bool isUsartConnected();

#endif // USART_H
