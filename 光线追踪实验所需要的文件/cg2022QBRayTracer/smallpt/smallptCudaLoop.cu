
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "cutil_math.h"

// limited version of checkCudaErrors from helper_cuda.h in CUDA examples
#define checkCudaErrors(val) cudaCheck( (val), #val, __FILE__, __LINE__ )

void cudaCheck(cudaError_t result, char const *const func, const char *const file, int const line)
{
	if (result) {
		const char *errorName, *errorString;

		errorName = cudaGetErrorName(result);
		errorString = cudaGetErrorString(result);

		// Make sure we call CUDA Device Reset before exiting
		cudaDeviceReset();

		system("Pause");

	}
}

#define M_PI 3.14159265359f  
#define width 512  
#define height 384 
#define samples 512

struct Ray {
	float3 origin;
	float3 direction;
	__device__ Ray(float3 o, float3 d) : origin(o), direction(d) {}
};

enum Material { Diffuse, Specular, Refraction };

struct Sphere {

	float radius;
	float3 position, emission, color;
	Material material;

	__device__ float intersect_sphere(const Ray &r) const {

		float3 op = r.origin - position;
		float t, epsilon = 0.0001f;           // epsilon required to prevent floating point precision artefacts
		float b = dot(op, r.direction);       // b in quadratic equation
											  // discriminant quadratic equation
		float discriminant = b * b - dot(op, op) + radius * radius;
		                                      // if (disc < 0) no real solution (not interested in complex roots) 
		if (discriminant < 0)  return 0;      // else check for solutions using negative and positive discriminant
		else discriminant = sqrtf(discriminant);

		t = -b - discriminant;                // pick closest point in front of ray origin
		if (t > epsilon)	return t;
		else	{
			t = -b + discriminant;
			if (t > epsilon)	return t;
			else				return 0;
		}
	}
};

// Scene
__constant__ Sphere spheres[] = {
	{ 1e5f,{ 1e5f + 1.0f,   40.8f, 81.6f },{ 0.0f, 0.0f, 0.0f },{ 0.75f, 0.25f, 0.25f }, Diffuse },   // Left 
	{ 1e5f,{ -1e5f + 99.0f, 40.8f, 81.6f },{ 0.0f, 0.0f, 0.0f },{ 0.25f, 0.25f, 0.75f }, Diffuse },   // Right 
	{ 1e5f,{         50.0f, 40.8f,  1e5f },{ 0.0f, 0.0f, 0.0f },{ 0.75f, 0.75f, 0.75f }, Diffuse },   // Back 
	{ 1e5f,{ 50.0f, 40.8f, -1e5f + 600.0f},{ 0.0f, 0.0f, 0.0f },{ 1.00f, 1.00f, 1.00f }, Diffuse },   // Front 
	{ 1e5f,{ 50.0f,  1e5f,         81.6f },{ 0.0f, 0.0f, 0.0f },{ 0.75f, 0.75f, 0.75f }, Diffuse },   // Bottom 
	{ 1e5f,{ 50.0f, -1e5f + 81.6f, 81.6f },{ 0.0f, 0.0f, 0.0f },{ 0.75f, 0.75f, 0.75f }, Diffuse },   //Top 
	{ 16.5f,{ 27.0f, 16.5f, 47.0f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f },   Specular }, // small glass sphere 1
	{ 16.5f,{ 73.0f, 16.5f, 78.0f },{ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f }, Refraction }, // small glass sphere 2
    { 10.5f,{ 50.0f, 46.5f, 90.0f}, { 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f }, Diffuse },    // small white sphere 3
	{ 600.0f,{ 50.0f, 681.6f - .77f, 81.6f },{ 2.0f, 1.8f, 1.6f },{ 0.0f, 0.0f, 0.0f }, Diffuse }  // Light
};

__device__ inline bool intersect_scene(const Ray &r, float &t, int &id) 
{

	float n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;

	for (int i = int(n); i--;)
		if ((d = spheres[i].intersect_sphere(r)) && d < t) {
			t = d;
			id = i;
		}
	// returns true if an intersection with the scene occurred, false when no hit
	return t < inf;
}

__device__ static float getrandom(unsigned int *seed0, unsigned int *seed1) 
{
	*seed0 = 36969 * ((*seed0) & 65535) + ((*seed0) >> 16);
	*seed1 = 18000 * ((*seed1) & 65535) + ((*seed1) >> 16);

	unsigned int ires = ((*seed0) << 16) + (*seed1);

	union {
		float f;
		unsigned int ui;
	} res;

	res.ui = (ires & 0x007fffff) | 0x40000000;

	return (res.f - 2.f) / 2.f;
}

__device__ inline void maskWeight(float3& mask, float3 color, float cosine, float scale)
{
	mask *= color;
	mask *= cosine;
	mask *= 2;
}

