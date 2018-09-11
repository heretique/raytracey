#define SDL_MAIN_HANDLED

#define FMT_HEADER_ONLY
#include "HitableList.h"
#include "JobManager.h"
#include "camera.h"
#include "fmt/printf.h"
#include "material.h"
#include "ray.h"
#include "sphere.h"
#include "utils.h"
#include "vector.h"

#include <chrono>
#include <limits>
#include <random>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 400;
const int SAMPLES       = 200;

using namespace math;

void ClearSurface(SDL_Surface* surface)
{
    assert(nullptr != surface);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x0, 0x0, 0x0));
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
    blendedColor.r = color.r * blendFactor + (1.f - blendFactor) * bufferColor.r;
    blendedColor.g = color.g * blendFactor + (1.f - blendFactor) * bufferColor.g;
    blendedColor.b = color.b * blendFactor + (1.f - blendFactor) * bufferColor.b;
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

int main(int argc, char** argv)
{
    SDL_Window*  window  = nullptr;
    SDL_Surface* surface = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fmt::print("SDL could no initialize! SDL_Error: {}.\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Raytracey", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (nullptr == window)
    {
        fmt::print("Window could not be created! SDL_Error: {}.\n", SDL_GetError());
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
    SDL_Event e;
    Uint32    y = 0;
    Vector3f  eye(3.f, 3.f, 2.f);
    Vector3f  lookAt(0.f, 0.f, -1.f);
    float     focusDist = length(eye - lookAt);
    float     aperture  = .2f;
    Camera    cam(eye, lookAt, Vector3f(0.f, 1.f, 0.f), 45, float(SCREEN_WIDTH) / float(SCREEN_HEIGHT), aperture,
               focusDist);
    HitableList world;
    world.list.push_back(
        new Sphere(Vector3f(0.f, 0.f, -1.f), 0.5f, std::make_unique<Lambertian>(math::Vector3f(.8f, .3f, .3f))));
    world.list.push_back(
        new Sphere(Vector3f(0.f, -100.5f, -1.f), 100.f, std::make_unique<Lambertian>(math::Vector3f(.8f, .8f, .3f))));
    world.list.push_back(
        new Sphere(Vector3f(1.f, 0.f, -1.f), 0.5f, std::make_unique<Metal>(math::Vector3f(.8f, .6f, .2f), 0.3f)));
    world.list.push_back(new Sphere(Vector3f(-1.f, 0.f, -1.f), 0.5f, std::make_unique<Dielectric>(1.5f)));
    world.list.push_back(new Sphere(Vector3f(-1.f, 0.f, -1.f), -0.45f, std::make_unique<Dielectric>(1.5f)));
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
            for (int x = 0; x < SCREEN_WIDTH; ++x)
            {
                // main processing job (captures stuff)
                auto color = [=, &cam, &world](void*, size_t) {
                    Vector3f colorVec;
                    for (int i = 0; i < SAMPLES; ++i)
                    {
                        float u = (float(x) + rand01()) / SCREEN_WIDTH;
                        float v = (float(SCREEN_HEIGHT - y - 1) + rand01()) / SCREEN_HEIGHT;
                        Rayf  r = cam.getRay(u, v);

                        auto colorImpl = [](const Rayf& r, const HitableList& world, int depth,
                                            auto& colorRef) -> Vector3f {
                            Vector3f colorVec;
                            HitData  hitData;
                            if (world.hit(r, 0.001f, std::numeric_limits<float>::max(), hitData))
                            {
                                math::Rayf     scattered;
                                math::Vector3f attenuation;
                                if (depth < 20 && hitData.materialPtr->scatter(r, hitData, attenuation, scattered))
                                {
                                    return attenuation * colorRef(scattered, world, depth + 1, colorRef);
                                }
                                else
                                {
                                    return math::Vector3f(0.f, 0.f, 0.f);
                                }
                            }
                            else
                            {
                                float t  = 0.5f * (r.direction().y + 1.f);
                                colorVec = (1.f - t) * Vector3f(1.f, 1.f, 1.f) + t * Vector3f(.3f, .5f, 1.f);
                            }

                            return colorVec;
                        };

                        colorVec += colorImpl(r, world, 0, colorImpl);
                    }

                    SDL_Color color;
                    color.r = 255.99f * (std::sqrt(colorVec.r / SAMPLES));
                    color.g = 255.99f * (std::sqrt(colorVec.g / SAMPLES));
                    color.b = 255.99f * (std::sqrt(colorVec.b / SAMPLES));
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

    for (auto* hitable : world.list)
    {
        delete hitable;
    }

    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
