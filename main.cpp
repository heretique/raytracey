#define SDL_MAIN_HANDLED

#include "BvhNode.h"
#include "HitableList.h"
#include "camera.h"
#include "material.h"
#include "sphere.h"
#include <Hq/JobManager.h>
#include <Hq/Math/Ray.h>
#include <Hq/Math/Utils.h>
#include <Hq/Math/Vector.h>

#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <SDL.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;
const int SAMPLES       = 50;
const int MAX_DEPTH     = 20;

using namespace hq;
using namespace hq::math;

void ClearSurface(SDL_Surface* surface)
{
    assert(nullptr != surface);
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 0x0, 0x0, 0x0));
}

void SetPixel(SDL_Surface* surface, Uint32 x, Uint32 y, SDL_Color color, float blendFactor = 1.f)
{
    assert(surface != nullptr);
    assert(blendFactor <= 1.f && blendFactor >= 0.f);

    void*            pixels               = surface->pixels;
    int              surfacePitch         = surface->pitch;
    Uint8            surfaceBytesPerPixel = surface->format->BytesPerPixel;
    SDL_PixelFormat* surfaceFormat        = surface->format;

    Uint8* pixel = static_cast<Uint8*>(pixels);
    pixel += surfacePitch * y + surfaceBytesPerPixel * x;
    if (blendFactor == 1.f)
    {
        // convert and copy
        Uint32 col = SDL_MapRGB(surfaceFormat, color.r, color.g, color.b);
        memcpy(pixel, &col, surfaceBytesPerPixel);
        return;
    }

    // else we need to get existing color for blending
    Uint32    bufferCol = 0;
    SDL_Color bufferColor;
    memcpy(&bufferCol, pixel, surfaceBytesPerPixel);
    SDL_GetRGB(bufferCol, surfaceFormat, &bufferColor.r, &bufferColor.g, &bufferColor.b);
    SDL_Color blendedColor;
    // and blend
    blendedColor.r = Uint8(color.r * blendFactor + (1.f - blendFactor) * bufferColor.r);
    blendedColor.g = Uint8(color.g * blendFactor + (1.f - blendFactor) * bufferColor.g);
    blendedColor.b = Uint8(color.b * blendFactor + (1.f - blendFactor) * bufferColor.b);
    // convert and copy
    Uint32 col = SDL_MapRGB(surfaceFormat, blendedColor.r, blendedColor.g, blendedColor.b);
    memcpy(pixel, &col, surfaceBytesPerPixel);
}

SDL_Color GetPixel(SDL_Surface* surface, Uint32 x, Uint32 y)
{
    assert(surface != nullptr);

    void*            pixels               = surface->pixels;
    int              surfacePitch         = surface->pitch;
    Uint8            surfaceBytesPerPixel = surface->format->BytesPerPixel;
    SDL_PixelFormat* surfaceFormat        = surface->format;

    SDL_Color color;
    Uint32    col   = 0;
    Uint8*    pixel = static_cast<Uint8*>(pixels);
    pixel += surfacePitch * y + surfaceBytesPerPixel * x;
    memcpy(&col, pixel, surfaceBytesPerPixel);
    SDL_GetRGB(col, surfaceFormat, &color.r, &color.g, &color.b);
    return color;
}

class StbImage
{
public:
    StbImage() {}
    bool load(const std::string& filename)
    {
        int channels;
        _image = std::shared_ptr<unsigned char>(stbi_load(filename.c_str(), &_width, &_height, &channels, 4),
                                                StbImage::freeImage);
        return _image != nullptr;
    }

    const unsigned char* data() const
    {
        return _image.get();
    }

    int width() const
    {
        return _width;
    }
    int height() const
    {
        return _height;
    }

private:
    static void freeImage(unsigned char* image)
    {
        stbi_image_free(image);
    }
    std::shared_ptr<unsigned char> _image  = {nullptr};
    int                            _width  = {0};
    int                            _height = {0};
};

