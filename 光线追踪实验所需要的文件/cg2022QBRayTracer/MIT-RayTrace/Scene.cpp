#include "stdafx.h"
#include "Scene.h"

#include <cmath>
//#include "Vector.h"

CCgScene::CCgScene(int32 num_objects)
{
	size = num_objects;
	objects = new SceneObject*[size];
	for (int i = 0; i < size; i++) objects[i] = nullptr;
}

CCgScene::~CCgScene()
{
	for (int i = 0; i < size; i++) {
		if (objects[i] != nullptr) {
			delete objects[i];
		}
	}
	delete[] objects;
}

// intersects with all objects in the scene (except the given excluded object if its not null)
// returns the point hit and the surface normal
void CCgScene::intersect(Vector &ray_source, Vector &ray, SceneObject *exclude,
	                     Vector *final_result, Vector *result_normal, SceneObject **hit_object, double dt)
{
	double nearest = 1024 * 1024;
	SceneObject *nearest_obj = nullptr;
	Vector result(0, 0, 0);
	Vector normal(0, 0, 0);
	for (int i = 0; i < size; i++) {
		if (objects[i] == exclude) {
			continue;
		}
		if (objects[i]->intersect(&ray_source, &ray, &result, &normal, dt)) {
			double dist = (result.x - ray_source.x) * (result.x - ray_source.x);
			dist += (result.y - ray_source.y) * (result.y - ray_source.y);
			dist += (result.z - ray_source.z) * (result.z - ray_source.z);
			if (dist < nearest) {
				nearest = dist;
				nearest_obj = objects[i];
				final_result->set(result.x, result.y, result.z);
				result_normal->set(normal.x, normal.y, normal.z);
			}
		}
	}
	*hit_object = nearest_obj;
}

SphereObject::SphereObject(double x0, double y0, double z0, double r0, 
	                       uint32 col, double d, double s, double t, double a) 
{
	x = x0;
	y = y0;
	z = z0;
	radius = r0;
	red = ((col >> 16) & 0xFF) / 255.0f;
	green = ((col >> 8) & 0xFF) / 255.0f;
	blue = ((col) & 0xFF) / 255.0f;
	absorb_chance = a;
	diffuse_chance = d;
	specular_chance = s;
	transmission_chance = t;
	refraction = 0;
	specular_coeff = 0;
	dx = 0;
	dy = 0;
	dz = 0;
}

SphereObject::SphereObject(double x0, double y0, double z0, double r0, 
	                       uint32 col, double d, double s, double t, double a, 
	                       double dx0, double dy0, double dz0)
{
	x = x0;
	y = y0;
	z = z0;
	radius = r0;
	red = ((col >> 16) & 0xFF) / 255.0f;
	green = ((col >> 8) & 0xFF) / 255.0f;
	blue = ((col) & 0xFF) / 255.0f;
	absorb_chance = a;
	diffuse_chance = d;
	specular_chance = s;
	transmission_chance = t;
	refraction = 0;
	specular_coeff = 0;
	dx = dx0;
	dy = dy0;
	dz = dz0;
}

bool SphereObject::intersect(Vector *ray_source, Vector *ray, Vector *result, Vector *result_normal, double dt) {
	// A geometric intersection solution
	// described here: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
	double x0 = x + dt * dx;
	double y0 = y + dt * dy;
	double z0 = z + dt * dz;
	// check that we're casting in the right direction
	Vector l(x0 - ray_source->x, y0 - ray_source->y, z0 - ray_source->z);
	double b = ray->dot(&l);
	if (b < 0) {
		return false;
	}
	double d2 = l.dot(&l) - b  * b;
	if (d2 > radius * radius) {
		return false;
	}
	double t = std::sqrt(radius * radius - d2);
	if (t < 0) {
		t = b + t;
	} else {
		t = b - t;
	}
	if (t < 0) {
		return false;
	}
	result->set(ray->x * t + ray_source->x, ray->y * t + ray_source->y, ray->z * t + ray_source->z);
	result_normal->set(result->x - x0, result->y - y0, result->z - z0);
	result_normal->normalize();
	return true;
}

PlaneObject::PlaneObject(double x0, double y0, double z0, 
	                     double min, double max, uint32 col, double d, double s, double t, double a)
{
	x = x0;
	y = y0;
	z = z0;
	red = ((col >> 16) & 0xFF) / 255.0f;
	green = ((col >> 8) & 0xFF) / 255.0f;
	blue = ((col) & 0xFF) / 255.0f;
	absorb_chance = a;
	diffuse_chance = d;
	specular_chance = s;
	transmission_chance = t;
	refraction = 0;
	max_bound = max;
	min_bound = min;
	specular_coeff = 0;
}

bool PlaneObject::intersect(Vector *camera, Vector *ray, Vector *result, Vector *normal, double dt) 
{
	if (x != 0) {
		// find the distance from the source to the plane in the normal of the plane
		double dx = x - camera->x;
		double mul = dx / ray->x;
		if (mul < 0) {
			return false;
		}
		// find the distance traveled in the other two dimensions to
		// reach the plane
		double hz = camera->z + mul * ray->z;
		if (hz < -0.01) {
			return false;
		}
		double hy = camera->y + mul * ray->y;
		// check that we're within the bounds of the plane
		if (hy > max_bound || hy < min_bound) {
			return false;
		}
		result->set(x, hy, hz);
		normal->set(x < 0 ? 1 : -1, 0, 0);
		return true;
	} else if (y != 0) {
		double dy = y - camera->y;
		double mul = dy / ray->y;
		if (mul < 0) {
			return false;
		}
		double hz = camera->z + mul * ray->z;
		if (hz < -0.01) {
			return false;
		}
		double hx = camera->x + mul * ray->x;
		if (hx < min_bound || hx > max_bound) {
			return false;
		}
		result->set(hx, y, hz);
		normal->set(0, y < 0 ? 1 : -1, 0);
		return true;
	} else if (z > 0) {
		double dz = z - camera->z;
		double mul = dz / ray->z;
		if (mul < 0) {
			return false;
		}
		double hx = camera->x + mul * ray->x;
		if (hx < -5 || hx > 5) {
			return false;
		}
		double hy = camera->y + mul * ray->y;
		if (hy < min_bound || hy > max_bound) {
			return false;
		}
		result->set(hx, hy, z);
		normal->set(0, 0, z < 0 ? 1 : -1);
		return true;
	}
	return false;
}
