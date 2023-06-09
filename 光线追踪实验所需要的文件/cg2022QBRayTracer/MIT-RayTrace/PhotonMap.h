#pragma once

#include "Vector.h"
#include "Scene.h"

struct photon {
	float x, y, z;
	uint8 power[4];
	float dx, dy, dz;
	int8 bounce;
};

struct kdnode {
	photon *value;
	kdnode *left;
	kdnode *right;
	Axis splitting_axis;
};

class CCgPhotonMap
{
public:
	CCgPhotonMap();
	~CCgPhotonMap();

	photon *findMedianPhoton(photon** photons, int size, Axis axis);
	kdnode *createKDTree(photon **photons, int size, photon **scratch, int scratch_size);
	void insert(photon **nearest, double *distances, int k, int size, photon *next, double dist);
	int find_nearest_photons(photon **nearest, double *distances, int k, int size, Vector *target, kdnode *root, double max_dist);
	void showPhotons(uint32 *pane, kdnode *tree);

	kdnode *createPhotonMap(int32 photon_count, Vector &light_source, Vector &light_color, CCgScene *scene);
	kdnode *createCausticPhotonMap(int32 photon_count, Vector &light_source, Vector &light_color, CCgScene *scene);

	void deleteTree(kdnode *tree);
};

