#include "pch.h"
#include "CppUnitTest.h"
#include "../raytracer.h"
#include "../sphere.h"
#include <chrono>
#include <iostream>
#include <string>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TracerTest
{
	TEST_CLASS(TracerTest)
	{
	public:
		TEST_METHOD(Raytrace)
		{
            std::vector<Color> framebuffer;
            const unsigned w = 200;
            const unsigned h = 100;
            framebuffer.resize(w * h);

            int raysPerPixel = 1;
            int maxBounces = 5;

            Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);
            
            // Create some objects
            Material* mat = new Material();
            mat->type = "Lambertian";
            mat->color = { 0.5,0.5,0.5 };
            mat->roughness = 0.3;
            Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);
            rt.AddObject(ground);

            for (int it = 0; it < 12; it++)
            {
                {
                    Material* mat = new Material();
                    mat->type = "Lambertian";
                    float r = RandomFloat();
                    float g = RandomFloat();
                    float b = RandomFloat();
                    mat->color = { r,g,b };
                    mat->roughness = RandomFloat();
                    const float span = 10.0f;
                    Sphere* ground = new Sphere(
                        RandomFloat() * 0.7f + 0.2f,
                        {
                            RandomFloatNTP() * span,
                            RandomFloat() * span + 0.2f,
                            RandomFloatNTP() * span
                        },
                        mat);
                    rt.AddObject(ground);
                } {
                    Material* mat = new Material();
                    mat->type = "Conductor";
                    float r = RandomFloat();
                    float g = RandomFloat();
                    float b = RandomFloat();
                    mat->color = { r,g,b };
                    mat->roughness = RandomFloat();
                    const float span = 30.0f;
                    Sphere* ground = new Sphere(
                        RandomFloat() * 0.7f + 0.2f,
                        {
                            RandomFloatNTP() * span,
                            RandomFloat() * span + 0.2f,
                            RandomFloatNTP() * span
                        },
                        mat);
                    rt.AddObject(ground);
                } {
                    Material* mat = new Material();
                    mat->type = "Dielectric";
                    float r = RandomFloat();
                    float g = RandomFloat();
                    float b = RandomFloat();
                    mat->color = { r,g,b };
                    mat->roughness = RandomFloat();
                    mat->refractionIndex = 1.65;
                    const float span = 25.0f;
                    Sphere* ground = new Sphere(
                        RandomFloat() * 0.7f + 0.2f,
                        {
                            RandomFloatNTP() * span,
                            RandomFloat() * span + 0.2f,
                            RandomFloatNTP() * span
                        },
                        mat);
                    rt.AddObject(ground);
                }
            }

            auto start = std::chrono::high_resolution_clock::now();
            rt.Raytrace();
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time = end - start;
            std::string strTime = "RT time: " + std::to_string(time.count()) + "s";
            Logger::WriteMessage(strTime.data());
		}
	};
}