__device__ float3 loopRadiance(Ray &r, unsigned int *s1, unsigned int *s2)
{
	int depth = 0;
	float3 mask = make_float3(1.0f, 1.0f, 1.0f);
	float3 accucolor = make_float3(0.0f, 0.0f, 0.0f);

	// ray bounce loop 8 Times
	while (depth++ < 8) {
		float t;
		int id = 0;

		// find closest intersection with object's index, if miss break.
		if (!intersect_scene(r, t, id))	break;

		const Sphere &obj = spheres[id];
		float3 x = r.origin + r.direction*t;                 // hitpoint
		float3 n = normalize(x - obj.position);              // normal
		float3 nl = dot(n, r.direction) < 0 ? n : n * -1;    // front facing normal
														 
		r.origin = x + nl * 0.05f;                           // prevent self-intersection	
		accucolor += mask * obj.emission;                    // emissive

		if (obj.material == Diffuse) {    //diffuse
            // uniform sampling hemisphere
			float r1 = 2 * M_PI * getrandom(s1, s2);
			float r2 = getrandom(s1, s2);
			float r2s = sqrtf(r2);

			// compute local coordinate on the hit point
			float3 w = nl;
			float3 u = normalize(cross((fabs(w.x) > .1 ? make_float3(0, 1, 0) : make_float3(1, 0, 0)), w));
			float3 v = cross(w, u);

			float3 d = normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrtf(1 - r2));

			r.direction = d;
			r.origin = x + nl * 0.05f;              //offset for self intersection
            // weigh light contribution using cosine of angle between
			// incident light and normal fudge factor
			maskWeight(mask, obj.color, dot(d, nl), 2);
		} else if (obj.material == Specular) {  //specular
			r.direction = r.direction - n * 2 * dot(n, r.direction);
			r.origin = x + r.direction * 0.07f;

			maskWeight(mask, obj.color, dot(r.direction, nl), 2);
		} else { //refraction
			double n1, n2, n3;
			double cosI = dot(n, r.direction);
			if (cosI > 0.0) {
				n1 = 1.5;	n2 = 1.0;	n = -n;
			} else {
				n1 = 1.0;	n2 = 1.5;	cosI = -cosI;
			}
			n3 = n1 / n2;
			double sinT2 = n3 * n3*(1.0 - cosI * cosI);
			double cosT = sqrt(1.0 - sinT2);
			//fernesel equations
			double rn = (n1*cosI - n2 * cosT) / (n1*cosI + n2 * cosT);
			double rt = (n2*cosI - n1 * cosT) / (n2*cosI + n2 * cosT);
			rn *= rn;
			rt *= rt;
			double refl = (rn + rt)*0.5;
			double trans = 1.0 - refl;
			if (n3 == 1.0) {
				maskWeight(mask, obj.color, dot(r.direction, nl), 2);
			}
			//total internal reflection
			if (cosT*cosT < 0.0) {
				r.origin = x + nl * 0.07;
				r.direction = r.direction - n * 2 * dot(n, r.direction);

				maskWeight(mask, obj.color, dot(r.direction, nl), 2);
			}
			else { //refracton
				r.direction = n3 * r.direction + (n3*cosI - cosT)*n;
				r.origin = x + r.direction * 0.07;
				maskWeight(mask, obj.color, dot(r.direction, nl), 2);
			}
		}
	}

	return accucolor;
}

inline  __host__ __device__ float clamp(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

inline  __host__ __device__ int   toInt(float x) { return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5); }

__global__ void smallptRenderkernel(float3 *output, unsigned int outputSize)
{
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if ((x >= width) || (y >= height)) return;

	unsigned int i = (height - y - 1)*width + x;

	if (i >= outputSize) return;

	unsigned int s1 = x;
	unsigned int s2 = y;

	float3 r = make_float3(0.0f);
	Ray cam(make_float3(50.0, 52.0, 295.6), normalize(make_float3(0.0, -0.042612, -1)));
	float3 cx = make_float3(width * 0.5135 / height, 0.0f, 0.0f);
	float3 cy = normalize(cross(cx, cam.direction)) * 0.5135;

	for (int s = 0; s < samples; s++) {

		float3 d = cam.direction + cx * ((0.25 + x) / width - 0.5) + cy * ((0.25 + y) / height - 0.5);

		r = r + loopRadiance(Ray(cam.origin + d * 40, normalize(d)), &s1, &s2)*(1.0 / samples);
	}

	output[i] = make_float3(clamp(r.x, 0.0f, 1.0f), clamp(r.y, 0.0f, 1.0f), clamp(r.z, 0.0f, 1.0f));
}

extern "C"  int smallpt1RayTrace(unsigned int *sample)
{

	// get number of SMs on this GPU
//	int devID = 0;
//	cudaDeviceProp deviceProps;
//	checkCudaErrors(cudaGetDeviceProperties(&deviceProps, devID));

	cudaDeviceReset();

	cudaError_t cudaStatus;
	// Choose which GPU to run on
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) return 0;

	float3* output_d;
	float3* output_h = new float3[width*height];
	
	// Allocate GPU buffers for outout image
	cudaStatus = cudaMalloc(&output_d, width * height * sizeof(float3));
	if (cudaStatus != cudaSuccess) return 0;

	dim3 block(8, 8, 1);
	dim3 grid(width / block.x, height / block.y, 1);

	// Launch a kernel on the GPU with one thread for each element.
	smallptRenderkernel<<< grid, block >>>(output_d, width * height);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) return 0;

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) return 0;

	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(output_h, output_d, width * height * sizeof(float3), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) return 0;

	for (int i = 0; i < width*height; i++) {
		unsigned int red = 0;
		unsigned int green = 0;
		unsigned int blue = 0;

		  red   = toInt(output_h[i].x);
		  green = toInt(output_h[i].y);
		  blue  = toInt(output_h[i].z);

		  sample[i] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
	}

	delete[] output_h;
	cudaFree(output_d);

	return 1;
}