void createRandomScene(HitableList& world)
{
    world.list.push_back(new Sphere(Vector3f(0.f, -1000.f, 0.f), 1000.f,
                                    std::make_unique<Lambertian>(std::make_shared<CheckerTexture>(
                                        std::make_shared<ColorTexture>(Vector3f(.5f, .5f, .5f)),
                                        std::make_shared<ColorTexture>(Vector3f(.2f, .3f, .1f))))));
    for (int a = -11; a < 11; ++a)
        for (int b = -11; b < 11; ++b)
        {
            float    chooseMat = rand01();
            Vector3f center(a + 0.9f * rand01(), 0.2f, b + 0.9f * rand01());
            if (length(center - Vector3f(4.f, .2f, 0.f)) > 0.9f)
            {
                if (chooseMat < .8f)
                {
                    world.list.push_back(
                        new Sphere(center, .2f,
                                   std::make_unique<Lambertian>(std::make_shared<ColorTexture>(
                                       Vector3f(rand01() * rand01(), rand01() * rand01(), rand01() * rand01())))));
                }
                else if (chooseMat < .95f)
                {
                    world.list.push_back(
                        new Sphere(center, .2f,
                                   std::make_unique<Metal>(
                                       Vector3f(.5f * (1 + rand01()), .5f * (1 + rand01()), .5f * (1 + rand01())))));
                }
                else
                {
                    world.list.push_back(new Sphere(center, .2f, std::make_unique<Dielectric>(1.5f)));
                }
            }
        }

    world.list.push_back(new Sphere(Vector3f(0.f, 1.f, 0.f), 1.f, std::make_unique<Dielectric>(1.5f)));
    world.list.push_back(
        new Sphere(Vector3f(-4.f, 1.f, 0.f), 1.f,
                   std::make_unique<Lambertian>(std::make_shared<ColorTexture>(Vector3f(.4f, .2f, .1f)))));
    world.list.push_back(
        new Sphere(Vector3f(4.f, 1.f, 0.f), 1.f, std::make_unique<Metal>(Vector3f(.7f, .6f, .5f), 0.f)));
}

void createScenePerlinTest(HitableList& world)
{
    std::shared_ptr<NoiseTexture> noiseTexture = std::make_shared<NoiseTexture>(FastNoise::SimplexFractal);
    noiseTexture->noise.SetFrequency(1.f);

    world.list.push_back(new Sphere(Vector3f(0.f, -1000.f, 0.f), 1000.f, std::make_unique<Lambertian>(noiseTexture)));
    world.list.push_back(new Sphere(Vector3f(0.f, 2.f, 0.f), 2.f, std::make_unique<Dielectric>(1.5f)));
    world.list.push_back(new Sphere(Vector3f(0.f, 2.f, 0.f), 1.5f, std::make_unique<Lambertian>(noiseTexture)));
}

std::vector<StbImage> createTexturedScene(HitableList& world)
{
    std::vector<StbImage> resources;

    StbImage moon;
    if (moon.load("moonmap2k.jpg"))
    {
        resources.emplace_back(moon);
        std::shared_ptr<NoiseTexture> noiseTexture = std::make_shared<NoiseTexture>(FastNoise::SimplexFractal);

        noiseTexture->noise.SetFrequency(1.f);
        world.list.push_back(
            new Sphere(Vector3f(0.f, -1000.f, 0.f), 1000.f, std::make_unique<Lambertian>(noiseTexture)));
        world.list.push_back(new Sphere(
            Vector3f(0.f, 2.f, 0.f), 1.5f,
            std::make_unique<Lambertian>(std::make_shared<ImageTexture>(moon.data(), moon.width(), moon.height()))));
        world.list.push_back(
            new Sphere(Vector3f(1.f, 1.f, 2.f), 0.5f,
                       std::make_unique<DiffuseLight>(std::make_shared<ColorTexture>(Vector3f(2.f, 2.f, 2.f)))));
    }

    return resources;
}

