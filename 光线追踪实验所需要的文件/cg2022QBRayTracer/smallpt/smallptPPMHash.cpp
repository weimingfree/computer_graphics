#include<stdafx.h>
#include "smallptPPM.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

CCgSmallptPPM::CCgSmallptPPM()
{
}

CCgSmallptPPM::~CCgSmallptPPM()
{
}

class _Vector_
{
public:
	double _x, _y, _z;

	_Vector_(double x = 0, double y = 0, double z = 0) :_x(x), _y(y), _z(z) {}

	inline _Vector_ operator+(const _Vector_ &b)const {
		return _Vector_(_x + b._x, _y + b._y, _z + b._z);
	};
	inline _Vector_ operator-(const _Vector_ &b)const {
		return _Vector_(_x - b._x, _y - b._y, _z - b._z);
	};
	inline _Vector_ operator*(double b) const {
		return _Vector_(_x * b, _y * b, _z * b);
	};
	inline _Vector_ mult(const _Vector_ &b)const {
		return _Vector_(_x * b._x, _y * b._y, _z * b._z);
	};
	// 向量标准化
	inline _Vector_ normalize() {
		double t = sqrt(_x * _x + _y * _y + _z * _z);//向量的模
		if (t == 0) return *this;
		*this = this->operator*(1.0 / t);
		return *this;
	};
	//点乘
	inline double dot(const _Vector_ &b)const {
		return _x * b._x + _y * b._y + _z * b._z;
	};
	//叉乘
	inline _Vector_ cross(const _Vector_ &b)const {
		return _Vector_(_y*b._z - _z*b._y, _z*b._x - _x*b._z, _x*b._y - _y*b._x);
	};
};

// Axis Aligned Bounding Box
class BoundingBox
{
public:
	inline void fit(const _Vector_ &point) {
		if (point._x < min._x) min._x = point._x; // min
		if (point._y < min._y) min._y = point._y; // min
		if (point._z < min._z) min._z = point._z; // min
		if (point._x > max._x) max._x = point._x; // max
		if (point._y > max._y) max._y = point._y; // max
		if (point._z > max._z) max._z = point._z; // max
	};

	inline void reset() {
		min = _Vector_(1e20, 1e20, 1e20);
		max = _Vector_(-1e20, -1e20, -1e20);
	};

	_Vector_ min;
	_Vector_ max;
};

class HitPoint
{
public:
	int pixel;
	unsigned int n;
	double radius_squared;
	_Vector_ color, position, normal, flux;
};

class HitpointList
{
public:
	HitPoint *data;
	HitpointList *next;
};

// Halton sequence with reverse permutation
static int primes[61] = {
	2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,
	83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,
	191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283
};

static double hashScale;
static unsigned int numHash;
static HitpointList **hashgridList;
static HitpointList *hitpointList = NULL;
static BoundingBox hitpointBoundbox;

inline static HitpointList* HitpointListAdd(HitPoint *item, HitpointList* head)
{
	HitpointList* p = new HitpointList;
	p->data = item;
	p->next = head;

	return p;
}

inline static int rev(const int i, const int p)
{
	if (i == 0)	return i;
	else		return p - i;
}

static double genHalton(const int b, int j)
{
	const int p = primes[b];
	double h = 0.0;
	double f = 1.0 / (double)p;
	double fct = f;
	while (j > 0) {
		h += rev(j % p, p) * fct;
		j /= p;
		fct *= f;
	}
	return h;
}

// Spatial hash function
inline static unsigned int hash(const int ix, const int iy, const int iz)
{
	return (unsigned int)((ix * 73856093) ^ (iy * 19349663) ^ (iz * 83492791)) % numHash;
}

