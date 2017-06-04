#include "CubeConnection.h"
#include "Operations.h"
#include "DialogInput.h"

CubeState state;
bool convertVoxelMap = true;
char letter = 'A';

// Palette Handle
HINSTANCE mainInstance;
HPALETTE hPalette = NULL;
char szFile[MAX_PATH];       // buffer for file name
string textInput;
string lettersPath;
bool rButtonDown = false, deviceInit = false;
short lx, ly, windowWidth, windowHight;
HDC specdc = 0;
HBITMAP specbmp = 0;
BYTE *specbuf;
HANDLE specMut;

static LPCTSTR lpszAppName = "Voxel Editor";

// Declaration for Window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Set Pixel Format function - forward declaration
void SetDCPixelFormat(HDC hDC);

// If necessary, creates a 3-3-2 palette for the device context listed.
HPALETTE GetOpenGLPalette(HDC hDC) {
	HPALETTE hRetPal = NULL;	// Handle to palette to be created
	PIXELFORMATDESCRIPTOR pfd;	// Pixel Format Descriptor
	LOGPALETTE *pPal;			// Pointer to memory for logical palette
	int nPixelFormat;			// Pixel format index
	int nColors;				// Number of entries in palette
	int i;						// Counting variable
	BYTE RedRange,GreenRange,BlueRange;
								// Range for each color entry (7,7,and 3)

	// Get the pixel format index and retrieve the pixel format description
	nPixelFormat = GetPixelFormat(hDC);
	DescribePixelFormat(hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	// Does this pixel format require a palette?  If not, do not create a
	// palette and just return NULL
	if(!(pfd.dwFlags & PFD_NEED_PALETTE))
		return NULL;

	// Number of entries in palette.  8 bits yeilds 256 entries
	nColors = 1 << pfd.cColorBits;	

	// Allocate space for a logical palette structure plus all the palette entries
	pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +nColors*sizeof(PALETTEENTRY));

	// Fill in palette header 
	pPal->palVersion = 0x300;		// Windows 3.0
	pPal->palNumEntries = nColors; // table size

	// Build mask of all 1's.  This creates a number represented by having
	// the low order x bits set, where x = pfd.cRedBits, pfd.cGreenBits, and
	// pfd.cBlueBits.  
	RedRange = (1 << pfd.cRedBits) -1;
	GreenRange = (1 << pfd.cGreenBits) - 1;
	BlueRange = (1 << pfd.cBlueBits) -1;

	// Loop through all the palette entries
	for(i = 0; i < nColors; i++) {
		// Fill in the 8-bit equivalents for each component
		pPal->palPalEntry[i].peRed = (i >> pfd.cRedShift) & RedRange;
		pPal->palPalEntry[i].peRed = (unsigned char)(
			(double) pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (i >> pfd.cGreenShift) & GreenRange;
		pPal->palPalEntry[i].peGreen = (unsigned char)(
			(double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (i >> pfd.cBlueShift) & BlueRange;
		pPal->palPalEntry[i].peBlue = (unsigned char)(
			(double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (unsigned char) NULL;
	}
		
	// Create the palette
	hRetPal = CreatePalette(pPal);

	// Go ahead and select and realize the palette for this device context
	SelectPalette(hDC,hRetPal,FALSE);
	RealizePalette(hDC);

	// Free the memory used for the logical palette structure
	free(pPal);

	// Return the handle to the new palette
	return hRetPal;
}

// Select the pixel format for a given device context
void SetDCPixelFormat(HDC hDC) {
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this structure
		1,								// Version of this structure	
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_DOUBLEBUFFER,				// Double buffered mode
		PFD_TYPE_RGBA,					// RGBA Color mode
		32,								// Want 32 bit color
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		0,0,0,0,0,						// Not used to select mode
		16,								// Size of depth buffer
		0,								// Not used to select mode
		0,								// Not used to select mode
		0,								// Draw in main plane
		0,								// Not used to select mode
		0,0,0 };						// Not used to select mode

	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hDC, &pfd);

	// Set the pixel format for the device context
	SetPixelFormat(hDC, nPixelFormat, &pfd);
}

// Entry point of all Windows programs
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG			msg;		// Windows message structure
	WNDCLASS	wc;			// Windows class structure
	HWND		hWnd;		// Storeage for window handle

	mainInstance = hInstance;
	// Register Window style
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance 		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	
	// No need for background brush for OpenGL window
	wc.hbrBackground	= NULL;		
	
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= lpszAppName;

	// Register the window class
	if(RegisterClass(&wc) == 0)
		return FALSE;

	// Create the main application window
	hWnd = CreateWindow(lpszAppName, lpszAppName,
				// OpenGL requires WS_CLIPCHILDREN and WS_CLIPSIBLINGS
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				100, 10, 1000, 700, NULL, NULL, hInstance, NULL);

	// If window was not created, quit
	if (hWnd == NULL)
		return FALSE;

	// Display the window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	HANDLE connectThread, operationsThread;
	DWORD IDThread;
	specMut = CreateMutex(0, 0, 0);
	connectThread = CreateThread(0, 0, connect, NULL, 0, &IDThread);
	operationsThread = CreateThread(0, 0, operations, &hWnd, 0, &IDThread);

	// Process application messages until the application closes
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseHandle(connectThread);
	CloseHandle(operationsThread);
	CloseHandle(specMut);
	return msg.wParam;
}

char* getFileName(HWND hWnd, int mode) {
	OPENFILENAME ofn;       // common dialog box structure

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	if (mode / 2)
		ofn.lpstrFilter = "playable files\0*.mo3;*.mp3;*.mp2;*.mp1;*.ogg;*.wav;*.aif\0All files\0*.*\0\0";
	else
		ofn.lpstrFilter = "Voxel Map\0*.vm\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.

	switch (mode % 2) {
	case 0:
		if (GetOpenFileName(&ofn))
			return ofn.lpstrFile;
		break;
	case 1:
		if (GetSaveFileName(&ofn))
			return ofn.lpstrFile;
		break;
	}
	return NULL;
}

void createBitmap() { // create bitmap to draw spectrum in (8 bit for easy updating)
	if (specbmp)
		DeleteObject(specbmp);
	BYTE data[2000] = {0};
	BITMAPINFOHEADER *bh = (BITMAPINFOHEADER*)data;
	RGBQUAD *pal = (RGBQUAD*)(data + sizeof(*bh));
	int a;
	bh->biSize = sizeof(*bh);
	bh->biWidth = windowWidth;
	bh->biHeight = windowHight; // upside down (line 0 = bottom)
	bh->biPlanes = 1;
	bh->biBitCount = 8;
	bh->biClrUsed = bh->biClrImportant = 256;
	// setup palette
	for (a = 0; a < 256; ++a) {
		pal[a].rgbGreen = 256 - a;
		pal[a].rgbRed = a;
	}
	// create the bitmap
	specbmp = CreateDIBSection(0, (BITMAPINFO*)bh, DIB_RGB_COLORS, (void**)&specbuf, NULL, 0);
	SelectObject(specdc, specbmp);
}

// Window procedure, handles all messages for this program
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	short temp;
	static HGLRC hRC;		// Permenant Rendering context
	static HDC hDC;			// Private GDI Device context

	switch (message) {
		// Window creation, setup for OpenGL
		case WM_CREATE:
			// Store the device context
			hDC = GetDC(hWnd);		

			// Select the pixel format
			SetDCPixelFormat(hDC);		

			// Create the rendering context and make it current
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			// Create the palette
			hPalette = GetOpenGLPalette(hDC);
			SetupRC(hDC);
			specdc = CreateCompatibleDC(0);
			break;

		// Window is being destroyed, cleanup
		case WM_DESTROY:
			if (specdc)
				DeleteDC(specdc);
			if (specbmp)
				DeleteObject(specbmp);
			// Kill the timer that we created
			KillTimer(hWnd, 101);

			// Deselect the current rendering context and delete it
			wglMakeCurrent(hDC,NULL);
			wglDeleteContext(hRC);

			// Delete the palette
			if(hPalette != NULL)
				DeleteObject(hPalette);

			// Tell the application to terminate after the window
			// is gone.
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			switch (wParam) {
			case VK_RIGHT:
				map.altXShift(0.5);
				break;
			case VK_LEFT:
				map.altXShift(-0.5);
				break;
			case VK_UP:
				map.altYShift(0.5);
				break;
			case VK_DOWN:
				map.altYShift(-0.5);
				break;
			case 'U':
				++selector.x;
				break;
			case 'J':
				--selector.x;
				break;
			case 'I':
				++selector.y;
				break;
			case 'K':
				--selector.y;
				break;
			case 'O':
				++selector.z;
				break;
			case 'L':
				--selector.z;
				break;
			case VK_INSERT:
				map.addVoxel(selector);
				break;
			case VK_DELETE:
				map.deleteVoxel(selector);
				break;
			case 'S':
				getFileName(hWnd, 1);
				map.saveToFile(szFile);
				break;
			case 'W':
				getFileName(hWnd, 0);
				map.readFromFile(szFile);
				break;
			case 'C':
				map.correctCenterMas();
				break;
			case 'X':
				map.createLetter(letter);
				map.correctCenter();
				++letter;
				break;
			case 'A':
                textInput.clear();
				stopMusic();
                if (lettersPath.empty()) {
                    getFileName(hWnd, 0);
                    lettersPath.assign(szFile);
                    size_t found = lettersPath.find_last_of("/\\");
                    lettersPath.resize(found + 1);
                }
                DialogBox(mainInstance, MAKEINTRESOURCE(IDD_DIALOG_TEXT_INPUT), 0, &dialogInputTextProc);
                if (!textInput.empty()) {
                    show3DText(textInput, lettersPath);
                }
				break;
			case 'M':
				getFileName(hWnd, 2);
				stopMusic();
				playMusic(string(szFile));
				break;
			case 'N':
				stopMusic();
				break;
			case 'E':
				stopMusic();
				DialogBox(mainInstance, MAKEINTRESOURCE(IDD_DIALOG_DEVICE_INPUT), 0, &dialogInputDeviceProc);
				if (deviceInit)
					showSpectrogram();
				deviceInit = false;
				break;
			}
			InvalidateRect(hWnd, NULL, 0);
			break;
		// Window is resized.
		case WM_SIZE:
			// Call our function which modifies the clipping
			// volume and viewport
			WaitForSingleObject(specMut, INFINITE);
			if (LOWORD(lParam) > 0 || HIWORD(lParam) > 0) {
				windowWidth = ((LOWORD(lParam) + 10) / 4) * 4;
				windowHight = ((HIWORD(lParam) + 10) / 4) * 4;
			} else
				windowWidth = windowHight = 0;
			ChangeSize(LOWORD(lParam), HIWORD(lParam));
			createBitmap();
			ReleaseMutex(specMut);
			break;

		// The painting function.  This message sent by Windows 
		// whenever the screen needs updating.
		case WM_PAINT:
			if (convertVoxelMap) {
				// Call OpenGL drawing code
				RenderScene();
				map.convertToCubeState(0.29, &state);
				// Call function to swap the buffers
				SwapBuffers(hDC);
				ValidateRect(hWnd, NULL);
			}
			break;

		// Windows is telling the application that it may modify
		// the system palette.  This message in essance asks the 
		// application for a new palette.
		case WM_QUERYNEWPALETTE:
			// If the palette was created.
			if(hPalette) {
				int nRet;

				// Selects the palette into the current device context
				SelectPalette(hDC, hPalette, FALSE);

				// Map entries from the currently selected palette to
				// the system palette.  The return value is the number 
				// of palette entries modified.
				nRet = RealizePalette(hDC);

				// Repaint, forces remap of palette in current window
				InvalidateRect(hWnd,NULL,FALSE);

				return nRet;
			}
			break;

		// This window may set the palette, even though it is not the 
		// currently active window.
		case WM_PALETTECHANGED:
			// Don't do anything if the palette does not exist, or if
			// this is the window that changed the palette.
			if((hPalette != NULL) && ((HWND)wParam != hWnd)) {
				// Select the palette into the device context
				SelectPalette(hDC,hPalette,FALSE);

				// Map entries to system palette
				RealizePalette(hDC);
				
				// Remap the current colors to the newly realized palette
				UpdateColors(hDC);
				return 0;
			}
			break;
		case WM_LBUTTONDBLCLK:
			ProcessSelection(LOWORD(lParam), HIWORD(lParam), 1);
			InvalidateRect(hWnd, NULL, 0);
			break;
		case WM_LBUTTONDOWN:
			ProcessSelection(LOWORD(lParam), HIWORD(lParam), 0);
			InvalidateRect(hWnd, NULL, 0);
			break;
		case WM_RBUTTONDOWN:
			rButtonDown = true;
			ly = HIWORD(lParam);
			lx = LOWORD(lParam);
			break;
		case WM_RBUTTONUP:
			rButtonDown = false;
			break;
		case WM_MOUSEMOVE:
			//MK_SHIFT
			if (rButtonDown) {
				map.rotate(LOWORD(lParam) - lx, HIWORD(lParam) - ly);
				ly = HIWORD(lParam);
				lx = LOWORD(lParam);
				InvalidateRect(hWnd, 0, TRUE);
			}
			break;
		case WM_MOUSEWHEEL:
			temp = GET_WHEEL_DELTA_WPARAM(wParam) / 120;
			scale *= pow(scaleUnit, temp);
			if (scale > maxScale)
				scale = maxScale;
			if (scale < minScale)
				scale = minScale;
			//ChangeSize(windowWidth, windowHight);
			InvalidateRect(hWnd, 0, TRUE);
			break;
        default:   // Passes it on if unproccessed
            return (DefWindowProc(hWnd, message, wParam, lParam));
	}
    return (0L);
}
