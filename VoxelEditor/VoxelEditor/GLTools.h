#pragma once
#include <windows.h>
#include <gl\glut.h>
#include "VoxelMap.h"
#define pi 3.1415926
#define scaleUnit 1.1
#define maxScale pow(scaleUnit, 10)
#define minScale pow(scaleUnit, -10)
#define angelAdjust 0.01
#define screenAngel 20

typedef GLfloat GLTVector2[2];      // Two component floating point vector
typedef GLfloat GLTVector3[3];      // Three component floating point vector
typedef GLfloat GLTVector4[4];      // Four component floating point vector
typedef GLfloat GLTMatrix[16];      // A column major 4x4 matrix of type GLfloat

void ChangeSize(GLsizei, GLsizei);
void SetupRC(HDC);
void RenderScene();
void ProcessSelection(int, int, int);

struct VoxelPoint {
	VoxelPoint() {
		x = 0;
		y = 0;
		z = 0;
	}
	VoxelPoint(char x, char y, char z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	bool operator==(const VoxelPoint p) {return x == p.x && y == p.y && z == p.z;}
	char x, y, z;
};
class VoxelMap;

extern VoxelPoint selector;
extern VoxelMap map;
extern float scale;
extern bool convertVoxelMap;
extern GLint nFontList;

// vector functions in VectorMath.cpp
void gltAddVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult);
void gltSubtractVectors(const GLTVector3 vFirst, const GLTVector3 vSecond, GLTVector3 vResult);
void gltScaleVector(GLTVector3 vVector, const GLfloat fScale);
GLfloat gltGetVectorLengthSqrd(const GLTVector3 vVector);
GLfloat gltGetVectorLength(const GLTVector3 vVector);
void gltNormalizeVector(GLTVector3 vNormal);
void gltGetNormalVector(const GLTVector3 vP1, const GLTVector3 vP2, const GLTVector3 vP3, GLTVector3 vNormal);
void gltCopyVector(const GLTVector3 vSource, GLTVector3 vDest);
GLfloat gltVectorDotProduct(const GLTVector3 u, const GLTVector3 v);
void gltVectorCrossProduct(const GLTVector3 vU, const GLTVector3 vV, GLTVector3 vResult);
void gltTransformPoint(const GLTVector3 vSrcPoint, const GLTMatrix mMatrix, GLTVector3 vPointOut);
void gltRotateVector(const GLTVector3 vSrcVector, const GLTMatrix mMatrix, GLTVector3 vPointOut);
void gltGetPlaneEquation(GLTVector3 vPoint1, GLTVector3 vPoint2, GLTVector3 vPoint3, GLTVector3 vPlane);
GLfloat gltDistanceToPlane(GLTVector3 vPoint, GLTVector4 vPlane);

//////////////////////////////////////////
// Other matrix functions in matrixmath.cpp
void gltLoadIdentityMatrix(GLTMatrix m);
void gltMultiplyMatrix(const GLTMatrix m1, const GLTMatrix m2, GLTMatrix mProduct );
void gltRotationMatrix(float angle, float x, float y, float z, GLTMatrix mMatrix);
void gltTranslationMatrix(GLfloat x, GLfloat y, GLfloat z, GLTMatrix mTranslate);
void gltScalingMatrix(GLfloat x, GLfloat y, GLfloat z, GLTMatrix mScale);
//void gltMakeShadowMatrix(GLTVector3 vPoints[3], GLTVector4 vLightPos, GLTMatrix destMat);
void gltTransposeMatrix(GLTMatrix mTranspose);
void gltInvertMatrix(const GLTMatrix m, GLTMatrix mInverse);
void gltCopyMatrix(GLTMatrix source, GLTMatrix dest);