inline static void buildHashgrid(const int width, const int height)
{
	// Find the bounding box of all the measurement points
	hitpointBoundbox.reset();
	HitpointList *hpList = hitpointList;

	while (hpList != NULL)	{
		HitPoint *hitpoint = hpList->data;
		hpList = hpList->next;
		hitpointBoundbox.fit(hitpoint->position);
	}

	// Initial radius calculation
	_Vector_ bboxSize = hitpointBoundbox.max - hitpointBoundbox.min;
	double initial_radius = ((bboxSize._x + bboxSize._y + bboxSize._z) / 3.0) / ((width + height) / 2.0) * 2.0 * 4.0;

	// Determine hash table size, Find the bounding box of all the measurement points.
	// This time inflated by the initial radius
	int vphoton = 0;
	hpList = hitpointList;
	hitpointBoundbox.reset();
	while (hpList != NULL) {
		HitPoint *hitpoint = hpList->data;
		hpList = hpList->next;
		hitpoint->radius_squared = initial_radius * initial_radius;
		hitpoint->n = 0;
		hitpoint->flux = _Vector_();

		vphoton++;
		hitpointBoundbox.fit(hitpoint->position - initial_radius);
		hitpointBoundbox.fit(hitpoint->position + initial_radius);
	}

	// Make each grid cell two times larger than the initial radius
	numHash = vphoton;
	hashScale = 1.0 / (initial_radius*2.0);

	// Build the hash table
	hashgridList = new HitpointList*[numHash];
	for (unsigned int i = 0; i < numHash; i++)	hashgridList[i] = NULL;
	
	hpList = hitpointList;
	while (hpList != NULL)	{
		HitPoint *hitpoint = hpList->data;
		hpList = hpList->next;

		_Vector_ BMin = ((hitpoint->position - initial_radius) - hitpointBoundbox.min) * hashScale;
		_Vector_ BMax = ((hitpoint->position + initial_radius) - hitpointBoundbox.min) * hashScale;

		for (int iz = abs(int(BMin._z)); iz <= abs(int(BMax._z)); iz++)	{
			for (int iy = abs(int(BMin._y)); iy <= abs(int(BMax._y)); iy++)	{
				for (int ix = abs(int(BMin._x)); ix <= abs(int(BMax._x)); ix++) {
					int hv = hash(ix, iy, iz);
					hashgridList[hv] = HitpointListAdd(hitpoint, hashgridList[hv]);
				}
			}
		}
	}
}

class Ray
{
public:
	Ray() {};
	Ray(_Vector_ origin_, _Vector_ direction_) : origin(origin_), direction(direction_) {}

	_Vector_ origin;
	_Vector_ direction;
};

// Material types
enum MaterialType
{
	DIFFUSE,
	SPECULAR,
	REFRACTIVE
};

class Sphere
{
public:
	Sphere(double radius_, _Vector_ position_, _Vector_ color_, MaterialType material_) :
		   radius(radius_), position(position_), color(color_), material(material_) {}

	// Ray-Sphere intersection
	inline double intersect(const Ray &ray) const	{
		_Vector_ op = position - ray.origin;
		double t;
		double b = op.dot(ray.direction);
		double determinant = b*b - op.dot(op) + radius*radius;
		if (determinant < 0) {
			return 1e20;
		} else {
			determinant = sqrt(determinant);
		}
		t = b - determinant;
		if (t > 1e-4) {
			return t;
		} else	{
			t = b + determinant;
			if (t > 1e-4) {
				return t;
			}
		}
		return 1e20;
	}

	double radius;
	_Vector_ position;
	_Vector_ color;
	MaterialType material;
};

