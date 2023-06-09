#include "stdafx.h"
#include "PhotonMap.h"

#include <chrono>
#include <cmath>
#include <cstdio>

#include "Random.h"

CCgPhotonMap::CCgPhotonMap()
{
}


CCgPhotonMap::~CCgPhotonMap()
{
}

// A recursive method to find the median photon by only computing a partial sort
photon* CCgPhotonMap::findMedianPhoton(photon** photons, int size, Axis axis)
{
	int k = randutil::nextInt(0, size);     // choose a photon at random
	
	photon *p = photons[k];
	photon** scratch = new photon*[size];
	int less_index = 0;
	int great_index = size - 1;
	double p_val;
	if (axis == X_AXIS) {
		p_val = p->x;
	}
	else if (axis == Y_AXIS) {
		p_val = p->y;
	}
	else {
		p_val = p->z;
	}
	// split the photons into those less and greater than our chosen photon
	for (int i = 0; i < size; i++) {
		photon *next = photons[i];
		if (next == p) {
			continue;
		}
		double val;
		if (axis == X_AXIS) {
			val = next->x;
		}
		else if (axis == Y_AXIS) {
			val = next->y;
		}
		else {
			val = next->z;
		}
		if (val <= p_val) {
			scratch[less_index++] = next;
		}
		else {
			scratch[great_index--] = next;
		}
	}
	photon *r;
	if (less_index == size / 2) {
		// the chosen photon was the median photon
		r = p;
	}
	else if (less_index > size / 2) {
		// more photons were less than our photon so recurse into that side
		r = findMedianPhoton(scratch, less_index, axis);
	}
	else {
		// more photons were greater than our photon so recurse into that side
		r = findMedianPhoton(scratch + great_index + 1, size - great_index - 1, axis);
	}
	delete[] scratch;
	return r;
}

// creates a k dimensional tree from the given array of photons
kdnode* CCgPhotonMap::createKDTree(photon **photons, int size, photon **scratch, int scratch_size)
{
	Vector bmin(1000, 1000, 1000);
	Vector bmax(-1000, -1000, -1000);
	// find the min and max bounds of our photons
	for (int i = 0; i < size; i++) {
		photon *next = photons[i];
		if (next->x < bmin.x) {
			bmin.x = next->x;
		}
		if (next->x > bmax.x) {
			bmax.x = next->x;
		}
		if (next->y < bmin.y) {
			bmin.y = next->y;
		}
		if (next->y > bmax.y) {
			bmax.y = next->y;
		}
		if (next->z < bmin.z) {
			bmin.z = next->z;
		}
		if (next->z > bmax.z) {
			bmax.z = next->z;
		}
	}
	// we split the photons along the largest axis
	double x_size = (bmax.x - bmin.x);
	double y_size = (bmax.y - bmin.y);
	double z_size = (bmax.z - bmin.z);
	Axis split = largestAxis(x_size, y_size, z_size);
	photon *median = findMedianPhoton(photons, size, split);
	double m_val;
	if (split == X_AXIS) {
		m_val = median->x;
	}
	else if (split == Y_AXIS) {
		m_val = median->y;
	}
	else {
		m_val = median->z;
	}
	int less_index = 0;
	int greater_index = size - 1;
	// split the photons into groups less or greater than our median photon
	for (int i = 0; i < size; i++) {
		photon *next = photons[i];
		if (next == median) {
			continue;
		}
		double val;
		if (split == X_AXIS) {
			val = next->x;
		}
		else if (split == Y_AXIS) {
			val = next->y;
		}
		else {
			val = next->z;
		}
		if (val <= m_val) {
			scratch[less_index++] = next;
		}
		else {
			scratch[greater_index--] = next;
		}
	}
	kdnode *node = new kdnode;
	node->splitting_axis = split;
	node->value = median;
	// recurse into the left and right sides if there are any photons left there
	if (less_index != 0) {
		node->left = createKDTree(scratch, less_index, photons, less_index);
	}
	else {
		node->left = nullptr;
	}
	if (greater_index < size - 1) {
		node->right = createKDTree(scratch + greater_index + 1, 
			                       size - (greater_index + 1), photons + (greater_index + 1),
			                       size - (greater_index + 1));
	}
	else {
		node->right = nullptr;
	}
	return node;
}

