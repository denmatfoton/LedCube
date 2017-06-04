#include "VoxelMap.h"

VoxelMap::VoxelMap(void) {
	xCor = yCor = zCor = 0.0;
	gltLoadIdentityMatrix(curRotTransform);
	xShift = yShift = 0.0;
	zShift = 0.0;
    mutex = CreateMutex(0, FALSE, 0);
}

VoxelMap::~VoxelMap(void) {
}

void VoxelMap::addVoxel(VoxelPoint point) {
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++)
		if (*it == point) return;
	map.push_back(point);
    ReleaseMutex(mutex);
}

void VoxelMap::deleteVoxel(VoxelPoint point) {
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++)
		if (*it == point) {
			map.erase(it);
			return;
		}
    ReleaseMutex(mutex);
}

void VoxelMap::draw() {
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++) {
		glPushMatrix();
			glTranslatef(it->x, it->y, it->z);
			glColor3ub(0, 0, 255);
			glutSolidCube(1.0);
			glColor3ub(0, 0, 0);
			glutWireCube(1.0);
		glPopMatrix();
	}
    ReleaseMutex(mutex);
}

void VoxelMap::virtualDraw() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTransformMatrix();

	glColor3ub(0, 0, 255);
	GLint i = 0;
	// Initialize the names stack
	glInitNames();
	glPushName(0);
    
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); ++it, ++i) {
		glPushMatrix();
			glTranslatef(it->x, it->y, it->z);
			solidCube(i);
		glPopMatrix();
	}
    ReleaseMutex(mutex);
	glPopMatrix();
}

// get selected by cursor voxel
VoxelPoint VoxelMap::getSelector(int i, int mode) {
	VoxelPoint v = map[--i / 6];
	switch (i % 6 + mode * 6) {
	case 0:
		++v.x;
		break;
	case 1:
		--v.z;
		break;
	case 2:
		--v.x;
		break;
	case 3:
		++v.z;
		break;
	case 4:
		++v.y;
		break;
	case 5:
		--v.y;
		break;
	}
	return v;
}

// draw a voxel to determine, if it was selected by a cursor
void VoxelMap::solidCube(int n) {
	glPushMatrix();
	n *= 6;
	for (int i = 1; i < 7; ++i) {
		glLoadName(n + i);
		glBegin(GL_QUADS);
			glVertex3f(0.5, 0.5, 0.5);
			glVertex3f(0.5, -0.5, 0.5);
			glVertex3f(0.5, -0.5, -0.5);
			glVertex3f(0.5, 0.5, -0.5);
		glEnd();
		if (i == 4) {
			glRotatef(90.0, 0.0, 0.0, 1.0);
			continue;
		}
		if (i == 5) {
			glRotatef(180.0, 0.0, 0.0, 1.0);
			continue;
		}
		glRotatef(90.0, 0.0, 1.0, 0.0);
	}
	glPopMatrix();
}

void VoxelMap::saveToFile(char* fileName) {
	FILE *fp;
	int l = strlen(fileName);
	int i = l - 1;
	bool correct = false;

	for (; i >= 0; --i) {
		if (fileName[i] == '\\') {
			i = l;
			break;
		}
		if (fileName[i] == '.') {
			++i;
			break;
		}
	}
	
	if (i == -1)
		return;

	if (!((i == l - 2) && (fileName[i] == 'v') && (fileName[i + 1] == 'm'))) {
		fileName[l] = '.';
		fileName[++l] = 'v';
		fileName[++l] = 'm';
		fileName[++l] = '\0';
	}

	if(!(fp = fopen(fileName, "wb")))
		return;
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++) {
		fwrite(&(*it), sizeof(VoxelPoint), 1, fp);
	}
    ReleaseMutex(mutex);
	fclose(fp);
}

void VoxelMap::readFromFile(const char* fileName) {
	FILE *fp;
	int l = strlen(fileName);
	
	if (!((l > 3) && (fileName[l - 3] == '.')
        && (fileName[l - 2] == 'v')
        && (fileName[l - 1] == 'm')))
		return;

	if(fopen_s(&fp, fileName, "rb"))
		return;
	
    WaitForSingleObject(mutex, INFINITE);
	map.clear();
	VoxelPoint point;
	while (true) {
		if(fread(&point, sizeof(VoxelPoint), 1, fp) != 1) {
			break;
		}
		map.push_back(point);
	}
    ReleaseMutex(mutex);

	fclose(fp);
}

void VoxelMap::convertToCubeState(float scale, CubeState *state) {
	state->clear();
    INT8 i, j, k;
	GLTVector3 v, out;
    UINT8 mas[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE];

    memset(mas, 0, sizeof(mas));
    
    WaitForSingleObject(mutex, INFINITE);
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++) {
        // add rotation center
		v[0] = it->x + xCor;
		v[1] = it->y + yCor;
		v[2] = it->z + zCor;
        // rotate voxel
		gltRotateVector(v, curRotTransform, out);
        i = (INT8)floor((out[0] + xShift) * scale + CUBE_CENTER);
		j = (INT8)floor((out[1] + yShift) * scale + CUBE_CENTER);
		k = (INT8)floor((out[2] + zShift) * scale + CUBE_CENTER);
		if ((i | j | k) & MARGIN_MASK) // if out of margin
			continue;
		++mas[i][j][k];
	}
    ReleaseMutex(mutex);

	smoothing(mas);
	for (i = 0; i < 8; ++i)
		for (j = 0; j < 8; ++j)
			for (k = 0; k < 8; ++k) {
				if (mas[i][j][k] > findLocalMax(mas, i, j, k) * 0.5)
					state->set(i, k, j);
			}
}

