#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

#include <climits> // for INT_MAX
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
#include "MengerSponge.h"



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

void render_pass(std::vector<vec3f> &image_HDR, const int frame, const int pass, const int image_width, const int image_height, const int frames, const Scene &world)
{
	#pragma omp parallel for schedule(dynamic, 1)
	for (int y = 0; y < image_height; y++)
	for (int x = 0; x < image_width;  x++)
		image_HDR[y * image_width + x] += generateColour(x, y, frame, pass, image_width, image_height, frames, world);
}

void tonemap(std::vector<sRGBPixel> &image_LDR, const std::vector<vec3f> &image_HDR, const int passes, const int image_width, const int image_height)
{
	const float scale = 1.0f / passes;
	#pragma omp parallel for
	for (int y = 0; y < image_height; y++)
	for (int x = 0; x < image_width;  x++)
	{
		const int pixel_idx = y * image_width + x;
		const vec3f pixel_colour = image_HDR[pixel_idx];
		image_LDR[pixel_idx] =
		{
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.x * scale) * 256))),
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.y * scale) * 256))),
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.z * scale) * 256)))
		};
	}
}

int main(int argc, char ** argv)
{
	// parse command line arguments
	enum { mode_progressive, mode_animation } mode = mode_animation;
	if (argc > 1)
	{
		if (std::string(argv[1]) == "--progressive")
		{
			mode = mode_progressive;
		}
	}

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
	//MengerSpongeAnalytic bulb;
	//MengerSpongeDual bulb;
	//bulb.radius = sqrt(3);
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

	std::vector<vec3f>     image_HDR(image_width * image_height);
	std::vector<sRGBPixel> image_LDR(image_width * image_height);

	switch (mode)
	{
		case mode_animation:
		{
			const int frames = 30 * 8;
			const int passes = 2 * 3 * 5;
			printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);
			for (int frame = 0; frame < frames; ++frame)
			{
				std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });
		
				// Render image passes
				for (int pass = 0; pass < passes; ++pass)
				{
					render_pass(image_HDR, frame, pass, image_width, image_height, frames, world);
				}
		
				// Tonemap and convert to LDR sRGB
				tonemap(image_LDR, image_HDR, passes, image_width, image_height);
		
				// Save frame
				char filename[64];
				snprintf(filename, 64, "frame_%08d.png", frame);
				stbi_write_png(filename, image_width, image_height, 3, &image_LDR[0], image_width * 3);
				printf("Saved %s\n", filename);
			}
			break;
		}

		case mode_progressive:
		{
			printf("Progressive rendering at resolution %d x %d with doubling passes\n", image_width, image_height);
			int pass = 0;
			std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });
			for (int passes = 1; true; passes <<= 1)
			{
				// Render image passes
				for (; pass < passes; ++pass)
				{
					// frame/frames 0/INT_MAX to reduce motion blur for still image
					render_pass(image_HDR, 0, pass, image_width, image_height, INT_MAX, world);
				}

				// Tonemap and convert to LDR sRGB
				tonemap(image_LDR, image_HDR, passes, image_width, image_height);

				// Save frame
				char filename[64];
				snprintf(filename, 64, "pass_%08d.png", pass);
				stbi_write_png(filename, image_width, image_height, 3, &image_LDR[0], image_width * 3);
				printf("Saved %s\n", filename);
			}
			break;
		}
	}

	return 0;
}