void CCgPhotonMap::insert(photon **nearest, double *distances, int k, int size, photon *next, double dist)
{
	// inserts a photon into a max-heap in the `nearest` array
	if (size < k) {
		// our heap isn't full so just insert the photon into the next slot
		// and ensure that the heap property is still satisfied
		int i = size;
		nearest[i] = next;
		distances[i] = dist;
		while (i != 0) {
			// heapify up
			int parent = (i - 1) / 2;
			if (distances[parent] >= distances[i]) {
				break;
			}
			double t = distances[parent];
			photon *tp = nearest[parent];
			distances[parent] = distances[i];
			nearest[parent] = nearest[i];
			distances[i] = t;
			nearest[i] = tp;
			i = parent;
		}
	}
	else {
		// otherwise if out photon is closer than the farthest away photon (the root
		// of the max-heap) then replace the root and heapify downwards to ensure the
		// heap property
		if (dist > distances[0]) {
			return;
		}
		distances[0] = dist;
		nearest[0] = next;
		int i = 0;
		while (i < 31) {
			// heapify down the largest child node
			if (distances[2 * i + 1] < distances[2 * i + 2]) {
				if (distances[2 * i + 2] > distances[i]) {
					int parent = i;
					i = 2 * i + 2;
					double t = distances[parent];
					photon *tp = nearest[parent];
					distances[parent] = distances[i];
					nearest[parent] = nearest[i];
					distances[i] = t;
					nearest[i] = tp;
				}
				else {
					break;
				}
			}
			else {
				if (distances[2 * i + 1] > distances[i]) {
					int parent = i;
					i = 2 * i + 1;
					double t = distances[parent];
					photon *tp = nearest[parent];
					distances[parent] = distances[i];
					nearest[parent] = nearest[i];
					distances[i] = t;
					nearest[i] = tp;
				}
				else {
					break;
				}
			}
		}
	}

}

// performs a nearest neighbour search of a k-dimensional tree to find the nearest set
// of photons
int CCgPhotonMap::find_nearest_photons(photon **nearest, double *distances, 
	                                   int k, int size, Vector *target, kdnode *root, double max_dist) 
{
	double dx = root->value->x - target->x;
	double dy = root->value->y - target->y;
	double dz = root->value->z - target->z;
	double dist = dx * dx + dy * dy + dz * dz;
	if (dist < max_dist) {
		if (size < k) {
			insert(nearest, distances, k, size, root->value, dist);
			size++;
		}
		else if (dist < distances[0]) {
			insert(nearest, distances, k, size, root->value, dist);
		}
	}

	// determine which side of the splitting plane we are on
	// true is positive side
	bool side = true;
	double axis_dist;
	if (root->splitting_axis == X_AXIS) {
		side = target->x <= root->value->x;
		axis_dist = dx * dx;
	}
	else if (root->splitting_axis == Y_AXIS) {
		side = target->y <= root->value->y;
		axis_dist = dy * dy;
	}
	else if (root->splitting_axis == Z_AXIS) {
		side = target->z <= root->value->z;
		axis_dist = dz * dz;
	}

	// recurse into that side first
	kdnode *side_node = side ? root->left : root->right;
	if (side_node != nullptr) {
		size = find_nearest_photons(nearest, distances, k, size, target, side_node, max_dist);
	}

	// check if the current max distance is larger than the distance from the target to the splitting plane
	if (size == 0 || distances[0] > axis_dist) {
		// if yes then recurse into that side as well
		side_node = !side ? root->left : root->right;
		if (side_node != nullptr) {
			size = find_nearest_photons(nearest, distances, k, size, target, side_node, max_dist);
		}
	}

	return size;
}

