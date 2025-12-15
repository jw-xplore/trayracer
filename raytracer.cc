#include "raytracer.h"
#include <random>

//------------------------------------------------------------------------------
/**
*/
Raytracer::Raytracer(unsigned w, unsigned h, std::vector<Color>& frameBuffer, unsigned rpp, unsigned bounces) :
    frameBuffer(frameBuffer),
    rpp(rpp),
    bounces(bounces),
    width(w),
    height(h)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Raytrace(int start, int w)
{
    static int leet = 1337;
    std::mt19937 generator (leet++);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    float u;
    float v;
    vec3 direction;
    Ray* ray = new Ray();
    double colorDiv = 1.0f / this->rpp;

    w += start;

    for (size_t x = start; x < w; ++x)
    {
        for (size_t y = 0; y < this->height; ++y)
        {
            Color color;
            for (size_t i = 0; i < this->rpp; ++i)
            {
                u = ((float(x + dis(generator)) * (1.0f / this->width)) * 2.0f) - 1.0f;
                v = ((float(y + dis(generator)) * (1.0f / this->height)) * 2.0f) - 1.0f;

                direction = vec3(u, v, -1.0f);
                direction = transform(direction, this->frustum);
 
                ray->b = get_position(this->view);
                ray->m = direction;
                color += this->TracePath(*ray, 0);
            }

            // divide by number of samples per pixel, to get the average of the distribution
            color.r *= colorDiv;
            color.g *= colorDiv;
            color.b *= colorDiv;

            this->frameBuffer[y * this->width + x] += color;
        }
    }

    delete ray;
}

void Raytracer::RaytraceThreaded()
{
    int w = this->width * 0.0625f;
    std::thread t1(&Raytracer::Raytrace, this, 0, w);
    std::thread t2(&Raytracer::Raytrace, this, w, w);
    std::thread t3(&Raytracer::Raytrace, this, w + w, w);
    std::thread t4(&Raytracer::Raytrace, this, w + w * 2, w);
    std::thread t5(&Raytracer::Raytrace, this, w + w * 3, w);
    std::thread t6(&Raytracer::Raytrace, this, w + w * 4, w);
    std::thread t7(&Raytracer::Raytrace, this, w + w * 5, w);
    std::thread t8(&Raytracer::Raytrace, this, w + w * 6, w);
    std::thread t9(&Raytracer::Raytrace, this, w + w * 7, w);
    std::thread t10(&Raytracer::Raytrace, this, w + w * 8, w);
    std::thread t11(&Raytracer::Raytrace, this, w + w * 9, w);
    std::thread t12(&Raytracer::Raytrace, this, w + w * 10, w);
    std::thread t13(&Raytracer::Raytrace, this, w + w * 11, w);
    std::thread t14(&Raytracer::Raytrace, this, w + w * 12, w);
    std::thread t15(&Raytracer::Raytrace, this, w + w * 13, w);
    std::thread t16(&Raytracer::Raytrace, this, w + w * 14, w);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
    t11.join();
    t12.join();
    t13.join();
    t14.join();
    t15.join();
    t16.join();
}

//------------------------------------------------------------------------------
/**
 * @parameter n - the current bounce level
*/
Color
Raytracer::TracePath(Ray& ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;

    if (Raycast(ray, hitPoint, hitNormal, hitObject, distance))
    {
        //Ray* scatteredRay = new Ray(hitObject->ScatterRay(ray, hitPoint, hitNormal));
        Ray scatteredRay = hitObject->ScatterRay(ray, hitPoint, hitNormal);
        if (n < this->bounces)
        {
            return hitObject->GetColor() * this->TracePath(scatteredRay, n + 1);
        }
        //delete scatteredRay;

        if (n == this->bounces)
        {
            return {0,0,0};
        }
    }

    return this->Skybox(ray.m);
}

//------------------------------------------------------------------------------
/**
*/
bool
Raytracer::Raycast(Ray& ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance)
{
    bool isHit = false;
    HitResult closestHit;

    HitResult hit;

    for (size_t i = 0; i < objectsSize; i++)
    {
        Object* object = this->objects[i];

        if (object->Intersect(hit, ray, closestHit.t))
        {
            //hit = opt.Get();
            //assert(hit.t < closestHit.t);
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
        }
    }

    hitPoint = closestHit.p;
    hitNormal = closestHit.normal;
    hitObject = closestHit.object;
    distance = closestHit.t;
    
    return isHit;
}


//------------------------------------------------------------------------------
/**
*/
void
Raytracer::Clear()
{
    for (auto& color : this->frameBuffer)
    {
        color.r = 0.0f;
        color.g = 0.0f;
        color.b = 0.0f;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
Raytracer::UpdateMatrices()
{
    mat4 inverseView = inverse(this->view); 
    mat4 basis = transpose(inverseView);
    this->frustum = basis;
}

//------------------------------------------------------------------------------
/**
*/
Color
Raytracer::Skybox(vec3 direction)
{
    float t = 0.5*(direction.y + 1.0);
    vec3 vec = vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
    return {(float)vec.x, (float)vec.y, (float)vec.z};
}
