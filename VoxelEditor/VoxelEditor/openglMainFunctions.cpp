#include "GLTools.h"
#define BUFFER_LENGTH 64

VoxelPoint selector;
VoxelMap map;
HDC tempDC;
float scale = 1.0;
GLint nFontList;

// Light values and coordinates
GLfloat lightPos[] = { -100.0f, 200.0f, 50.0f, 1.0f };
GLfloat fAspect;

void ChangeSize(GLsizei w, GLsizei h) {
	GLfloat nRange = 100.0f;

	// Prevent a divide by zero
	if(h == 0)
		h = 1;

	fAspect = (GLfloat)w/(GLfloat)h;

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);

	// Reset coordinate system
	glLoadIdentity();

	// Setup perspective for viewing
	gluPerspective(screenAngel, fAspect, 1, 300);

	// Viewing transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -15.0f);
	glScalef(0.3, 0.3, 0.3);
}

void RenderScene(void) {
	glEnable(GL_DEPTH_TEST);	// Hidden surface removal
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	GLfloat  ambientLight[] = {0.3f * scale, 0.3f * scale, 0.3f * scale, 1.0f};
    GLfloat  diffuseLight[] = {0.7f * scale, 0.7f * scale, 0.7f * scale, 1.0f};
	GLfloat lightPos[] = {-100.0f * scale, 200.0f * scale, 50.0f * scale, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Blue 3D Text
	glColor3ub(0, 0, 255);

    glPushMatrix();
		map.glTransformMatrix();

		map.draw();

		glTranslatef((GLfloat)selector.x, (GLfloat)selector.y, (GLfloat)selector.z);
		glColor4ub(255, 0, 0, 150);
		glDisable(GL_DEPTH_TEST);	// Hidden surface removal
		glDisable(GL_CULL_FACE);
		glutWireCube(1.0);
		glEnable(GL_DEPTH_TEST);	// Hidden surface removal
		glEnable(GL_CULL_FACE);

    glPopMatrix();
}

void SetupRC(HDC hDC) {
	// Setup the Font characteristics
	HFONT hFont;
	GLYPHMETRICSFLOAT agmf[128]; // Throw away
	LOGFONT logfont;

	logfont.lfHeight = -10;
	logfont.lfWidth = 0;
	logfont.lfEscapement = 0;
	logfont.lfOrientation = 0;
	logfont.lfWeight = FW_LIGHT;
	logfont.lfItalic = FALSE;
	logfont.lfUnderline = FALSE;
	logfont.lfStrikeOut = FALSE;
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	logfont.lfQuality = DEFAULT_QUALITY;
	logfont.lfPitchAndFamily = DEFAULT_PITCH;
	strcpy_s(logfont.lfFaceName, "Arial");

	// Create the font and display list
	hFont = CreateFontIndirect(&logfont);
	SelectObject (hDC, hFont); 

	
	//create display lists for glyphs 0 through 128 with 0.1 extrusion 
	// and default deviation. 
	nFontList = glGenLists(128);
	wglUseFontOutlines(hDC, 0, 128, nFontList, 0.0f, 0.5f, 
				WGL_FONT_POLYGONS, agmf); 

	DeleteObject(hFont);

	glEnable(GL_DEPTH_TEST);	// Hidden surface removal
	glEnable(GL_COLOR_MATERIAL);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );

	glEnable(GL_LIGHTING);
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);
	glEnable(GL_LIGHT0);


	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void ProcessSelection(int xPos, int yPos, int mode) {
	// Space for selection buffer
	static GLuint selectBuff[BUFFER_LENGTH];

	// Hit counter and viewport storage
	GLint hits, viewport[4];

	// Setup selection buffer
	glSelectBuffer(BUFFER_LENGTH, selectBuff);
	
	// Get the viewport
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Switch to projection and save the matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	// Change render mode
	glRenderMode(GL_SELECT);

	// Establish new clipping volume to be unit cube around
	// mouse cursor point (xPos, yPos) and extending two pixels
	// in the vertical and horizontal direction
	glLoadIdentity();
	gluPickMatrix(xPos, viewport[3] - yPos, 2,2, viewport);

	// Apply perspective matrix 
	gluPerspective(screenAngel, fAspect, 1, 300);

	// Draw the scene
	
	map.virtualDraw();

	// Collect the hits
	hits = glRenderMode(GL_RENDER);

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Go back to modelview for normal rendering
	glMatrixMode(GL_MODELVIEW);

	if(hits) {
		hits <<= 2;
		unsigned int cur = 3, min = selectBuff[1];
		for (int i = 5; i < hits; i += 4) {
			if (min > selectBuff[i]) {
				min = selectBuff[i];
				cur = i + 2;
			}
		}
		selector = map.getSelector(selectBuff[cur], mode);
	}
}
