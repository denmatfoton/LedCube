#include "Operations.h"
#include "bass.h"
#include <string>
#pragma comment (lib, "bass.lib")

extern CubeState state;
extern BYTE *specbuf;
extern short windowWidth, windowHight;
extern HDC specdc;
extern HANDLE specMut;

HANDLE sem;
HANDLE mutex;
CubeState tempState;
unsigned char func = 0xff;
bool busy = false, stop;
string text;
HWND hWnd;
string pathToLetters;


void moveObject() {
	map.altZShift(-16);
	for (int i = 0; i < 8; ++i) {
		map.altZShift(2);
		InvalidateRect(hWnd, NULL, 0);
		Sleep(50);
	}
	for (int i = 0; i < 32; ++i) {
		map.rotate(20, 0);
		InvalidateRect(hWnd, NULL, 0);
		Sleep(25);
	}
}

#define READ_LETTERS_FROM_FILE 1

void show3DTextExecute() {
#if READ_LETTERS_FROM_FILE
    string temp;
#endif
	
    for (unsigned int i = 0; i < text.size(); ++i) {
		map.clear();
#if READ_LETTERS_FROM_FILE
        temp = pathToLetters + text[i] + ".vm";
        map.readFromFile(temp.c_str());
#else
        map.createLetter(text[i]);
#endif
		map.correctCenter();
		moveObject();
	}
	map.clear();
	InvalidateRect(hWnd, NULL, 0);
}

void show3DText(string &str, string &lettersPath) {
	WaitForSingleObject(mutex, INFINITE);
	text = str;
    pathToLetters = lettersPath;
	func = 0;
	ReleaseSemaphore(sem, 1, 0);
}

void stopMusic() {
	stop = true;
}

void updateSpectrogram(DWORD chan) {
	int y, x, b0, y2;
	QWORD pos, oldPos = 0;
	float fft[1024];
	stop = false;

	while (!stop && BASS_ChannelIsActive(chan)) {
		WaitForSingleObject(specMut, INFINITE);
		pos = BASS_ChannelGetPosition(chan, BASS_POS_BYTE);
		if (pos < oldPos) {
			BASS_ChannelStop(chan);
			ReleaseMutex(specMut);
			break;
		}
		oldPos = pos;
#if MOVING_SPECTROGRAM
		tempState.moveValues();
#else
		tempState.clear();
#endif
		BASS_ChannelGetData(chan, fft, BASS_DATA_FFT2048); // get the FFT data

		b0 = 0;
		memset(specbuf, 0, windowWidth * windowHight);

		for (x = 0; x < BANDS; x++) {
			float peak = 0;
			int areaOfData = pow(2, x * 10.0 / (BANDS - 1));
			if (areaOfData > 1022)
				areaOfData = 1022;
			if (areaOfData <= b0)
				areaOfData = b0 + 1; // make sure it uses at least 1 FFT bin
			
            int dataCount = areaOfData - b0;
            // get average value in current area
            for (;b0 < areaOfData; b0++)
                peak += fft[2 + b0];
            peak /= dataCount;

			
            //y = sqrt(peak) * 3 * SPECHEIGHT;
            y = log(peak * 10000 + 1) * 0.14 * SPECHEIGHT;
			y2 = log(peak * 10000 + 1) * 0.14 * windowHight;

			if (y > SPECHEIGHT)
				y = SPECHEIGHT; // cap it
			if (y2 > windowHight)
				y2 = windowHight; // cap it
			while (--y >= 0) {
#if MOVING_SPECTROGRAM
        		tempState.set(0, x, y);
#else
		        tempState.setRow(x, y, 0xff);
#endif
            }
			while (--y2 >= 0) {
				memset(specbuf + y2 * windowWidth + x * (windowWidth / BANDS),
                    (y2 + 1) * 256 / windowHight, windowWidth / BANDS - 2); // draw bar
            }
		}
		state = tempState;
		HDC dc = GetDC(hWnd);
		BitBlt(dc, 0, 0, windowWidth, windowHight, specdc, 0, 0, SRCCOPY);
		ReleaseDC(hWnd, dc);
		ReleaseMutex(specMut);
		Sleep(50);
	}
}

void playMusicExecute() {
	stop = false;
	DWORD chan;

	// check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		//printf("An incorrect version of BASS was loaded");
		return;
	}

	// setup output - default device
	if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
		//printf("Can't initialize device");
		BASS_Free();
		return;
	}

	// try streaming the file/url
    if (!(chan = BASS_StreamCreateFile(FALSE, text.c_str(), 0, 0, BASS_SAMPLE_LOOP)))
        if (!(chan = BASS_StreamCreateURL(text.c_str(), 0, BASS_SAMPLE_LOOP, 0, 0))) {
			BASS_Free();
			return;
		}

	BASS_ChannelPlay(chan, FALSE);

	updateSpectrogram(chan);

	// wind the frequency down...
	BASS_ChannelSlideAttribute(chan, BASS_ATTRIB_FREQ, 1000, 500);
	Sleep(300);
	// ...and fade-out to avoid a "click"
	BASS_ChannelSlideAttribute(chan, BASS_ATTRIB_VOL, -1, 200);
	// wait for slide to finish
	while (BASS_ChannelIsSliding(chan, 0))
		Sleep(1);

	BASS_Free();
}

BOOL CALLBACK DuffRecording(HRECORD handle, const void *buffer, DWORD length, void *user) {
	return TRUE; // continue recording
}

void showSpectrogramExecute() {
	DWORD chan;
	if (!(chan = BASS_RecordStart(44100, 1, 0, &DuffRecording, 0))) {
		return;
	}
	updateSpectrogram(chan);
	BASS_RecordFree();
	BASS_Free();
}

void showSpectrogram() {
	WaitForSingleObject(mutex, INFINITE);
	func = 2;
	ReleaseSemaphore(sem, 1, 0);
}

void playMusic(string &fileName) {
	WaitForSingleObject(mutex, INFINITE);
	text = fileName;
	func = 1;
	ReleaseSemaphore(sem, 1, 0);
}

DWORD WINAPI operations(LPVOID n) {
	hWnd = *((HWND*)n);
	sem = CreateSemaphore(0, 0, 1, 0);
    mutex = CreateMutex(0, FALSE, 0);

	while (true) {
		WaitForSingleObject(sem, INFINITE);
		busy = true;
		switch (func) {
		case OPERATION_3D_TEXT:
			show3DTextExecute();
			break;
        case OPERATION_PLAY_MUSIC:
			convertVoxelMap = false;
			playMusicExecute();
			convertVoxelMap = true;
			InvalidateRect(hWnd, NULL, 0);
			break;
        case OPERATION_SHOW_SPECTROGRAM:
			convertVoxelMap = false;
			showSpectrogramExecute();
			convertVoxelMap = true;
			InvalidateRect(hWnd, NULL, 0);
			break;
		}
		busy = false;
        ReleaseMutex(mutex);
	}
	CloseHandle(sem);
	CloseHandle(mutex);
	return 0;
}
