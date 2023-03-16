#pragma once

class CCgSmallptPPM
{
public:
	CCgSmallptPPM();
	~CCgSmallptPPM();

	int smallptPPMRayTrace(int width, int height, unsigned int *bmpImage, int samples, int threadNum);
};

