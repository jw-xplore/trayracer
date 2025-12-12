#pragma once
#include <vector>
#include "vec3.h"
#include "mat4.h"
#include "color.h"
#include "ray.h"
#include "object.h"
#include <float.h>
#include <chrono>
#include <thread>
#include <future>

//------------------------------------------------------------------------------
/**
*/
class Raytracer
{
public:
    Raytracer(unsigned w, unsigned h, Color*& frameBuffer, unsigned rpp, unsigned bounces);
    ~Raytracer() { }

    // start raytracing!
    void Raytrace(int start, int w);

    void RaytraceThreaded();

    // add object to scene
    void AddObject(Object* obj);

    // single raycast, find object
    bool Raycast(Ray& ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance);

    // set camera matrix
    void SetViewMatrix(mat4 val);

    // clear screen
    void Clear();

    // update matrices. Called automatically after setting view matrix
    void UpdateMatrices();

    // trace a path and return intersection color
    // n is bounce depth
    Optional<HitResult> intersectTask(Object* obj, Ray& r, float& tOut);
    Color TracePath(Ray& ray, unsigned n);

    // get the color of the skybox in a direction
    Color Skybox(vec3 direction);

    //std::vector<Color>& frameBuffer;
    Color*& frameBuffer;
    int frameBufferSize;
    
    // rays per pixel
    unsigned rpp;
    // max number of bounces before termination
    unsigned bounces = 5;

    // width of framebuffer
    const unsigned width;
    // height of framebuffer
    const unsigned height;
    
    const vec3 lowerLeftCorner = { -2.0, -1.0, -1.0 };
    const vec3 horizontal = { 4.0, 0.0, 0.0 };
    const vec3 vertical = { 0.0, 2.0, 0.0 };
    const vec3 origin = { 0.0, 2.0, 10.0f };

    // view matrix
    mat4 view;
    // Go from canonical to view frustum
    mat4 frustum;

    // Static allocation
    int objectsSize;
    Object** objects;

private:
    //std::vector<Object*> objects;
    Ray* scatteredRay;
};

/*
inline void Raytracer::AddObject(Object* o)
{
    this->objects.push_back(o);
}
*/

inline void Raytracer::SetViewMatrix(mat4 val)
{
    this->view = val;
    this->UpdateMatrices();
}
