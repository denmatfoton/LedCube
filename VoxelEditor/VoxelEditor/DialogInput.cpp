#include "DialogInput.h"
#include "bass.h"
#include <memory>
#pragma comment (lib, "bass.lib")

#define ItemMes(id, m, w, l) SendDlgItemMessage(hwnd, id, m, (WPARAM)(w), (LPARAM)(l))
extern bool deviceInit;
extern string textInput;

// display error messages
void Error(HWND hwnd, const char *es) {
	char mes[200];
	sprintf_s(mes, "%s\n(error code: %d)", es, BASS_ErrorGetCode());
	MessageBox(hwnd, mes, 0, 0);
}

bool InitDevice(HWND hwnd, int device) {
	BASS_RecordFree(); // free current device (and recording channel) if there is one
	// initalize new device
	if (!BASS_RecordInit(device)) {
		Error(hwnd, "Can't initialize recording device");
		return false;
	}
	return true;
}

BOOL CALLBACK dialogInputDeviceProc(HWND hwnd, UINT m, WPARAM w, LPARAM l) {
	switch (m) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case IDCANCEL:
					DestroyWindow(hwnd);
					break;
				case IDOK:
					int i = ItemMes(IDC_COMBO2, CB_GETCURSEL, 0, 0); // get the selection
					deviceInit = InitDevice(hwnd, i);
					DestroyWindow(hwnd);
					break;
			}
			break;
		case WM_INITDIALOG:
			// get list of recording devices
			BASS_DEVICEINFO di;

			for (int c = 1; ; ++c) {
                if (BASS_RecordGetDeviceInfo(c, &di)) {
				    ItemMes(IDC_COMBO2, CB_ADDSTRING, 0, di.name);
                } else {
                    break;
                }
            }
			break;
		case WM_DESTROY:
			break;
	}
	return 0;
}

static bool checkTextInput(string &str) {
    for (unsigned int i = 0; i < str.length(); ++i) {
        if (str[i] < 'A' || str[i] > 'Z')
            return false;
    }
    return true;
}

INT_PTR CALLBACK dialogInputTextProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    WORD textLength;
    INT_PTR result = FALSE;

    switch (message) { 
        case WM_INITDIALOG: 
            ItemMes(IDC_EDIT1, ES_UPPERCASE, 0, 0);
            // Set the default push button to "OK." 
            SendMessage(hwnd, 
                        DM_SETDEFID, 
                        (WPARAM) IDCANCEL, 
                        (LPARAM) 0); 

            return TRUE;
        case WM_COMMAND: 
            // Set the default push button to "OK" when the user enters text. 
            if(HIWORD (wParam) == EN_CHANGE && 
                                LOWORD(wParam) == IDC_EDIT1) {
                SendMessage(hwnd, 
                            DM_SETDEFID, 
                            (WPARAM) IDOK, 
                            (LPARAM) 0); 
            }
            switch(wParam) { 
                case IDOK: 
                    // Get number of characters. 
                    textLength = (WORD)ItemMes(IDC_EDIT1, EM_LINELENGTH, 0, 0);

                    if (textLength == 0) { 
                        MessageBox(hwnd, 
                                   "No characters entered.", 
                                   "Error", 
                                   MB_OK); 

                        EndDialog(hwnd, TRUE); 
                        return FALSE; 
                    } 
                    {
                        unique_ptr<char[]> tempText(new char[textLength + 1]);

                        // Get the characters. 
                        ItemMes(IDC_EDIT1, EM_GETLINE, 0, tempText.get()); 

                        textInput.assign(tempText.get(), textLength);
                    }

                    if (checkTextInput(textInput)) {
                        result = TRUE;
                    } else {
                        textInput.clear();
                        MessageBox(hwnd, 
                                   "Invalid characters input", 
                                   "Error", 
                                   MB_OK); 
                    }

                    EndDialog(hwnd, TRUE); 
                    return result; 

                case IDCANCEL: 
                    EndDialog(hwnd, TRUE); 
                    return TRUE; 
            } 
            return 0; 
    } 
    return result; 
    
    UNREFERENCED_PARAMETER(lParam); 
}

