#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

#include <cmath> // For std::sqrt and so on
#include <vector>
#include <string>
#include <algorithm> // For std::pair and std::min and max

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vec2.h"
#include "vec3.h"

#include "Ray.h"
#include "SceneObject.h"
#include "Renderer.h"

#include "SimpleObjects.h"
#include "AnalyticDEObject.h"
#include "DualDEObject.h"

#include "Mandelbulb.h"
#include "QuadraticJuliabulb.h"



inline float sRGB(const float u)
{
	return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f;
}


struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


int main(int argc, char ** argv)
{
	// Squash -Wunused-parameter
	(void) argc;
	(void) argv;
	std::vector<Sphere> spheres;
	{
		const real main_sphere_rad = 4;
		//Sphere s;
		//s.centre = { 0, 0, 0 };
		//s.radius = main_sphere_rad;
		//s.colour = { 0.1f, 0.3f, 0.7f };
		//spheres.push_back(s);

		Sphere s2;
		const real bigrad = 1024;
		s2.centre = { 0, -bigrad - main_sphere_rad, 0 };
		s2.radius = bigrad;
		s2.colour = { 0.6f, 0.3f, 0.2f };
		spheres.push_back(s2);
	}

	//MandelbulbAnalytic bulb;
	MandelbulbDual bulb;
	//QuadraticJuliabulbAnalytic bulb;
	//QuadraticJuliabulbDual bulb;
	bulb.radius = 4;
	bulb.colour = { 0.1f, 0.3f, 0.7f };

	// Set up the world
	Scene world;
	{
		for (const Sphere & s : spheres)
			world.objects.push_back((SceneObject *)&s);

		world.objects.push_back((SceneObject *)&bulb);
	}

	const int image_multi  = 40;
	const int image_width  = image_multi * 16;
	const int image_height = image_multi * 9;
	const int frames = 30 * 8;
	const int passes = 2 * 3 * 5;
	printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);


	std::vector<vec3f>     image_HDR(image_width * image_height);
	std::vector<sRGBPixel> image_LDR(image_width * image_height);

	for (int frame = 0; frame < frames; ++frame)
	{
		std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });

		// Render image passes
		for (int pass = 0; pass < passes; ++pass)
		{
			#pragma omp parallel for schedule(dynamic, 1)
			for (int y = 0; y < image_height; y++)
			for (int x = 0; x < image_width;  x++)
				image_HDR[y * image_width + x] += generateColour(x, y, frame, pass, image_width, image_height, frames, world);
		}

		// Tonemap and convert to LDR sRGB
		#pragma omp parallel for
		for (int y = 0; y < image_height; y++)
		for (int x = 0; x < image_width;  x++)
		{
			const int pixel_idx = y * image_width + x;
			const float scale = 1.0f / passes;

			const vec3f pixel_colour = image_HDR[pixel_idx];
			image_LDR[pixel_idx] =
			{
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.x * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.y * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.z * scale) * 256)))
			};
		}

		// Save frame
		char filename[64];
		snprintf(filename, 64, "frame_%08d.png", frame);
		stbi_write_png(filename, image_width, image_height, 3, &image_LDR[0], image_width * 3);
		printf("Saved %s\n", filename);
	}

	return 0;
}
