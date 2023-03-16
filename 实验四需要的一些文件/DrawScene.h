#pragma once

#include "TypeDefine.h"
#include "cg2022QB3DTransDoc.h"
#include "cg2022QB3DTransView.h"

class CDrawScene
{
public:
	CDrawScene();
	~CDrawScene();

	CDrawScene(Ccg2022QB3DTransView* pView);

	void DrawScene();

private:
	int      m_objNumber;
	Object_p m_whoObject;           // Object pointer
	Ccg2022QB3DTransView* m_pView;
	float glObjColor[SPACEOBJECTS][3];

	void DrawBackground();
	void DrawSpaceObject();
	void TransSpaceObject();
	void projectSpaceObject();

	void CaculateMatrix();
	void rotateX3Dmatrix(float S, float C);
	void rotateY3Dmatrix(float S, float C);
	void rotateZ3Dmatrix(float S, float C);
	void translate3dMatrix(float dx, float dy, float dz);

	void pRemoveBackFace();

	void pClipSpaceObject();
	int pVisible(float x, float y, float z, int pane);
	int outPut(float x, float y, float z, int *outCount, Gpoint_p polyClip);
	int pLineCrossPane(float sx, float sy, float sz,
		               float px, float py, float pz, int pane);
	int pLineInterSectPane(float sx, float sy, float sz,
		                   float px, float py, float pz,
		                   int pane, int *outCount, Gpoint_p polyClip);

	void pObjectCenter();
	float xCenter, yCenter, zCenter;

	void pLightSpaceBall();
	void pLightSpaceObject();
};

