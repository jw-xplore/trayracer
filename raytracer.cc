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
Raytracer::Raytrace()
{
    static int leet = 1337;
    std::mt19937 generator (leet++);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // RE: Looks like caching doesn't really save performance?
    float u = 0;
    float v = 0;
    vec3 direction;
    Ray* ray = new Ray(get_position(this->view), direction);

    float wFactor = (1.0f / this->width);
    float hFactor = (1.0f / this->height);

    for (int x = 0; x < this->width; ++x)
    {
        for (int y = 0; y < this->height; ++y)
        {
            Color color;
            for (int i = 0; i < this->rpp; ++i)
            {
                // RE: u and v setting takes around 18 instruction each
                float disres = dis(generator);
                u = ((float(x + disres) * wFactor) * 2.0f) - 1.0f;
                v = ((float(y + disres) * hFactor) * 2.0f) - 1.0f;

                // RE: direction and ray also takes a many instruction to setup, consider caching?
                direction = vec3(u, v, -1.0f);
                direction = transform(direction, this->frustum);
                
                // RENOTE: Don't recreate the ray, only update magnitude
                //ray = new Ray(get_position(this->view), direction);
                ray->m = direction;
                color += this->TracePath(*ray, 0);
            }

            // divide by number of samples per pixel, to get the average of the distribution
            color.r /= this->rpp;
            color.g /= this->rpp;
            color.b /= this->rpp;

            this->frameBuffer[y * this->width + x] += color;
        }
    }

    delete ray;
}

//------------------------------------------------------------------------------
/**
 * @parameter n - the current bounce level
*/
Color
Raytracer::TracePath(Ray ray, unsigned n)
{
    vec3 hitPoint;
    vec3 hitNormal;
    Object* hitObject = nullptr;
    float distance = FLT_MAX;

    // RE: This compare is also quite heavy, to be checked
    if (Raycast(ray, hitPoint, hitNormal, hitObject, distance))
    {
        // RE: ray creation take a lot of instruction + deleting it just add lot of load for each iteration
        Ray* scatteredRay = new Ray(hitObject->ScatterRay(ray, hitPoint, hitNormal));
        if (n < this->bounces)
        {
            return hitObject->GetColor() * this->TracePath(*scatteredRay, n + 1);
        }
        delete scatteredRay;

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
Raytracer::Raycast(Ray ray, vec3& hitPoint, vec3& hitNormal, Object*& hitObject, float& distance)
{
    bool isHit = false;
    HitResult closestHit;
    int numHits = 0;
    HitResult hit;

    // RENOTE: Seems like there is no real deference between world and unique objects

    // First, sort the world objects
    //std::sort(world.begin(), world.end()); // RE: Maybe have sorting at start once? It seems pointless maybe?

    // then add all objects into a remaining objects set of unique objects, so that we don't trace against the same object twice
    // RE: Objects should be unique from start
    /*
    std::vector<Object*> uniqueObjects;
    for (size_t i = 0; i < world.size(); ++i)
    {
        Object* obj = world[i];

        // RE: This simple looping is much faster than iterator with lambda 
        int objSize = uniqueObjects.size();
        bool found = false;
        for (int oi = 0; i < objSize; i++)
        {
            if (obj->GetName() == uniqueObjects[i]->GetName() && obj->GetId() == uniqueObjects[i]->GetId())
            {
                bool found = true;
                break;
            }
        }

        if (!found)
        {
            // RE: Pushing gives loot of load, only option is to have unique items from the start
            uniqueObjects.push_back(obj);
        }
    }
    */

    int objSize = this->objects.size();

    for (int i = 0; i < objSize; i++)
    {
        // RE: Is auto taking more than strong typing?
        Object* object = this->objects[i];

        // RE: Type casting could be an issue?
        auto opt = object->Intersect(ray, closestHit.t);
        if (opt.HasValue())
        {
            hit = opt.Get();
            assert(hit.t < closestHit.t);
            closestHit = hit;
            closestHit.object = object;
            isHit = true;
            numHits++;
        }
    }

    // RENOTE: Clear improves speed as there is no erase and resize for each iteration
    // Clear once can't be used for proper rendering
    //world.clear();

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
