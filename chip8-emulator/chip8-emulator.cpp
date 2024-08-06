#include <array>
#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stack>
#include <vector>
#include <fstream>
#include <sstream>

const int SCALE_FACTOR = 10;
const int EMULATOR_WIDTH = 64;
const int EMULATOR_HEIGHT = 32;

const int default_color_r = 255;
const int default_color_g = 255;
const int default_color_b = 255;

enum registers
{
    V0 = 0x00,
    V1 = 0x01,
    V2 = 0x02,
    V3 = 0x03,
    V4 = 0x04,
    V5 = 0x05,
    V6 = 0x06,
    V7 = 0x07,
    V8 = 0x08,
    V9 = 0x09,
    VA = 0x0A,
    VB = 0x0B,
    VC = 0x0C,
    VD = 0x0D,
    VE = 0x0E,
    VF = 0x0F /* flag register */
};

uint16_t read_2_bytes(std::vector<uint8_t>& memory, uint32_t index) {
    if (index + 1 >= memory.size()) {
        throw std::out_of_range("Index out of bounds!!!");
    }
    return (static_cast<uint16_t>(memory[index]) << 8) | static_cast<uint16_t>(memory[index + 1]);
}


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
    if (argc < 2) return 0;
	/* prepare file operations */
    std::ifstream input_file(argv[1], std::ios::binary | std::ios::ate);
    const std::streamsize file_size = input_file.tellg();
    input_file.seekg(0, std::ios::beg);

    /* allocate the necessary vector size */
	std::vector<uint8_t> file_content(file_size);

    /* read the file! */ 
    if(!input_file.read(reinterpret_cast<char*>(file_content.data()), file_size))
    {
        std::cerr << "Error while reading file\n";
        return -1;
    }

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

    std::array<uint8_t, 16> registers;

    int16_t register_PC = 0;
    int16_t register_I = 0;

    int8_t delay_timer = 0;
    int8_t sound_timer = 0;

    std::stack<uint16_t> program_stack;

    SDL_Event e;

    uint16_t num = 0x00 + 0xe0;

    while(register_PC + 1 < file_content.size())
    {
        SDL_PollEvent(&e);
	    /* fetch instruction */
        uint16_t instruction = file_content[register_PC] + file_content[register_PC + 1];
        uint8_t inst_first = file_content[register_PC];
        uint8_t inst_second = file_content[register_PC + 1];

        /* retrieve the nibbles */
        uint8_t nibble_1 = inst_first >> 4;
        uint8_t nibble_2 = inst_first & 15;
        uint8_t nibble_3 = inst_second >> 4;
        uint8_t nibble_4 = inst_second & 15;

        std::cout << "Instruction: 0x" << std::hex << inst_first << std::dec << std::endl;
        std::cout << "Instruction 2 : 0x" << std::hex << inst_second << std::dec << std::endl;

        uint16_t t_test = instruction & 0xF000; /* -> t_test will only contain data from first nibble */
        uint16_t t_test2 = instruction & 0x00FF; /* -> only data from third and fourth nibble */

        switch(instruction & 0xF000) 
        {
            /* clear screen */
			case 0x0000:
	        {
                    switch(instruction & 0x00FF)
                    {
						case 0x00e0:
                        {
                            std::cout << "clearing screen" << std::endl;
                            SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 0);
                            SDL_RenderClear(sdl_renderer);
                            SDL_RenderPresent(sdl_renderer);
                            break;
                        }
						default:
                        {
                            break;
                        }

                    }
	        }
			default:
            {
					std::cout << "default case" << std::endl;
					break;
            }
        }

        register_PC += 2;
        Sleep(2000);

    }

    /* try to destroy the created windows, textures etc.! */
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