Sphere spheres[] =
{ // Scene: radius, position, color, material
	Sphere(1e5, _Vector_( 1e5 +  1, 40.8, 81.6), _Vector_(.75,.25,.25),DIFFUSE), //Left
	Sphere(1e5, _Vector_(-1e5 + 99, 40.8, 81.6), _Vector_(.25,.25,.75),DIFFUSE), //Right
	Sphere(1e5, _Vector_(       50, 40.8,  1e5), _Vector_(.75,.75,.75),DIFFUSE), //Back
	Sphere(1e5, _Vector_(       50, 40.8, -1e5 + 170), _Vector_(),     DIFFUSE), //Front
	Sphere(1e5, _Vector_(       50,  1e5, 81.6), _Vector_(.75,.75,.75),DIFFUSE), //Bottomm
	Sphere(1e5, _Vector_( 50, -1e5 + 81.6,81.6), _Vector_(.75,.75,.75),DIFFUSE), //Top
	Sphere(16.5,_Vector_( 27, 16.5, 47),  _Vector_(1,1,1)*.999, SPECULAR),       //Mirror
	Sphere(16.5,_Vector_( 73, 16.5, 88),  _Vector_(1,1,1)*.999, REFRACTIVE),     //Glass
	Sphere(8.5, _Vector_( 50,  8.5, 60),  _Vector_(1,1,1)*.999, DIFFUSE)         //Middle
};

// Tone mapping and gamma correction
inline static int gamma(double x)
{
	return int(pow(1 - exp(-x), 1 / 2.2) * 255 + .5);
}

// Find the closest intersection
inline static bool intersect(const Ray &ray, double &t, int &object_id)
{
	int N = sizeof(spheres) / sizeof(Sphere);
	double distance;
	const double infinity = 1e20;
	t = infinity;
	for (int i = 0; i < N; i++)	{
		distance = spheres[i].intersect(ray);
		if (distance < t)
		{
			t = distance;
			object_id = i;
		}
	}
	return t < infinity;
}

#define M_PI       3.14159265358979323846   // pi
#define ALPHA ((double)0.7)                 // the alpha parameter of PPM
#define PHOTON_COUNT 8192

// Generate a photon ray from the point light source with Quasi-Monte Carlo
inline static void genPhotonRay(Ray* photon_ray, _Vector_* flux, int photon_id)
{
	*flux = _Vector_(2500, 2500, 2500)*(M_PI*4.0);
	double p = 2.*M_PI*genHalton(0, photon_id);
	double t = 2.*acos(sqrt(1. - genHalton(1, photon_id)));
	double sint = sin(t);
	photon_ray->direction = _Vector_(cos(p)*sint, cos(t), sin(p)*sint);
	photon_ray->origin = _Vector_(50, 60, 85);
}

// HitPoint Pass
inline static void traceCreatePhotonMap(const Ray &ray, int depth, const _Vector_ &attenuation, unsigned int pixel_index)
{
	double t;
	int object_id;

	depth++;
	if (!intersect(ray, t, object_id) || (depth >= 20)) return;

	const Sphere &object = spheres[object_id];

	_Vector_ intersection_point = ray.origin + ray.direction * t;
	_Vector_ normal = (intersection_point - object.position).normalize();

	_Vector_ base_color = object.color;

	// Lambertian
	if (object.material == DIFFUSE)	{
		// Store the measurement point
		HitPoint* hitpoint = new HitPoint;
		hitpoint->color = base_color.mult(attenuation);
		hitpoint->position = intersection_point;
		hitpoint->normal = normal;
		hitpoint->pixel = pixel_index;
		hitpointList = HitpointListAdd(hitpoint, hitpointList);
	} else if (object.material == SPECULAR)	{ // Mirror
		Ray reflection_ray(intersection_point, ray.direction - normal*2.0*normal.dot(ray.direction));
		traceCreatePhotonMap(reflection_ray, depth, base_color.mult(attenuation), pixel_index);
	} else	{                                // Glass
		_Vector_ nl = normal.dot(ray.direction) < 0 ? normal : normal * -1;

		Ray reflection_ray(intersection_point, ray.direction - normal*2.0*normal.dot(ray.direction));
		bool into = (normal.dot(nl)>0.0);

		double air_index = 1.0;
		double refractive_index = 1.5;

		double nnt = into ? air_index / refractive_index : refractive_index / air_index;

		double ddn = ray.direction.dot(nl);
		double cos2t = 1 - nnt*nnt*(1 - ddn*ddn);

		if (cos2t < 0)
			return traceCreatePhotonMap(reflection_ray, depth, attenuation, pixel_index);

		_Vector_ refraction_direction = (ray.direction*nnt - normal*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).normalize();

		double a = refractive_index - air_index;
		double b = refractive_index + air_index;
		double R0 = a*a / (b*b);

		double cosinealpha = into ? -ddn : refraction_direction.dot(normal);
		double c = 1 - cosinealpha;

		double fresnel = R0 + (1 - R0)*c*c*c*c*c;
		Ray refraction_ray(intersection_point, refraction_direction);
		_Vector_ attenuated_color = base_color.mult(attenuation);

		traceCreatePhotonMap(reflection_ray, depth, attenuated_color*fresnel, pixel_index);
		traceCreatePhotonMap(refraction_ray, depth, attenuated_color*(1.0 - fresnel), pixel_index);
	}
}

