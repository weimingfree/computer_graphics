
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <cuda.h>
#include <curand.h>
#include <curand_kernel.h>
#include <cuda_runtime.h>
#include <driver_functions.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <curand_kernel.h>
#include <device_functions.h>

#include "smallptCudaBVH.h"

#define ARRAY_SIZE(array) (sizeof((array))/sizeof((array[0])))
#define BLOCK_SIZE 256
#define CODE_OFFSET (1<<21)
#define CODE_LENGTH (21)
#define INTERSECT_STACK_SIZE (18)
#define RESTRUCT_STACK_SIZE (4)
#define Ci 1.2
#define Cl 0.0
#define Ct 1.0

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

inline __host__ __device__ double clamp(double x) { return x<0 ? 0 : x>1 ? 1 : x; }

inline __host__ __device__ int toInt(double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + .5); }

__device__ double drandom(curandState *s)
{
	double d = curand_uniform_double(s);
	return d;
}

inline __device__ float min2(float a, float b)
{
	return (a < b) ? a : b;
}

inline __device__ float max2(float a, float b)
{
	return (a > b) ? a : b;
}

__device__ void merge_bounds(Bound& b1, Bound& b2, Bound* b3)
{
	b3->min_x = min2(b1.min_x, b2.min_x);
	b3->max_x = max2(b1.max_x, b2.max_x);
	b3->min_y = min2(b1.min_y, b2.min_y);
	b3->max_y = max2(b1.max_y, b2.max_y);
	b3->min_z = min2(b1.min_z, b2.min_z);
	b3->max_z = max2(b1.max_z, b2.max_z);
	return;
}

inline __device__ int intMin(int i, int j)
{
	return (i > j) ? j : i;
}

inline __device__ int intMax(int i, int j)
{
	return (i > j) ? i : j;
}

/**
* Longest common prefix for morton code
*/
inline __device__ int longestCommonPrefix(int i, int j, int len)
{
	if (0 <= j && j < len) {
		return __clz(i ^ j);
	}
	else {
		return -1;
	}
}

/**
* Test if a ray intersect a bound
*/
__device__ bool intersection_bound_test(const Ray &r, Bound& bound)
{
	float t_min, t_max, t_xmin, t_xmax, t_ymin, t_ymax, t_zmin, t_zmax;
	float x_a = 1.0 / r.d.x, y_a = 1.0 / r.d.y, z_a = 1.0 / r.d.z;
	float  x_e = r.o.x, y_e = r.o.y, z_e = r.o.z;

	// calculate t interval in x-axis
	if (x_a >= 0) {
		t_xmin = (bound.min_x - x_e) * x_a;
		t_xmax = (bound.max_x - x_e) * x_a;
	}
	else {
		t_xmin = (bound.max_x - x_e) * x_a;
		t_xmax = (bound.min_x - x_e) * x_a;
	}

	// calculate t interval in y-axis
	if (y_a >= 0) {
		t_ymin = (bound.min_y - y_e) * y_a;
		t_ymax = (bound.max_y - y_e) * y_a;
	}
	else {
		t_ymin = (bound.max_y - y_e) * y_a;
		t_ymax = (bound.min_y - y_e) * y_a;
	}

	// calculate t interval in z-axis
	if (z_a >= 0) {
		t_zmin = (bound.min_z - z_e) * z_a;
		t_zmax = (bound.max_z - z_e) * z_a;
	}
	else {
		t_zmin = (bound.max_z - z_e) * z_a;
		t_zmax = (bound.min_z - z_e) * z_a;
	}

	// find if there an intersection among three t intervals
	t_min = max2(t_xmin, max2(t_ymin, t_zmin));
	t_max = min2(t_xmax, min2(t_ymax, t_zmax));

	return (t_min <= t_max);
}

