#define SDL_MAIN_HANDLED

#define FMT_HEADER_ONLY
#include "JobManager.h"
#include "fmt/printf.h"
#include "ray.h"
#include "vector.h"
#include <chrono>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE     = 32;

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

bool HitSphere(const Vector3f& center, float radius, const Rayf& r)
{
    Vector3f oc           = r.origin() - center;
    float    a            = dot(r.direction(), r.direction());
    float    b            = 2.f * dot(oc, r.direction());
    float    c            = dot(oc, oc) - radius * radius;
    float    discriminant = b * b - 4 * a * c;
    return discriminant > 0;
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

    bool running = true;
    // Event handler
    SDL_Event e;
    Uint32    y = 0;
    Vector3f  lowerLeft(-2.f, -1.f, -1.f);
    Vector3f  horizontal(4.f, 0.f, 0.f);
    Vector3f  vertical(0.f, 2.f, 0.f);
    Vector3f  origin(0.f, 0.f, 0.f);

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
                float u = float(x) / SCREEN_WIDTH;
                float v = float(SCREEN_HEIGHT - y - 1) / SCREEN_HEIGHT;
                Rayf  r(origin, lowerLeft + u * horizontal + v * vertical);

                auto color = [=](void*, size_t) {
                    Vector3f colorVec;
                    if (HitSphere(Vector3f(0.f, 0.f, -1.f), 0.5, r))
                    {
                        colorVec = Vector3f(1.f, 0.f, 0.f);
                    }
                    else
                    {
                        float t  = 0.5f * (r.direction().y + 1.f);
                        colorVec = (1.f - t) * Vector3f(1.f, 1.f, 1.f) + t * Vector3f(.3f, .5f, 1.f);
                    }
                    SDL_Color color;
                    color.r = 255.99f * colorVec.r;
                    color.g = 255.99f * colorVec.g;
                    color.b = 255.99f * colorVec.b;
                    color.a = 255;
                    SetPixel(surface, x, y, color);
                };

                jobMgr.addJob(color, nullptr);
            }

            ++y;
            jobMgr.wait();
        }

        if (elapsedTime.count() > 1.0)
        {
            lastTime = nowTime;
            // Update the surface every second
            SDL_UpdateWindowSurface(window);
        }
    }

    jobMgr.release();
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
