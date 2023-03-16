
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <curand_kernel.h>
#include <device_functions.h>

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
#define samples 1024

struct Ray {
	float3 origin;
	float3 direction;
	__device__ Ray() {}
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

__constant__ const int nsphere = sizeof(spheres) / sizeof(Sphere);

__device__ float rgbToLuminance(const float3& rgb)
{
	const float cWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
	return cWeight[0] * rgb.x + cWeight[1] * rgb.y + cWeight[2] * rgb.z;
}

__device__ inline bool intersect_scene(const Ray &r, float &t, int &id, Sphere* sheres, int &nsp)
{
	// t is distance to closest intersection, initialise t to a huge number outside scene
	float d, inf = t = 1e20;

	for (int i = nsp; i--;)
		if ((d = spheres[i].intersect_sphere(r)) && d < t) {
			t = d;
			id = i;
		}
	// returns true if an intersection with the scene occurred, false when no hit
	return t < inf;
}

inline __host__ __device__ float clamp(float x) { return x < 0 ? 0 : x>1 ? 1 : x; }

inline __host__ __device__ int toInt(float x) { return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5); }

__device__ float gammaCorrection(float x)
{
	return pow(clamp(x), 1 / 2.2f);
}
__device__ inline void maskWeight(float3& mask, float3 color, float cosine, float scale)
{
	mask *= color;
//	mask *= cosine;
//	mask *= scale;
}

__device__ float3 loopRadiance(Ray &r, curandState* rs, Sphere* pshere, int &nsp)
{
	// accumulates ray colour with each iteration through bounce loop
	int depth = 0;
	float3 mask = make_float3(1.0f, 1.0f, 1.0f);
	float3 accucolor = make_float3(0.0f, 0.0f, 0.0f);

	// ray bounce loop
	while (true) {
		float t;
		int id = 0;

		// find closest intersection with object's index, if miss break.
		if (!intersect_scene(r, t, id, pshere, nsp))	break;

		const Sphere &obj = spheres[id];
		float3 x = r.origin + r.direction*t;                 // hitpoint
		float3 n = normalize(x - obj.position);              // normal
		float3 nl = dot(n, r.direction) < 0 ? n : n * -1;    // front facing normal
														 
		r.origin = x + nl * 0.05f;                           // prevent self-intersection	
		accucolor += mask * obj.emission;                    // emissive

		if (obj.material == Diffuse) {    //diffuse
            // uniform sampling hemisphere
			float r1 = 2 * M_PI * curand_uniform(rs);
			float r2 = curand_uniform(rs);
			float r2s = sqrtf(r2);

			// compute local coordinate on the hit point
			float3 w = nl;
			float3 u = normalize(cross((fabs(w.x) > .1 ? make_float3(0, 1, 0) : make_float3(1, 0, 0)), w));
			float3 v = cross(w, u);

			// local to world convert
			r.origin = x + nl * 0.05f;              // offset for self intersection
			r.direction = normalize(u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrtf(1 - r2));
            // weigh light contribution using cosine of angle between
			// incident light and normal fudge factor
			maskWeight(mask, obj.color, dot(r.direction, nl), 2);
		} else if (obj.material == Specular) {  //specular
			r.direction = r.direction - n * 2 * dot(n, r.direction);
			r.origin = x + r.direction * 0.07f;
			maskWeight(mask, obj.color, dot(r.direction, nl), 2);
		} else { //refraction
			r.origin = x;
			bool into = dot(n, nl) > 0;                 // Ray from outside going in?
			float nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = dot(r.direction, nl), cos2t;
			// Ideal dielectric REFRACTION
			float3 reflectDir = r.direction - n * 2 * dot(n, r.direction);
			
			// total internal reflection
			if ((cos2t = 1 - nnt*nnt*(1 - ddn*ddn)) < 0) {
				r.direction = reflectDir;
				maskWeight(mask, obj.color, dot(r.direction, nl), 2);
			} else {  // refract or reflect
				float3 tdir = normalize(r.direction*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t))));
				float a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : dot(tdir, n));
				float Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
			    if (curand_uniform(rs) < P)	{  // reflect		
				  r.direction = reflectDir;
				  maskWeight(mask, obj.color, dot(r.direction, nl), 2);
				  mask *= RP;
			    } else {		               // refract
				r.direction = tdir;
				maskWeight(mask, obj.color, dot(r.direction, nl), 2);
				mask *= TP;
			}
		  }
		}
		// Russian roulette Stop with at least some probability to avoid getting stuck
		if (depth++ >= 5) {
			float q = min(0.95f, rgbToLuminance(mask));
			if (curand_uniform(rs) >= q)
				break;
			mask /= q;
		}
	}
	return accucolor;
}

__global__ void smallptRenderkernel(float3 *output, unsigned int outputSize)
{
	//copy spheres to shared memory
	__shared__ int nsp;
	__shared__ Sphere sspheres[nsphere];
	__shared__ Ray tRay;

	nsp = nsphere;
	sspheres[threadIdx.x % nsp] = spheres[threadIdx.x % nsp];

	__syncthreads();

	// position of current pixel
	unsigned int x = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y*blockDim.y + threadIdx.y;

	if ((x >= width) || (y >= height)) return;

	// index of current pixel
	//int i = (blockIdx.x + blockIdx.y * gridDim.x) * (blockDim.x * blockDim.y) + (threadIdx.y * blockDim.x) + threadIdx.x;
	unsigned int i = (height - y - 1)*width + x;

	if (i >= outputSize) return;

	curandState rs;
	curand_init(i, 0, 0, &rs);

	float3 r = make_float3(0.0f);
	Ray cam(make_float3(50.0, 52.0, 295.6), normalize(make_float3(0.0, -0.042612, -1)));
	float3 cx = make_float3(width * 0.5135 / height, 0.0f, 0.0f);
	float3 cy = normalize(cross(cx, cam.direction)) * 0.5135;

	for (int sy = 0; sy < 2; sy++)	{
		for (int sx = 0; sx < 2; sx++)	{

			for (int s = 0; s < samples; s++) {
				float r1 = curand_uniform(&rs);
				float dx = r1 < 1 ? sqrtf(r1) - 1 : 1 - sqrtf(2 - r1);
				float r2 = curand_uniform(&rs);
				float dy = r2 < 1 ? sqrtf(r2) - 1 : 1 - sqrtf(2 - r2);
				//--! super sampling
				float3 d = cam.direction + cx*((((sx + dx + .5) / 2) + x) / width - .5) +
					                       cy*((((sy + dy + .5) / 2) + y) / height - .5);

				//Ray tRay = Ray(cam.origin + d * 140, normalize(d));
				tRay.direction = normalize(d);
				tRay.origin = cam.origin + d * 40;
				r += loopRadiance(tRay, &rs, sspheres, nsp) *(.25f / samples);
			}
		}
	}	
	// output to the cache
	__shared__ float3 temp;
	temp = make_float3(clamp(r.x, 0.0f, 1.0f), clamp(r.y, 0.0f, 1.0f), clamp(r.z, 0.0f, 1.0f));
	output[i] = temp;
}

extern "C"  int smallptRayTrace(unsigned int *sample)
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