int main(int /*argc*/, char** /*argv*/)
{
    SDL_Window*  window  = nullptr;
    SDL_Surface* surface = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could no initialize! SDL_Error: " << SDL_GetError() << ".\n";
        return -1;
    }

    window = SDL_CreateWindow("Raytracey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (nullptr == window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << ".\n";
        return -1;
    }

    surface = SDL_GetWindowSurface(window);
    ClearSurface(surface);
    JobManager jobMgr;
    jobMgr.init();
    jobMgr.wait();

    using namespace std::chrono;
    high_resolution_clock::time_point lastTime = high_resolution_clock::now();

    std::mt19937                          rng;
    std::uniform_real_distribution<float> distribution;

    bool running = true;
    // Event handler
    SDL_Event   e;
    Uint32      y = 0;
    Vector3f    eye(6.5f, 2.f, 1.5f);
    Vector3f    lookAt(0.f, 0.f, 0.f);
    float       focusDist = length(eye - lookAt);
    float       aperture  = 0.f;
    Camera      cam(eye, lookAt, Vector3f(0.f, 1.f, 0.f), 45, float(SCREEN_WIDTH) / float(SCREEN_HEIGHT), aperture,
               focusDist, 0.f, 1.f);
    HitableList world;
    //    world.list.push_back(
    //        new Sphere(Vector3f(0.f, 0.f, -1.f), 0.5f, std::make_unique<Lambertian>(math::Vector3f(.8f, .3f, .3f))));
    //    world.list.push_back(
    //        new Sphere(Vector3f(0.f, -100.5f, -1.f), 100.f, std::make_unique<Lambertian>(math::Vector3f(.8f, .8f,
    //        .3f))));
    //    world.list.push_back(
    //        new Sphere(Vector3f(1.f, 0.f, -1.f), 0.5f, std::make_unique<Metal>(math::Vector3f(.8f, .6f, .2f), 0.3f)));
    //    world.list.push_back(new Sphere(Vector3f(-1.f, 0.f, -1.f), 0.5f, std::make_unique<Dielectric>(1.5f)));
    //    world.list.push_back(new Sphere(Vector3f(-1.f, 0.f, -1.f), -0.45f, std::make_unique<Dielectric>(1.5f)));
    //    createRandomScene(world);
    //    createScenePerlinTest(world);
    std::vector<StbImage> resources = createTexturedScene(world);
    BvhNode               bvhRoot(world.list, 0.f, 1.f);
    while (running)
    {
        // Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_KEYDOWN)
            {
                SDL_Keycode keyPressed = e.key.keysym.sym;

                switch (keyPressed)
                {
                    case SDLK_ESCAPE:
                        running = false;
                        break;
                    default:
                        break;
                }
            }
            // User requests quit
            if (e.type == SDL_QUIT)
            {
                running = false;
            }
        }

        high_resolution_clock::time_point nowTime     = high_resolution_clock::now();
        duration<double>                  elapsedTime = duration_cast<duration<double> >(nowTime - lastTime);

        if (y < SCREEN_HEIGHT)
        {
            for (Uint32 x = 0; x < SCREEN_WIDTH; ++x)
            {
                // main processing job (captures stuff)
                auto color = [=, &cam, &bvhRoot](void*, size_t) {
                    Vector3f colorVec;
                    for (int i = 0; i < SAMPLES; ++i)
                    {
                        float u = (float(x) + rand01()) / SCREEN_WIDTH;
                        float v = (float(SCREEN_HEIGHT - y - 1) + rand01()) / SCREEN_HEIGHT;
                        Rayf  r = cam.getRay(u, v);

                        auto colorImpl = [](const Rayf& r, const BvhNode& bvhRoot, int depth,
                                            auto& colorRef) -> Vector3f {
                            Vector3f colorVec;
                            HitData  hitData;
                            if (bvhRoot.hit(r, 0.001f, std::numeric_limits<float>::max(), hitData))
                            {
                                math::Rayf     scattered;
                                math::Vector3f attenuation;
                                math::Vector3f emitted =
                                    hitData.materialPtr->emitted(hitData.uv.u, hitData.uv.v, hitData.p);
                                if (depth < MAX_DEPTH &&
                                    hitData.materialPtr->scatter(r, hitData, attenuation, scattered))
                                {
                                    return emitted + attenuation * colorRef(scattered, bvhRoot, depth + 1, colorRef);
                                }
                                else
                                {
                                    return emitted;
                                }
                            }
                            else
                            {
                                //                                float t  = 0.5f * (r.direction().y + 1.f);
                                //                                colorVec = (1.f - t) * Vector3f(1.f, 1.f, 1.f) + t *
                                //                                Vector3f(.3f, .5f, 1.f);
                                colorVec = Vector3f(0.f, 0.f, 0.f);
                            }

                            return colorVec;
                        };

                        colorVec += colorImpl(r, bvhRoot, 0, colorImpl);
                    }

                    SDL_Color color;
                    color.r = Uint8(255.99f * (std::sqrt(colorVec.r / SAMPLES)));
                    color.g = Uint8(255.99f * (std::sqrt(colorVec.g / SAMPLES)));
                    color.b = Uint8(255.99f * (std::sqrt(colorVec.b / SAMPLES)));
                    color.a = 255;
                    SetPixel(surface, x, y, color);
                };

                jobMgr.addJob(color, nullptr);
            }

            ++y;
            jobMgr.wait();
        }

        if (elapsedTime.count() > 0.016)
        {
            lastTime = nowTime;
            // Update the surface every second
            SDL_UpdateWindowSurface(window);
        }
    }

    jobMgr.release();

    bvhRoot.release();

    for (auto* hitable : world.list)
    {
        delete hitable;
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