char VoxelMap::findLocalMax(UINT8 mas[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE], UINT8 x, UINT8 y, UINT8 z) {
	char max = 0;
	char i, j, k;
	for (i = -1; i < 2; ++i) {
        if ((x + i) & MARGIN_MASK)
			continue;
		for (j = -1; j < 2; ++j) {
			if ((y + j) & MARGIN_MASK)
				continue;
			for (k = -1; k < 2; ++k) {
				if ((z + k) & MARGIN_MASK)
					continue;
				if (max < mas[x + i][y + j][z + k])
					max = mas[x + i][y + j][z + k];
			}
		}
	}
	return max;
}

// limit maximum value in mas by 70%
void VoxelMap::smoothing(UINT8 mas[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE]) {
	char max = 0;
	char i, j, k;
	for (i = 0; i < CUBE_SIZE; ++i)
		for (j = 0; j < CUBE_SIZE; ++j)
			for (k = 0; k < CUBE_SIZE; ++k)
				if (mas[i][j][k] > max)
					max = mas[i][j][k];
	max *= 0.7;
	if (max == 0)
		return;
	for (i = 0; i < CUBE_SIZE; ++i)
		for (j = 0; j < CUBE_SIZE; ++j)
			for (k = 0; k < CUBE_SIZE; ++k)
				if (mas[i][j][k] > max)
					mas[i][j][k] = max;
}

void VoxelMap::glTransformMatrix() {
	glScalef(scale, scale, scale);
	glTranslatef(xShift, yShift, zShift);
	glMultMatrixf(curRotTransform);
	glTranslatef(xCor, yCor, zCor);
}

// set rotation center to the center of mass
void VoxelMap::correctCenterMas() {
    WaitForSingleObject(mutex, INFINITE);

	if (!map.size())
		return;
	xCor = yCor = zCor = 0.5;
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++) {
		xCor -= it->x;
		yCor -= it->y;
		zCor -= it->z;
	}
	xCor /= map.size();
	yCor /= map.size();
	zCor /= map.size();
    
    ReleaseMutex(mutex);
}

// set rotation center to the center of the rectangle, that contains voxels
void VoxelMap::correctCenter() {
    WaitForSingleObject(mutex, INFINITE);
	
    if (!map.size())
		return;
	float maxX = map[0].x, maxY = map[0].y, minX = map[0].x, minY = map[0].y;
	xCor = yCor = 0.0;
	for (vector<VoxelPoint>::iterator it = map.begin(); it != map.end(); it++) {
		if (it->x > maxX)
			maxX = it->x;
		if (it->y > maxY)
			maxY = it->y;
		if (it->x < minX)
			minX = it->x;
		if (it->y < minY)
			minY = it->y;
	}
	xCor = -(maxX + minX) / 2;
	yCor = -(maxY + minY) / 2;
    
    ReleaseMutex(mutex);
}

void VoxelMap::rotate(short dx, short dy) {
	float a, x = dx, y = dy;
	GLTMatrix temp1, temp2;
	a = sqrt(x * x + y * y) * angelAdjust;

	gltRotationMatrix(a, y, x, 0.0, temp1);
	gltMultiplyMatrix(temp1, curRotTransform, temp2);
	gltCopyMatrix(temp2, curRotTransform);
}

void VoxelMap::clear() {
	xCor = yCor = zCor = 0.0;
	gltLoadIdentityMatrix(curRotTransform);
	xShift = yShift = 0.0;
	zShift = 0.0;
    WaitForSingleObject(mutex, INFINITE);
	map.clear();
    ReleaseMutex(mutex);
}

// create a voxel letter
void VoxelMap::createLetter(char c) {
	float d = 20;
	int xCenter = 522, yCenter = 301;
    
    WaitForSingleObject(mutex, INFINITE);
	map.clear();

	for (int i = -20; i < 20; ++i) {
		for (int j = -17; j < 15; ++j) {
			if (processPoint(xCenter + d * i, yCenter - d * j, c)) {
				map.push_back(VoxelPoint(i, j, -2));
			}
		}
	}
    ReleaseMutex(mutex);
}

bool VoxelMap::processPoint(int xPos, int yPos, char c) {
	// Space for selection buffer
	static GLuint selectBuff[16];

	// Hit counter and viewport storage
	GLint hits, viewport[4];

	// Setup selection buffer
	glSelectBuffer(16, selectBuff);
	
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
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -15.0f);
		glScalef(0.3, 0.3, 0.3);
		glTranslatef(-7.0, -6.4, 0);
		glScalef(20.0, 20.0, 20.0);	
		glListBase(nFontList);
		glCallLists(1, GL_UNSIGNED_BYTE, &c);
	glPopMatrix();

	// Collect the hits
	hits = glRenderMode(GL_RENDER);

	// Restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Go back to modelview for normal rendering
	glMatrixMode(GL_MODELVIEW);

	return hits != 0;
}
