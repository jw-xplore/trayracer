#include <stdio.h>
#include "window.h"
#include "vec3.h"
#include "raytracer.h"
#include "sphere.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <fstream>

#define degtorad(angle) angle * MPI / 180

/*
Test program part
*/
struct TestData
{
    bool ignore = false;

    int width, height, rayCount, spheresCount = 0;
    bool generateImg = false;
};

TestData testInput()
{
    TestData data;

    std::string input;
    std::cout << "Run test (n - stardant run/ y + w h raysPerPixels sphereCount createImg): ";
    std::getline(std::cin, input);
    input += " ";

    // Ignore
    if (input[0] == 'n')
    {
        data.ignore = true;
        return data;
    }

    // Parse
    int paramNumber = -1;
    std::string param = "";

    for (int i = 1; i < input.size(); i++)
    {
        char c = input[i];
        if (c == ' ' || i + 1 == input.size())
        {
            // Add param
            if (param != "")
            {
                switch (paramNumber)
                {
                case 0: data.width = std::stoi(param); break;
                case 1: data.height = std::stoi(param); break;
                case 2: data.rayCount = std::stoi(param); break;
                case 3: data.spheresCount = std::stoi(param); break;
                case 4:
                    if (param[0] == 'y')
                        data.generateImg = true; 
                    break;
                }

                param = "";
            }
            // Next 
            paramNumber++;
            continue;
        }

        param += input[i];
    }

    return data;
}

void createImage(int w, int h, std::vector<Color>& renderResult)
{
    std::ofstream img("testImg.pgm", std::ios_base::out
        | std::ios_base::binary
        | std::ios_base::trunc
    );

    img << "P5\n" << w << " " << h << "\n" << 255 << "\n";

    char* line = new char[w];
    int pos = 0;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            pos = w * y + x;
            line[x] = ((renderResult[pos].r + renderResult[pos].g + renderResult[pos].b) * 255) / 3;
        }
        
        //std::cout << line << std::endl;

        img.write(reinterpret_cast<const char*>(line), w);
    }

    delete line;
    img.close();
}

/*
Main run
*/
int main()
{ 
    Display::Window wnd;
    
    wnd.SetTitle("TrayRacer");
    
    if (!wnd.Open())
        return 1;

    unsigned w = 200;
    unsigned h = 100;
    
    int raysPerPixel = 1;
    int maxBounces = 5;

    int sphereCount = 12;

    // Setup test
    TestData test = testInput();
    if (!test.ignore)
    {
        w = test.width;
        h = test.height;
        raysPerPixel = test.rayCount;
        sphereCount = test.spheresCount / 3;
    }

    //framebuffer.resize(w * h);
    int frameBufferSize = w * h;
    Color* framebuffer = new Color[frameBufferSize];
    Color* framebufferCopy = new Color[frameBufferSize];

    Raytracer rt = Raytracer(w, h, framebuffer, raysPerPixel, maxBounces);

    // Create some objects
    Material* mat = new Material();
    mat->type = MaterialType::Lambertian;
    mat->color = { 0.5,0.5,0.5 };
    mat->roughness = 0.3;
    Sphere* ground = new Sphere(1000, { 0,-1000, -1 }, mat);

    rt.objectsSize = sphereCount;
    rt.objects = new Object * [sphereCount];
    rt.objects[0] = ground;
    //rt.AddObject(ground);

    for (int it = 1; it < sphereCount; it++)
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
            //rt.AddObject(ground);
            rt.objects[it] = ground;
        }
        
        {
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
            //rt.AddObject(ground);
            rt.objects[it] = ground;
        }{
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
            //rt.AddObject(ground);
            rt.objects[it] = ground;
        }
    }
    
    bool exit = false;

    // camera
    bool resetFramebuffer = false;
    vec3 camPos = { 0,1.0f,10.0f };
    vec3 moveDir = { 0,0,0 };

    wnd.SetKeyPressFunction([&exit, &moveDir, &resetFramebuffer](int key, int scancode, int action, int mods)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
            exit = true;
            break;
        case GLFW_KEY_W:
            moveDir.z -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_S:
            moveDir.z += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_A:
            moveDir.x -= 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_D:
            moveDir.x += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_SPACE:
            moveDir.y += 1.0f;
            resetFramebuffer |= true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            moveDir.y -= 1.0f;
            resetFramebuffer |= true;
            break;
        default:
            break;
        }
    });

    float pitch = 0;
    float yaw = 0;
    float oldx = 0;
    float oldy = 0;

    wnd.SetMouseMoveFunction([&pitch, &yaw, &oldx, &oldy, &resetFramebuffer](double x, double y)
    {
        x *= -0.1;
        y *= -0.1;
        yaw = x - oldx;
        pitch = y - oldy;
        resetFramebuffer |= true;
        oldx = x;
        oldy = y;
    });

    float rotx = 0;
    float roty = 0;

    // number of accumulated frames
    int frameIndex = 0;

    //std::vector<Color> framebufferCopy;
    //framebufferCopy.resize(w * h);

    // rendering loop
    while (wnd.IsOpen() && !exit)
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        resetFramebuffer = false;
        moveDir = {0,0,0};
        pitch = 0;
        yaw = 0;

        // poll input
        wnd.Update();

        rotx -= pitch;
        roty -= yaw;

        // RE: not needed since it is always 0
        //moveDir = normalize(moveDir);

        mat4 xMat = (rotationx(rotx));
        mat4 yMat = (rotationy(roty));
        mat4 cameraTransform = multiply(yMat, xMat);

        camPos = camPos + transform(moveDir * 0.2f, cameraTransform);
        
        cameraTransform.m30 = camPos.x;
        cameraTransform.m31 = camPos.y;
        cameraTransform.m32 = camPos.z;

        rt.SetViewMatrix(cameraTransform);
        
        if (resetFramebuffer)
        {
            rt.Clear();
            frameIndex = 0;
        }

        // RE: Make raytrace run on other thread
        //std::thread threadRaytrace(RaytraceTask, &rt);
        //threadRaytrace.join();
        //rt.Raytrace(0, w);
        rt.RaytraceThreaded();
        frameIndex++;

        // Get the average distribution of all samples
        for (size_t p = 0; p < frameBufferSize; p++)
        {
            framebufferCopy[p] = framebuffer[p];
            framebufferCopy[p].r /= frameIndex;
            framebufferCopy[p].g /= frameIndex;
            framebufferCopy[p].b /= frameIndex;
        }

        glClearColor(0, 0, 0, 1.0);
        glClear( GL_COLOR_BUFFER_BIT );

        wnd.Blit((float*)&framebufferCopy[0], w, h);
        wnd.SwapBuffers();

        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> ms_double = frameEnd - frameStart;
        double frameTime = ms_double.count();
        std::cout << "FPS: " << 1/frameTime << ", s: " << frameTime << std::endl;

        /*
        if (!test.ignore)
        {
            std::cout << "Time: " << frameTime << "s" << std::endl;

            //if (test.generateImg)
                //createImage(w, h, framebuffer);
            
            break;
        }
        */
    }

    // Run raytrace
    auto start = std::chrono::high_resolution_clock::now();
    //rt.Raytrace(w);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> ms_double = end - start;

    double t = ms_double.count();

    std::cout << "rt.RayTrace time: " << t << "s" << std::endl;

    // RE: Note original measured time: 27.0967ms per rt.RayTrace call

    if (wnd.IsOpen())
        wnd.Close();

    return 0;
} 