// Photon Pass
inline static void photonmapTrace(const Ray &ray, int depth, const _Vector_ &flux,
	                              const _Vector_ &attenuation, int photon_id)
{
	double t;
	int object_id;

	depth++;
	int depth3 = depth * 3;

	if (!intersect(ray, t, object_id) || (depth >= 20)) return;

	const Sphere &object = spheres[object_id];

	_Vector_ intersection_point = ray.origin + ray.direction * t;
	_Vector_ normal = (intersection_point - object.position).normalize();

	_Vector_ base_color = object.color;
	double p = (base_color._x > base_color._y && base_color._x > base_color._z) ?
		        base_color._x : (base_color._y > base_color._z) ? base_color._y : base_color._z;

	_Vector_ nl = normal.dot(ray.direction) < 0 ? normal : normal * -1;

    // Lambertian
	if (object.material == DIFFUSE) {
		// Use Quasi-Monte Carlo to sample the next direction
		double r1 = 2.*M_PI*genHalton(depth3 - 1, photon_id);
		double r2 = genHalton(depth3 + 0, photon_id);
		double r2s = sqrt(r2);

		_Vector_ w = nl;
		_Vector_ u = ((fabs(w._x) > .1 ? _Vector_(0, 1) : _Vector_(1)).cross(w)).normalize();
		_Vector_ v = w.cross(u);
		_Vector_ d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2)).normalize();

		// Find neighboring measurement points and accumulate flux via progressive density estimation
		_Vector_ hh = (intersection_point - hitpointBoundbox.min) * hashScale;
		int ix = abs(int(hh._x));
		int iy = abs(int(hh._y));
		int iz = abs(int(hh._z));
		{
			HitpointList* hitpoints = hashgridList[hash(ix, iy, iz)];
			while (hitpoints != NULL) {
				HitPoint *hit_point = hitpoints->data;

				_Vector_ v = hit_point->position - intersection_point;
				if ((hit_point->normal.dot(normal) > 1e-3) && (v.dot(v) <= hit_point->radius_squared)) {
					// Unlike N in the paper, hit_point->n stores "N / ALPHA" to make it an integer value
					double radius_reduction = (hit_point->n * ALPHA + ALPHA) / (hit_point->n * ALPHA + 1.0);
					hit_point->radius_squared = hit_point->radius_squared * radius_reduction;
					hit_point->n++;
					hit_point->flux = (hit_point->flux + hit_point->color.mult(flux)*(1. / M_PI))*radius_reduction;
				}

				hitpoints = hitpoints->next;
			}
		}
		if (genHalton(depth3 + 1, photon_id) < p)
			photonmapTrace(Ray(intersection_point, d), depth, base_color.mult(flux)*(1. / p), attenuation, photon_id);
	} else if (object.material == SPECULAR)	{ // Mirror
		photonmapTrace(Ray(intersection_point, ray.direction - normal*2.0*normal.dot(ray.direction)),
			           depth, base_color.mult(flux), base_color.mult(attenuation), photon_id);
	} else { // Glass
		Ray reflection_ray(intersection_point, ray.direction - normal*2.0*normal.dot(ray.direction));
		bool into = (normal.dot(nl)>0.0);

		double air_index = 1.0;
		double refractive_index = 1.5;

		double nnt = into ? air_index / refractive_index : refractive_index / air_index;

		double ddn = ray.direction.dot(nl);
		double cos2t = 1 - nnt*nnt*(1 - ddn*ddn);

		if (cos2t < 0)
			return photonmapTrace(reflection_ray, depth, flux, attenuation, photon_id);

		_Vector_ refraction_direction = (ray.direction*nnt - normal*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).normalize();

		double a = refractive_index - air_index;
		double b = refractive_index + air_index;
		double R0 = a*a / (b*b);

		double cosinealpha = into ? -ddn : refraction_direction.dot(normal);
		double c = 1 - cosinealpha;

		double fresnel = R0 + (1 - R0)*c*c*c*c*c;
		Ray refraction_ray(intersection_point, refraction_direction);
		_Vector_ attenuated_color = base_color.mult(attenuation);

		double P = fresnel;

		// Photon ray (pick one via Russian roulette)
		if (genHalton(depth3 - 1, photon_id) < P)
			photonmapTrace(reflection_ray, depth, flux, attenuated_color, photon_id);
		else
			photonmapTrace(refraction_ray, depth, flux, attenuated_color, photon_id);
	}
}

