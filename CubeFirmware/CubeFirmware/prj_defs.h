#ifndef PRJ_DEFS_H
#define PRJ_DEFS_H

#ifdef WIN32
#include <avr\iom16.h>
#define F_CPU      14745600UL
#endif

#define F_REF      100 // Hz - frequency of image refreshment (not less 8 Hz by prescaler = 1024)
#define F_TIME     10 // Hz - frequency of timer1 interrupt
#define F_FUNC     20 // Hz - frequency of function calculation
#define PRESCALER  0x05 // frequency timer prescaler (0x01 - 1, 0x02 - 8, 0x03 - 64, 0x04 - 256, 0x05 - 1024)
#define B_RATE     230400UL // baud rate

#define IMAGE_REFRESH_DELAY (F_CPU / F_REF / 1024 / 8)
#define TIMER_DELAY         (F_CPU / F_TIME / 1024) // F_CPU / F_TIME / prescaler

#define pi         3.14159265
#define twoPi      6.28318530
#define halfPi     1.57079632
#define UOFFSET    0.28571428

#endif // PRJ_DEFS_H
