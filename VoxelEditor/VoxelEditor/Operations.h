#pragma once
#include "GLTools.h"

DWORD WINAPI operations(LPVOID n);

#define OPERATION_3D_TEXT           0
#define OPERATION_PLAY_MUSIC        1
#define OPERATION_SHOW_SPECTROGRAM  2

#define MOVING_SPECTROGRAM          1

#define BANDS      CUBE_SIZE
#define SPECHEIGHT CUBE_SIZE
#define halfPi     1.57079632679489661923132169163
#define twoPi      6.28318530717958647692528676655
#define MAX_VAL    16

void show3DText(string &str, string &lettersPath);
void playMusic(string &fileName);
void stopMusic();
void showSpectrogram();