//int CCgSmallptPPM::smallptPPMRayTrace(unsigned int *sample, int threadNum)
//{
//	int width = 256;
//	int height = 256;
//	int samples = 128;                           
//	_Vector_ *color = new _Vector_[width * height];
//	
//	// Trace Create PhotonMap
//#pragma omp parallel for num_threads(threadNum) shared(hitpointList)//schedule(dynamic, 1)
//	for (int y = 0; y < height; y++) {  
//        // Shared varibles 
//		Ray camera(_Vector_(50, 48, 295.6), _Vector_(0, -0.042612, -1).normalize());
//		_Vector_ cx(0.5135 * width / height);
//		_Vector_ cy = cx.cross(camera.direction).normalize() * 0.5135;
//
//		for (int x = 0; x < width; x++)	{
//			unsigned int pixel_index = x + y * width;
//			_Vector_ direction = cx * ((x + 0.5) / width - 0.5) + cy * (-(y + 0.5) / height + 0.5) + camera.direction;
//			Ray ray = Ray(camera.origin + direction * 140, direction.normalize());
//
//			traceCreatePhotonMap(ray, 0, _Vector_(1, 1, 1), pixel_index);
//		}
//	}
//
//	// Build the hash table over the measurement points
//	buildHashgrid(width, height);
//
//	// Trace photonmap rays
//#pragma omp parallel for num_threads(threadNum)  shared(hashgridList) //schedule(dynamic, 1)
//	for (int i = 0; i < samples; i++) {
//		_Vector_ flux;
//		Ray photon_ray;
//		int m = PHOTON_COUNT * i;
//		double percentage = 100.0 * (i + 1) / samples;
//
//		for (int j = 0; j < PHOTON_COUNT; j++)	{
//			genPhotonRay(&photon_ray, &flux, m + j);
//			photonmapTrace(photon_ray, 0, flux, _Vector_(1, 1, 1), m + j);
//		}
//
//	}
//
//	// Density estimation
//	HitpointList* hpList = hitpointList;
//	while (hpList != NULL)	{
//		HitPoint* hitpoint = hpList->data;
//		hpList = hpList->next;
//		int i = hitpoint->pixel;
//		color[i] = color[i] + hitpoint->flux*(1.0 / (M_PI*hitpoint->radius_squared*samples*PHOTON_COUNT));
//	}
//
//	// Save the image after tone mapping and gamma correction
//	for (int i = 0; i < width*height; i++) {
//		unsigned int red   = gamma(color[i]._x);
//		unsigned int green = gamma(color[i]._y);
//		unsigned int blue  =  gamma(color[i]._z);
//
//		sample[i] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
//	}
//
//	return 0;
//}