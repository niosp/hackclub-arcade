#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

const int SCALE_FACTOR = 10;
const int EMULATOR_WIDTH = 64;
const int EMULATOR_HEIGHT = 32;

int8_t init_sdl()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }
    return 0;
}

SDL_Window* create_window()
{
    return SDL_CreateWindow("SDL2 Window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        EMULATOR_WIDTH * SCALE_FACTOR, EMULATOR_HEIGHT * SCALE_FACTOR,
        SDL_WINDOW_SHOWN);
}

SDL_Renderer* create_renderer(SDL_Window* window)
{
    return SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

SDL_Texture* create_texture(SDL_Renderer* renderer)
{
    return SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, EMULATOR_WIDTH * SCALE_FACTOR, EMULATOR_HEIGHT * SCALE_FACTOR);
}

int8_t set_pixel(Uint32* pixelData, int pitch, int x, int y, int r, int g, int b) {
    if (x < 0 || x >= 800 || y < 0 || y >= 600) return -1;
    Uint32 pixel = (r << 24) | (g << 16) | (b << 8) | 0xFF;
    pixelData[y * (pitch / 4) + x] = pixel;
    return 0;
}

int main(int argc, char* argv[]) {
    /* init sdl */
    init_sdl();
    /* create window */
    SDL_Window* sdl_window = create_window();
    if (!sdl_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }
    /* create sdl renderer */
    SDL_Renderer* sdl_renderer = create_renderer(sdl_window);
    if (!sdl_renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return -1;
    }
    /* create sdl texture */
    SDL_Texture* sdl_texture = create_texture(sdl_renderer);
    if (!sdl_texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(sdl_renderer);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return -1;
    }

    void* pixels;
    int pitch;
    if (SDL_LockTexture(sdl_texture, NULL, &pixels, &pitch) != 0) {
        std::cerr << "Failed to lock texture! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    Uint32* pixelData = static_cast<Uint32*>(pixels);

    /*set pixels */
    for (int i = 0; i < 600; ++i) {
        set_pixel(pixelData, pitch, i, i, 255, 0, 0); // Red color
    }

    SDL_UnlockTexture(sdl_texture);
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);

    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    /* try to destroy the created windows, textures etc.! */
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
