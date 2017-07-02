#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define DXF_IMPLEMENTATION
#include "dxf.h"

#define PI 3.14159265359f

static
void generate_polygon(dxf_real radius, int num_sides, dxf_real **vertices)
{
	const dxf_real delta_rad = 2 * PI / num_sides;
	*vertices = malloc(num_sides * 2 * sizeof(dxf_real));
	for (int i = 0; i < num_sides; ++i) {
		(*vertices)[2*i] = radius * cosf(delta_rad * i);
		(*vertices)[2*i+1] = radius * sinf(delta_rad * i);
	}
}

#define REQUEST_INPUT 1
#define CHECK(x) if ((x) == EOF) goto out

int main(int argc, const char *argv[])
{
	int num_sides, ret = 1;
	dxf_real *vertices;
	dxf_ctx_t dxf;
#if REQUEST_INPUT
	char fname[64];
#endif

#if REQUEST_INPUT
	fputs("Number of sides: ", stdout);
	if (scanf("%d", &num_sides) != 1)
		return ret;

	fputs("Filename: ", stdout);
	if (scanf("%63s", fname) != 1)
		return ret;
	dxf.fp = fopen(fname, "w");
#else
	if (argc != 3)
		return ret;

	num_sides = atoi(argv[1]);
	dxf.fp = fopen(argv[2], "w");
#endif

	if (num_sides < 3)
		return ret;

	if (!dxf.fp)
		return ret;

	generate_polygon(10, num_sides, &vertices);

	dxf_init(&dxf);
	CHECK(dxf_entities_begin(&dxf));
	CHECK(dxf_line(&dxf, 0, 0, 5, 0));
	CHECK(dxf_polygon(&dxf, vertices, num_sides));
	CHECK(dxf_text(&dxf, 1, 1, 4, "Test string"));
	CHECK(dxf_entities_end(&dxf));
	CHECK(dxf_end(&dxf));
	ret = 0;

out:
	free(vertices);
	fclose(dxf.fp);
	return ret;
}