/**
* Intersect test in BVH
*/
__device__ bool intersect(Sphere *start, TreeNode *cur,
	                      const Ray &r, double &t, int &id)
{
	// int n = 9;
	// double d, inf=t=1e20;
	// for(int i=n;i--;) if((d=start[i].intersect(r))&&d<t){t=d;id=i;}
	// return t<inf;

	// Use static allocation because malloc() can't be called in parallel
	// Use stack to traverse BVH to save space (cost is O(height))
	TreeNode *stack[INTERSECT_STACK_SIZE];
	int topIndex = INTERSECT_STACK_SIZE;
	stack[--topIndex] = cur;
	bool intersected = false;

	// Do while stack is not empty
	while (topIndex != INTERSECT_STACK_SIZE) {
		TreeNode *n = stack[topIndex++];
		if (intersection_bound_test(r, n->bound)) {
			if (n->leaf) {
				double d = n->sphere->intersect(r);
				if (d != 0.0) {
					if (d < t) {
						t = d;
						id = n->sphere->index;
					}
					intersected = true;
				}
			}
			else {
				stack[--topIndex] = n->right;
				stack[--topIndex] = n->left;

				if (topIndex < 0) {
//					printf("Intersect stack not big enough. Increase INTERSECT_STACK_SIZE!\n");
					return false;
				}
			}
		}
	}

	return intersected;
}