// renders the photons approximately to a pane for debugging
void CCgPhotonMap::showPhotons(uint32 *pane, kdnode *tree) {

	double dz = tree->value->z;
	Vector dir(-tree->value->x, -tree->value->y, -12 - tree->value->z);
	dir.normalize();
	dir.mul((dz / dir.z));

	int x0 = fastfloor(dir.x * 142) + 640;
	int y0 = fastfloor(dir.y * 142) + 360;
	pane[x0 + y0 * 1280] = 0xFFFF00FF;

	if (tree->right != nullptr) {
		showPhotons(pane, tree->right);
	}
	if (tree->left != nullptr) {
		showPhotons(pane, tree->left);
	}
}

// creates the global photon map
kdnode* CCgPhotonMap::createPhotonMap(int32 photon_size, Vector &light_source, 
	                                  Vector &light_color, CCgScene *scene)
{
//	printf("Building global photon map from %d photons\n", photon_size);
//	auto start = std::chrono::high_resolution_clock::now();
	int photon_index = 0;
	photon **photons = new photon*[photon_size];
	SceneObject *nearest_obj = nullptr;
	Vector nearest_result(0, 0, 0);
	Vector nearest_normal(0, 0, 0);
	Vector photon_power(light_color);
	// we keep going until we have the desired number of photons in our map
	while (photon_index < photon_size) {
		photon_power.set(&light_color);
		double x0 = randutil::nextDouble() * 2 - 1;
		double z0 = randutil::nextDouble() * 2 + 3;
		double y0 = 4.95;
		light_source.set(x0, y0, z0);
		// direction based on cosine distribution
		// formula for distribution from https://www.particleincell.com/2015/cosine-distribution/
		double sin_theta = sqrt(randutil::nextDouble());
		double cos_theta = sqrt(1 - sin_theta*sin_theta);
		double psi = randutil::nextDouble() * 6.2831853;
		Vector light_dir(sin_theta * cos(psi), -cos_theta, sin_theta * sin(psi));
		light_dir.normalize();
		int bounces = 0;
		SceneObject *exclude = nullptr;
		while (true) {
			bounces++;
			// trace photon

			scene->intersect(light_source, light_dir, exclude, &nearest_result, &nearest_normal, &nearest_obj, 0);
			if (nearest_obj == nullptr) {
				break;
			}
			// we have a hit time to decide whether to reflect, absorb, or transmit
			double chance = randutil::nextDouble();
			if (bounces > 3) {
				// force an absorption if we've already bounced too many times
				chance = 1;
			}
			if (chance < nearest_obj->diffuse_chance + nearest_obj->specular_chance) {
				// specular reflection
				Vector n1(nearest_normal);
				n1.mul(n1.x * light_dir.x + n1.y * light_dir.y + n1.z * light_dir.z);
				n1.mul(2);
				light_source.set(nearest_result.x, nearest_result.y, nearest_result.z);
				light_dir.set(light_dir.x - n1.x, light_dir.y - n1.y, light_dir.z - n1.z);
				light_dir.normalize();
				photon_power.mul(nearest_obj->red, nearest_obj->green, nearest_obj->blue);
				// exclude the object we just hit from the next search so we don't hit it again
				exclude = nearest_obj;
				continue;
			}
			else if (chance < nearest_obj->diffuse_chance + nearest_obj->specular_chance + nearest_obj->transmission_chance) {
				// transmission
				double n = 1 / nearest_obj->refraction;
				double d = nearest_normal.x * light_dir.x + nearest_normal.y * light_dir.y + nearest_normal.z * light_dir.z;
				Vector n1(nearest_normal);
				n1.mul(d);
				n1.set(light_dir.x - n1.x, light_dir.y - n1.y, light_dir.z - n1.z);
				n1.mul(n);
				Vector n2(nearest_normal);
				double s = 1 - (n * n) * (1 - d * d);
				n2.mul(sqrt(s));
				n1.add(-n2.x, -n2.y, -n2.z);
				// n1 is refracted vector
				// we should be able to step a tiny part along our refracted ray to avoid
				// having to exclude the object we just hit allowing us to hit the otherside
				light_source.set(nearest_result.x + n1.x * 0.01, nearest_result.y + n1.y * 0.01, nearest_result.z + n1.z * 0.01);
				light_dir.set(n1.x, n1.y, n1.z);
				light_dir.normalize();
				exclude = nullptr;
				continue;
			}
			else {
				// absorption
				photon *next = new photon;
				next->x = (float)nearest_result.x;
				next->y = (float)nearest_result.y;
				next->z = (float)nearest_result.z;
				next->power[0] = fastfloor(photon_power.x * 0xFF);
				next->power[1] = fastfloor(photon_power.y * 0xFF);
				next->power[2] = fastfloor(photon_power.z * 0xFF);
				next->power[3] = 255;
				next->dx = (float)light_dir.x;
				next->dy = (float)light_dir.y;
				next->dz = (float)light_dir.z;
				next->bounce = bounces;
				photons[photon_index++] = next;
				//printf("photon %.1f %.1f %.1f\n", next->x, next->y, next->z);
			}
			break;
		}
	}
//	auto end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double> duration = (end - start);
//  printf("Global photons traced in %.3fs\n", duration.count());

	// we just store the photons into an array when calculating them
	// and then after we have all the photons we can build the kd-tree
	// which is more efficient that continually trying to balance the kd-tree
	photon **scratch = new photon*[photon_size];
//	printf("Building global photons kd-tree\n");
//	start = std::chrono::high_resolution_clock::now();
	kdnode *global_tree = createKDTree(photons, photon_size, scratch, photon_size);
//	end = std::chrono::high_resolution_clock::now();
//	duration = (end - start);
//	printf("Global photons kd-tree built in %.3fs\n", duration.count());

	delete[] scratch;
	delete[] photons;
	return global_tree;
}

