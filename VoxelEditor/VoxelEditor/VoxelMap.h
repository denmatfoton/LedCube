#pragma once
#include "GLTools.h"
#include <vector>
#include <fstream>
#include "CubeState.h"
typedef GLfloat GLTMatrix[16];      // A column major 4x4 matrix of type GLfloat
using namespace std;

extern GLfloat fAspect;
struct VoxelPoint;

#define CUBE_SIZE   (8)  // must be power of 2
#define CUBE_CENTER (CUBE_SIZE / 2)
#define MARGIN_MASK (~(CUBE_SIZE - 1))

class VoxelMap {
public:
	VoxelMap(void);
	~VoxelMap(void);
	void addVoxel(VoxelPoint);
	void deleteVoxel(VoxelPoint);
	void draw();
	void virtualDraw();
	void saveToFile(char*);
	void readFromFile(const char*);
	VoxelPoint getSelector(int, int);
	void convertToCubeState(float, CubeState*);
	void glTransformMatrix();
	void correctCenterMas();
	void rotate(short, short);
	void altXShift(float d) {xShift += d;}
	void altYShift(float d) {yShift += d;}
	void altZShift(float d) {zShift += d;}
	void createLetter(char);
	void correctCenter();
	void clear();
private:
	vector<VoxelPoint> map;
	void solidCube(int);
	float xCor, yCor, zCor; // point of rotation center
	float xShift, yShift, zShift; // shift after rotation
	GLTMatrix curRotTransform;
	bool processPoint(int, int, char);
	char findLocalMax(UINT8 mas[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE], UINT8, UINT8, UINT8);
	void smoothing(UINT8 mas[CUBE_SIZE][CUBE_SIZE][CUBE_SIZE]);
    HANDLE mutex;
};