__device__ Vec radiance(Sphere *start, TreeNode *cur, const Ray &r_,
	                    int depth_, curandState *s)
{
	double t;                               // distance to intersection
	int id = 0;                               // id of intersected object
	Ray r = r_;
	int depth = depth_;
	Vec cl(0, 0, 0);   // accumulated color
	Vec cf(1, 1, 1);  // accumulated reflectance
	while (1) {
		t = 1e20;
		if (!intersect(start, cur, r, t, id)) return cl; // if miss, return black
		Sphere &obj = start[id];        // the hit object
		Vec x = r.o + r.d*t, n = (x - obj.p).norm(), nl = n.dot(r.d)<0 ? n : n*-1, f = obj.c;
		double p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl
		cl = cl + cf.mult(obj.e);
		if (++depth>5) if (drandom(s)<p) f = f*(1 / p); else return cl; //R.R.
		cf = cf.mult(f);
		if (obj.refl == DIFF) {                  // Ideal DIFFUSE reflection
			double r1 = 2 * M_PI*drandom(s), r2 = drandom(s), r2s = sqrt(r2);
			Vec w = nl, u = ((fabs(w.x)>.1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w%u;
			Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1 - r2)).norm();
			r = Ray(x, d);
			continue;
		}
		else if (obj.refl == SPEC) {           // Ideal SPECULAR reflection
			r = Ray(x, r.d - n * 2 * n.dot(r.d));
			continue;
		}
		Ray reflRay(x, r.d - n * 2 * n.dot(r.d));     // Ideal dielectric REFRACTION
		bool into = n.dot(nl)>0;                // Ray from outside going in?
		double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
		if ((cos2t = 1 - nnt*nnt*(1 - ddn*ddn))<0) {    // Total internal reflection
			r = reflRay;
			continue;
		}
		Vec tdir = (r.d*nnt - n*((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).norm();
		double a = nt - nc, b = nt + nc, R0 = a*a / (b*b), c = 1 - (into ? -ddn : tdir.dot(n));
		double Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
		if (drandom(s)<P) {
			cf = cf*RP;
			r = reflRay;
		}
		else {
			cf = cf*TP;
			r = Ray(x, tdir);
		}
		continue;
	}
}

/**
* Ray trace kernel
* Use BVH for better performance
*/
__global__ void kernelRayTrace(curandState* states, Vec *deviceSubpixelBuffer,
	                           int width, int height, int samps, Sphere *start,
	TreeNode *cudaDeviceTreeNodes, Ray cam, Vec cx, Vec cy)
{

	int subpixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
	if (subpixelIndex >= width * height * 4 || subpixelIndex < 0) return;
	int pixelIndex = subpixelIndex / 4;

	int y = pixelIndex / width;
	int x = pixelIndex % width;
	int sy = (subpixelIndex % 4) / 2;
	int sx = (subpixelIndex % 4) % 2;

	if (x < 0 || y < 0 || x >= width || y >= height) {
		return;
	}

	curand_init(y*y*y, subpixelIndex, 0, &states[subpixelIndex]);
	curandState state = states[subpixelIndex];

	Vec r = Vec();
	for (int s = 0; s<samps; s++) {
		double r1 = 2 * drandom(&state), dx = r1<1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
		double r2 = 2 * drandom(&state), dy = r2<1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
		Vec d = cx*(((sx + .5 + dx) / 2 + x) / width - .5) +
			cy*(((sy + .5 + dy) / 2 + y) / height - .5) + cam.d;
		r = r + radiance(start, cudaDeviceTreeNodes, Ray(cam.o + d * 140, d.norm()), 0, &state) * (1. / samps);
	}

	deviceSubpixelBuffer[subpixelIndex] = r;
}

/**
* Get result kernel
* Combine subpixel colors into one
*/
__global__ void kernelGetResult(Vec *deviceSubpixelBuffer,
	                            Vec *devicePixelBuffer, int width, int height)
{

	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index >= width * height) {
		return;
	}

	Vec res = Vec();
	for (int i = 0; i < 4; i++) {
		Vec subpixelVec = deviceSubpixelBuffer[index * 4 + i];
		res = res + Vec(clamp(subpixelVec.x),
			            clamp(subpixelVec.y),
			            clamp(subpixelVec.z)) * .25;
	}

	devicePixelBuffer[index] = res;
}

/**
* Radix tree construction kernel
* Algorithm described in karras2012 paper.
* Node-wise parallel
*/
__global__ void kernelConstructRadixTree(int len, TreeNode *radixTreeNodes,
	                                              TreeNode *radixTreeLeaves)
{

	int i = blockIdx.x * blockDim.x + threadIdx.x;

	if (i >= len) return;

	// Run radix tree construction algorithm
	// Determine direction of the range (+1 or -1)
	int d = longestCommonPrefix(i, i + 1, len + 1) -
		longestCommonPrefix(i, i - 1, len + 1) > 0 ? 1 : -1;

	// Compute upper bound for the length of the range
	int sigMin = longestCommonPrefix(i, i - d, len + 1);
	int lmax = 2;

	while (longestCommonPrefix(i, i + lmax * d, len + 1) > sigMin) {
		lmax *= 2;
	}

	// Find the other end using binary search
	int l = 0;
	int divider = 2;
	for (int t = lmax / divider; t >= 1; divider *= 2) {
		if (longestCommonPrefix(i, i + (l + t) * d, len + 1) > sigMin) {
			l += t;
		}
		t = lmax / divider;
	}

	int j = i + l * d;


	//printf("i:%d d:%d lmax:%d l:%d j:%d \n",i , d, lmax, l, j);
	// Find the split position using binary search
	int sigNode = longestCommonPrefix(i, j, len + 1);
	int s = 0;
	divider = 2;
	for (int t = (l + (divider - 1)) / divider; t >= 1; divider *= 2) {
		if (longestCommonPrefix(i, i + (s + t) * d, len + 1) > sigNode) {
			s = s + t;
		}
		t = (l + (divider - 1)) / divider;
	}

	int gamma = i + s * d + intMin(d, 0);

	// Output child pointers
	TreeNode *current = radixTreeNodes + i;


	if (intMin(i, j) == gamma) {
		current->left = radixTreeLeaves + gamma;
		(radixTreeLeaves + gamma)->parent = current;
	}
	else {
		current->left = radixTreeNodes + gamma;
		(radixTreeNodes + gamma)->parent = current;
	}

	if (intMax(i, j) == gamma + 1) {
		current->right = radixTreeLeaves + gamma + 1;
		(radixTreeLeaves + gamma + 1)->parent = current;
	}
	else {
		current->right = radixTreeNodes + gamma + 1;
		(radixTreeNodes + gamma + 1)->parent = current;
	}

	current->min = intMin(i, j);
	current->max = intMax(i, j);
}

__device__ bool check_bound(TreeNode *p, TreeNode *l, TreeNode *r) 
{
	return (
		p->bound.min_x == min2(l->bound.min_x, r->bound.min_x) &&
		p->bound.max_x == max2(l->bound.max_x, r->bound.max_x) &&
		p->bound.min_y == min2(l->bound.min_y, r->bound.min_y) &&
		p->bound.max_y == max2(l->bound.max_y, r->bound.max_y) &&
		p->bound.min_z == min2(l->bound.min_z, r->bound.min_z) &&
		p->bound.max_z == max2(l->bound.max_z, r->bound.max_z)
		);
}

__device__ bool check_sanity(TreeNode *n)
{
	if (n->leaf) {
		return true;
	}
	else {
		return (
			n->left->parent == n &&
			n->right->parent == n
			);
	}
}

/**
* BVH Construction kernel
* Algorithm described in karras2012 paper (bottom-up approach).
*/
__global__ void kernelConstructBVHTree(int len, TreeNode *treeNodes, TreeNode *treeLeaves,
	                                   int *nodeCounter, int *sorted_geometry_indices, Sphere *spheres)
{

	int index = blockIdx.x * blockDim.x + threadIdx.x;

	if (index >= len) return;

	TreeNode *leaf = treeLeaves + index;

	// Handle leaf first
	int geometry_index = sorted_geometry_indices[index];
	leaf->bound = spheres[geometry_index].bound;
	leaf->sphere = &(spheres[geometry_index]);

	TreeNode *current = leaf->parent;
	int currentIndex = current - treeNodes;
	int res = atomicAdd(nodeCounter + currentIndex, 1);

	// Go up and handle internal nodes
	while (1) {
		if (res == 0) {
			return;
		}

		merge_bounds(current->left->bound, current->right->bound,
			&(current->bound));

		// If current is root, return
		if (current == treeNodes) {
			return;
		}
		current = current->parent;
		currentIndex = current - treeNodes;
		res = atomicAdd(nodeCounter + currentIndex, 1);
	}
}

__inline__ __host__ __device__ float getArea(float min_x, float max_x,
	                                         float min_y, float max_y,
	                                         float min_z, float max_z)
{
	float dx = max_x - min_x;
	float dy = max_y - min_y;
	float dz = max_z - min_z;
	return 2 * (dx * dy + dx * dz + dy * dz);
}

__host__ __device__ int pwr(int base, unsigned exp)
{
	int acc = 1;
	for (unsigned c = 0; c < exp; c++) {
		acc *= base;
	}
	return acc;
}

__host__ __device__ float get_total_area(int n, TreeNode *leaves[], unsigned s)
{
	float lmin_x, lmin_y, lmin_z, lmax_x, lmax_y, lmax_z;
	float min_x = pos_infinity;
	float max_x = neg_infinity;
	float min_y = pos_infinity;
	float max_y = neg_infinity;
	float min_z = pos_infinity;
	float max_z = neg_infinity;
	for (int i = 0; i < n; i++) {
		if ((s >> i) & 1 == 1) {
			lmin_x = leaves[i]->bound.min_x;
			lmin_y = leaves[i]->bound.min_y;
			lmin_z = leaves[i]->bound.min_z;
			lmax_x = leaves[i]->bound.max_x;
			lmax_y = leaves[i]->bound.max_y;
			lmax_z = leaves[i]->bound.max_z;
			if (lmin_x < min_x) min_x = lmin_x;
			if (lmin_y < min_y) min_y = lmin_y;
			if (lmin_z < min_z) min_z = lmin_z;
			if (lmax_x > max_x) max_x = lmax_x;
			if (lmax_y > max_y) max_y = lmax_y;
			if (lmax_z > max_z) max_z = lmax_z;
		}
	}
	return getArea(min_x, max_x, min_y, max_y, min_z, max_z);
}

__host__ __device__ void calculateOptimalTreelet(int n, TreeNode **leaves,
	                                             unsigned char *p_opt)
{
	int num_subsets = pwr(2, n) - 1;
	// 0th element in array should not be used
	float a[128];
	float c_opt[128];
	// Calculate surface area for each subset
	for (unsigned char s = 1; s <= num_subsets; s++) {
		a[s] = get_total_area(n, leaves, s);
	}
	// Initialize costs of individual leaves
	for (unsigned i = 0; i <= (n - 1); i++) {
		c_opt[pwr(2, i)] = leaves[i]->cost;
	}
	// Optimize every subset of leaves
	for (unsigned k = 2; k <= n; k++) {
		for (unsigned char s = 1; s <= num_subsets; s++) {
			if (__popc(s) == k) {
				// Try each way of partitioning the leaves
				float c_s = pos_infinity;
				unsigned char p_s = 0;
				unsigned char d = (s - 1) & s;
				unsigned char p = (-d) & s;
				while (p != 0) {
					float c = c_opt[p] + c_opt[s ^ p];
					if (c < c_s) {
						c_s = c;
						p_s = p;
					}
					//printf("p=%x, c=%.0lf, c_s=%.0lf, p_s=%x\n", p & 0xff, c, c_s, p_s & 0xff);
					p = (p - d) & s;
				}
				// Calculate final SAH cost
				c_opt[s] = Ci * a[s] + c_s;
				p_opt[s] = p_s;
			}
		}
	}
}

__device__ void propagateAreaCost(TreeNode *root, TreeNode **leaves, int num_leaves)
{

	for (int i = 0; i < num_leaves; i++) {
		TreeNode *cur = leaves[i];
		cur = cur->parent;
		while (cur != root) {
			if (cur->cost == 0.0) {
				if (cur->left->cost != 0.0 && cur->right->cost != 0.0) {
					// Both left & right propagated
					Bound *bound = &cur->bound;
					merge_bounds(cur->left->bound, cur->right->bound, bound);
					cur->area = getArea(bound->min_x, bound->max_x, bound->min_y,
						bound->max_y, bound->min_z, bound->max_z);
					cur->cost = Ci * cur->area + cur->left->cost + cur->right->cost;
				}
				else {
					// Only one side propagated
					break;
				}
			}
			cur = cur->parent;
		}
	}

	// Propagate root
	Bound *bound = &root->bound;
	merge_bounds(root->left->bound, root->right->bound, bound);
	root->area = getArea(bound->min_x, bound->max_x, bound->min_y,
		bound->max_y, bound->min_z, bound->max_z);
	root->cost = Ci * root->area + root->left->cost + root->right->cost;
}

struct PartitionEntry {
	unsigned char partition;
	bool left;
	TreeNode *parent;
};

__device__ void restructTree(TreeNode *parent, TreeNode **leaves,
	                         TreeNode **nodes, unsigned char partition, unsigned char *optimal,
	                         int &index, bool left, int num_leaves)
{
	PartitionEntry stack[RESTRUCT_STACK_SIZE];
	int topIndex = RESTRUCT_STACK_SIZE;
	PartitionEntry tmp = { partition, left, parent };
	stack[--topIndex] = tmp;

	// Do while stack is not empty
	while (topIndex != RESTRUCT_STACK_SIZE) {
		PartitionEntry *pe = &stack[topIndex++];
		partition = pe->partition;
		left = pe->left;
		parent = pe->parent;

		if (partition == 0) return;

		if (__popc(partition) == 1) {    // Leaf
			
			int leaf_index = __ffs(partition) - 1;

			TreeNode *leaf = leaves[leaf_index];
			if (left) {
				parent->left = leaf;
			} else {
				parent->right = leaf;
			}
			leaf->parent = parent;
		} else {                        // Internal node
			
			if (index >= 7) return;

			TreeNode *node = nodes[index++];

			// Set cost to 0 as a mark
			node->cost = 0.0;

			if (left) {
				parent->left = node;
			} else {
				parent->right = node;
			}
			node->parent = parent;

			if (partition >= 128) return;

			unsigned char left_partition = optimal[partition];
			unsigned char right_partition = (~left_partition) & partition;

			if ((left_partition | partition) != partition) {
//				printf("left error: %x vs %x\n", left_partition & 0xff, partition & 0xff);
				return;
			}
			if ((right_partition | partition) != partition) {
//				printf("right error: %x vs %x\n", right_partition & 0xff, partition & 0xff);
				return;
			}

			if (topIndex < 2) {
				printf("restructTree stack not big enough. Increase RESTRUCT_STACK_SIZE!\n");
			}
			PartitionEntry tmp1 = { left_partition, true, node };
			stack[--topIndex] = tmp1;
			PartitionEntry tmp2 = { right_partition, false, node };
			stack[--topIndex] = tmp2;
		}
	}

	propagateAreaCost(parent, leaves, num_leaves);
}

__device__ void printPartition(TreeNode *root, unsigned char *optimal,
	                           unsigned char start, unsigned char mask)
{
	int level = 1;
	Queue *q = new Queue();
	q->push((void *)start);
	q->push((void *)((~start) & mask));

	Queue *qt = new Queue();

	while (!q->empty()) {

		while (!q->empty()) {
			unsigned char n = (unsigned char)(unsigned long)(q->last());
			q->pop();

			if (__popc(n) != 1) {
//				printf("[%d %p] %x\n", level, root, n & 0xff);
				qt->push((void *)optimal[n]);
				qt->push((void *)((~optimal[n]) & n));
			}
			else {
//				printf("[%d %p] (%d)\n", level, root, __ffs(n));
			}
		}
		level++;

		Queue *t = q;
		q = qt;
		qt = t;
	}

	delete q;
	delete qt;
}

/**
* treeletOptimize
* Find the treelet and optimize
*/
__device__ void treeletOptimize(TreeNode *root) 
{
	// Don't need to optimize if root is a leaf
	if (root->leaf) return;

	// Find a treelet with max number of leaves being 7
	TreeNode *leaves[7];
	int counter = 0;
	leaves[counter++] = root->left;
	leaves[counter++] = root->right;

	// Also remember the internal nodes
	// Max 7 (leaves) - 1 (root doesn't count) - 1
	TreeNode *nodes[5];
	int nodes_counter = 0;

	float max_area;
	int max_index = 0;

	while (counter < 7 && max_index != -1) {
		max_index = -1;
		max_area = -1.0;

		for (int i = 0; i < counter; i++) {
			if (!(leaves[i]->leaf)) {
				float area = leaves[i]->area;
				if (area > max_area) {
					max_area = area;
					max_index = i;
				}
			}
		}

		if (max_index != -1) {

			TreeNode *tmp = leaves[max_index];

			// Put this node in nodes array
			nodes[nodes_counter++] = tmp;

			// Replace the max node with its children
			leaves[max_index] = leaves[counter - 1];
			leaves[counter - 1] = tmp->left;
			leaves[counter++] = tmp->right;
		}
	}
	/*
#ifdef DEBUG_PRINT
	printf("%p counter=%d nodes_counter=%d\n", root, counter, nodes_counter);
	for (int i = 0; i < counter; i++) {
		printf("%p leaf %p\n", root, leaves[i]);
	}
	for (int i = 0; i < nodes_counter; i++) {
		printf("%p node %p\n", root, nodes[i]);
	}
#endif
	*/

	unsigned char optimal[128];

	// Call calculateOptimalCost here
	calculateOptimalTreelet(counter, leaves, optimal);
/*
#ifdef DEBUG_PRINT
	printPartition(root, optimal, optimal[(1 << counter) - 1], (1 << counter) - 1);
#endif
*/
	// Use complement on right tree, and use original on left tree
	unsigned char mask = (1 << counter) - 1;    // mask = max index
	int index = 0;                              // index for free nodes
	unsigned char leftIndex = mask;
	unsigned char left = optimal[leftIndex];
	restructTree(root, leaves, nodes, left, optimal, index, true, counter);

	unsigned char right = (~left) & mask;
	restructTree(root, leaves, nodes, right, optimal, index, false, counter);

	// Calculate current node's area & cost
	Bound *bound = &root->bound;
	merge_bounds(root->left->bound, root->right->bound, bound);
	root->area = getArea(bound->min_x, bound->max_x, bound->min_y,
		bound->max_y, bound->min_z, bound->max_z);
	root->cost = Ci * root->area + root->left->cost + root->right->cost;
}

/**
* BVH Optimization kernel
*/
__global__ void kernelOptimize(int num_leaves, int *nodeCounter,
	                           TreeNode *treeNodes, TreeNode *treeLeaves)
{

	int index = blockIdx.x * blockDim.x + threadIdx.x;

	if (index >= num_leaves) return;

	TreeNode *leaf = treeLeaves + index;

	// Handle leaf first
	// Leaf's cost is just its bounding volumn's cost
	Bound *bound = &leaf->bound;
	leaf->area = getArea(bound->min_x, bound->max_x, bound->min_y,
		bound->max_y, bound->min_z, bound->max_z);
	leaf->cost = Ct * leaf->area;

	__syncthreads();
/*
#ifdef DEBUG_PRINT
	__syncthreads();
	if (index == 0) {
		printf("Launching Print BVH GPU... (before Optimization)\n");
		printBVH(treeNodes);
		printf("Launched Print BVH GPU... (before Optimization)\n");
	}
	__syncthreads();
#endif
*/
	TreeNode *current = leaf->parent;
	int currentIndex = current - treeNodes;
	int res = atomicAdd(nodeCounter + currentIndex, 1);

	// Go up and handle internal nodes
	while (1) {
		if (res == 0) {
			return;
		}

//		printf("%d Going to optimize %p\n", index, current);

		treeletOptimize(current);

//		printf("%d Optimized %p\n", index, current);

		// If current is root, return
		if (current == treeNodes) {
			return;
		}
		current = current->parent;
		currentIndex = current - treeNodes;
		res = atomicAdd(nodeCounter + currentIndex, 1);
	}

}



extern "C"  int smallptCudaRayTrace(int width, int height, unsigned int *bmpImage, int samples)
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
//	smallptCudakernel <<< grid, block >>>(width, height, output_d, samples);
//	smallptCudakernel <<< height, width >>>(width, height, output_d, samples);

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

		  bmpImage[i] = (0xFF << 24) | (red << 16) | (green << 8) | blue;
	}

	delete[] output_h;
	cudaFree(output_d);

	return 1;
}