// builds the caustic photon map
// very similar to the global map except we only store photons which have undergone at
// least one reflection or transmission
kdnode* CCgPhotonMap::createCausticPhotonMap(int32 photon_size, Vector &light_source, 
	                                         Vector &light_color, CCgScene *scene)
{
//	printf("Building caustic photon map from %d photons\n", photon_size);
//	auto start = std::chrono::high_resolution_clock::now();
	int photon_index = 0;
	photon **photons = new photon*[photon_size];
	SceneObject *nearest_obj = nullptr;
	Vector nearest_result(0, 0, 0);
	Vector nearest_normal(0, 0, 0);
	Vector photon_power(light_color);
	while (photon_index < photon_size) {
		photon_power.set(&light_color);
		double x0 = randutil::nextDouble() * 2 - 1;
		double z0 = randutil::nextDouble() * 2 + 3;
		double y0 = 4.95;
		light_source.set(x0, y0, z0);
		// direction based on cosine distribution
		// formula for distribution from https://www.particleincell.com/2015/cosine-distribution/
		double sin_theta = sqrt(randutil::nextDouble());
		double cos_theta = sqrt(1 - sin_theta*sin_theta);
		double psi = randutil::nextDouble() * 6.2831853;
		// we cheat a little for this scene and angle the light more down beause we know our objects are below the light
		Vector light_dir(sin_theta * cos(psi), -2 * cos_theta, sin_theta * sin(psi));
		light_dir.normalize();
		int bounces = 0;
		bool specular_bounce = false;
		while (true) {
			bounces++;
			// trace photon

			scene->intersect(light_source, light_dir, nullptr, &nearest_result, &nearest_normal, &nearest_obj, 0);
			if (nearest_obj == nullptr) {
				break;
			}
			// we have a hit time to decide whether to reflect, absorb, or transmit
			double chance = randutil::nextDouble();
			if (bounces > 3) {
				// force an absorption if we've already bounced too many times
				chance = 1;
			}
			if (chance < nearest_obj->diffuse_chance + nearest_obj->specular_chance) {
				// specular reflection
				Vector n1(nearest_normal);
				n1.mul(n1.x * light_dir.x + n1.y * light_dir.y + n1.z * light_dir.z);
				n1.mul(2);
				light_source.set(nearest_result.x, nearest_result.y, nearest_result.z);
				light_dir.set(light_dir.x - n1.x, light_dir.y - n1.y, light_dir.z - n1.z);
				light_dir.normalize();
				specular_bounce = true;
				photon_power.mul(nearest_obj->red, nearest_obj->green, nearest_obj->blue);
				continue;
			}
			else if (chance < nearest_obj->diffuse_chance + nearest_obj->specular_chance + nearest_obj->transmission_chance) {
				// refract
				// Equation from Fundamentals of Computer Graphics 4th edition p 325.
				double n = 1 / nearest_obj->refraction;
				double d = nearest_normal.x * light_dir.x + nearest_normal.y * light_dir.y + nearest_normal.z * light_dir.z;
				Vector n1(nearest_normal);
				n1.mul(d);
				n1.set(light_dir.x - n1.x, light_dir.y - n1.y, light_dir.z - n1.z);
				n1.mul(n);
				Vector n2(nearest_normal);
				double s = 1 - (n * n) * (1 - d * d);
				n2.mul(sqrt(s));
				n1.add(-n2.x, -n2.y, -n2.z);
				// n1 is refracted vector
				// we should be able to step a tiny part along our refracted ray to avoid
				// having to exclude the object we just hit allowing us to hit the otherside
				light_source.set(nearest_result.x + n1.x * 0.01, nearest_result.y + n1.y * 0.01, nearest_result.z + n1.z * 0.01);
				light_dir.set(n1.x, n1.y, n1.z);
				light_dir.normalize();
				specular_bounce = true;
				continue;
			}
			else {
				if (!specular_bounce) {
					// not a caustic, skip and try again
					break;
				}
				// absorption
				photon *next = new photon;
				next->x = (float)nearest_result.x;
				next->y = (float)nearest_result.y;
				next->z = (float)nearest_result.z;
				next->power[0] = fastfloor(photon_power.x * 0xFF);
				next->power[1] = fastfloor(photon_power.y * 0xFF);
				next->power[2] = fastfloor(photon_power.z * 0xFF);
				next->power[3] = 255;
				next->dx = (float)light_dir.x;
				next->dy = (float)light_dir.y;
				next->dz = (float)light_dir.z;
				next->bounce = bounces;
				photons[photon_index++] = next;
				//printf("photon %.1f %.1f %.1f\n", next->x, next->y, next->z);
			}
			break;
		}
	}
//	auto end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double> duration = (end - start);
//	printf("Caustic photons traced in %.3fs\n", duration.count());

	// process photons
	photon **scratch = new photon*[photon_size];
//	printf("Building caustic photon kd-tree\n");
//	start = std::chrono::high_resolution_clock::now();
	kdnode *caustic_tree = createKDTree(photons, photon_size, scratch, photon_size);
//	end = std::chrono::high_resolution_clock::now();
//	duration = (end - start);
//	printf("Caustic photons kd-tree built in %.3fs\n", duration.count());

	delete[] scratch;
	delete[] photons;
	return caustic_tree;
}

// recursively deletes the tree and its children
void CCgPhotonMap::deleteTree(kdnode *tree)
{
	if (tree->left != nullptr) {
		deleteTree(tree->left);
	}
	if (tree->right != nullptr) {
		deleteTree(tree->right);
	}
	delete tree->value;
	delete tree;
}


