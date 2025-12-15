#include "pch.h"
#include "CppUnitTest.h"
#include "../raytracer.h"
#include "../sphere.h"
#include "ImageGenerator.h"
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
            const unsigned w = 500;
            const unsigned h = 250;
            framebuffer.resize(w * h);

            int raysPerPixel = 1;
            int maxBounces = 5;

            Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);
            
            // Create some objects
            Material* mat = new Material();
            mat->type = MaterialType::Lambertian;
            mat->color = { 0.5,0.5,0.5 };
            mat->roughness = 0.3;
            Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);

            rt.SetupObjects(901);
            rt.AddObject(ground);

            for (int it = 0; it < 300; it++)
            {
                {
                    Material* mat = new Material();
                    mat->type = MaterialType::Lambertian;
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
                    mat->type = MaterialType::Conductor;
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
                    mat->type = MaterialType::Dielectric;
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

            // Camera setup
            vec3 moveDir = { 0,0,0 };
            float pitch = 0;
            float yaw = 0;
            vec3 camPos = { 0,1.0f,10.0f };

            float rotx = 0;
            float roty = 0;

            // poll input
           // wnd.Update();

            mat4 xMat = (rotationx(rotx));
            mat4 yMat = (rotationy(roty));
            mat4 cameraTransform = multiply(yMat, xMat);

            camPos = camPos + transform(moveDir * 0.2f, cameraTransform);

            cameraTransform.m30 = camPos.x;
            cameraTransform.m31 = camPos.y;
            cameraTransform.m32 = camPos.z;

            rt.SetViewMatrix(cameraTransform);

            // Measure raytrace
            auto start = std::chrono::high_resolution_clock::now();
            rt.RaytraceThreaded();
            auto end = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> time = end - start;
            std::string strTime = "RT time: " + std::to_string(time.count()) + "s";
            CreateImage(w, h, framebuffer);

            Logger::WriteMessage(strTime.data());
		}
	};
}
