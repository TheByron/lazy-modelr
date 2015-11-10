#include <vector>

typedef struct vec3{	
	double x;
	double y;
	double z;
	float c[4];	// rgbA color info
} vec3;

typedef struct Poly{
	vec3 v[4];		// vertices info
} Poly;
