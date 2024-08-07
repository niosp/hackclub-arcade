#include <array>
#include <iostream>
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stack>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>

const int SCALE_FACTOR = 1;
const int EMULATOR_WIDTH = 64;
const int EMULATOR_HEIGHT = 32;

const int default_color_r = 255;
const int default_color_g = 255;
const int default_color_b = 255;

const int entry_point = 0x200;

uint8_t display[EMULATOR_WIDTH * SCALE_FACTOR][EMULATOR_HEIGHT * SCALE_FACTOR] = {0};

const uint8_t font_data[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

enum register_enum
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

void print_current_state_to_console() {
    for (int y = 0; y < EMULATOR_HEIGHT; ++y) {
        for (int x = 0; x < EMULATOR_WIDTH; ++x) {
            if (display[x][y] == 1) {
                std::cout << '#';
            }
            else {
                std::cout << '.';
            }
        }
        std::cout << "\n";
    }
	std::cout << "\n\n";
}

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
    /* print filename */
    std::cout << argv[1] << "\n";
	/* prepare file operations */
    std::ifstream input_file(argv[1], std::ios::binary | std::ios::ate);
    const std::streamsize file_size = input_file.tellg();
    /* check filesize, since memory is limited */
    if (file_size > 3584) return -1;
    /* later, read from the beginning of the file stream */
    input_file.seekg(0, std::ios::beg);
    /* allocate the necessary vector size */
    std::vector<uint8_t> file_content(4096);

    /* read the file! */ 
    if(!input_file.read(reinterpret_cast<char*>(file_content.data() + entry_point), file_size))
    {
        std::cerr << "Error while reading file\n";
        return -1;
    }

    /* close input file */

    /* insert font data into memory */
    // file_content.insert(file_content.begin() + 0x50, &font_data[0], &font_data[80]);


    std::ifstream file(argv[1], std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
    }

    unsigned char byte;
    int count = 0;

    while (file.read(reinterpret_cast<char*>(&byte), sizeof(byte))) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte) << ' ';
        if (++count % 16 == 0) { // New line every 16 bytes
            std::cout << std::endl;
        }
    }

    if (count % 16 != 0) { // Ensure to end with a new line if the last line is not complete
        std::cout << std::endl;
    }

    file.close();


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

    /*
    void* pixels;
    int pitch;
    if (SDL_LockTexture(sdl_texture, NULL, &pixels, &pitch) != 0) {
        std::cerr << "Failed to lock texture! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    */
    SDL_SetRenderDrawColor(sdl_renderer, 255, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
    SDL_RenderPresent(sdl_renderer);

    std::array<uint8_t, 16> registers = {0};

    int16_t register_PC = entry_point;
    int16_t register_I = 0;

    int8_t delay_timer = 0;
    int8_t sound_timer = 0;

    std::stack<uint16_t> program_stack = {};

    SDL_Event e;

    /*
     * Instructions are from here: https://en.wikipedia.org/wiki/CHIP-8
     * todo: implement the necessary instructions!
     * 00E0: clear screen, done
     * 1NNN: jump to NNN, done
     * 00EE: return from subroutine, 
     * 6XNN: set the register VX
     *
     *
     *
     */

    while(register_PC + 1 < file_content.size())
    {
        SDL_PollEvent(&e);
	    /* fetch instruction */
        uint16_t instruction = (file_content[register_PC] << 8) | file_content[register_PC + 1];
        uint8_t inst_first = file_content[register_PC];
        uint8_t inst_second = file_content[register_PC + 1];

        /* retrieve the nibbles */
        uint8_t nibble_1 = inst_first >> 4;
        uint8_t nibble_2 = inst_first & 15;
        uint8_t nibble_3 = inst_second >> 4;
        uint8_t nibble_4 = inst_second & 15;

        /* switch over the first nibble */
        switch(instruction & 0xF000)
        {
        case 0x00:
	        {
                /* switch over third + fourth nibble */
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
						std::cout << "default case 1\n";
		        		break;
			        }
		        }
                break;
	        }
        case 0x1000:
	        {
        		std::cout << "jump instruction" << std::endl;
                register_PC = instruction & 0x0FFF;
                std::cout << "asdasd" << std::endl;
                break;
	        }
        case 0x2000:
	        {
        		break;
	        }
        case 0x6000:
	        {
        		registers[nibble_2] = inst_second;
        		break;
	        }
        case 0x7000:
	        {
				registers[nibble_2] += inst_second;
        		break;
	        }
        case 0xA000:
	        {
				register_I = (instruction & 0x0FFF);
                std::cout << "asd" << std::endl;
        		break;
	        }
        case 0xD000:
	        {
        		const uint8_t x_coordinate = registers[nibble_2];
                const uint8_t y_coordinate = registers[nibble_3];

                std::cout << "X: " << (int)x_coordinate << " Y: " << (int)y_coordinate << "\n";

                const uint8_t height = nibble_4;

                registers[VF] = 0x00;

                /* draw "row" rows until height is reached */
                for(uint8_t row=0; row < height; row++)
                {
                    uint8_t sprite_data = file_content[register_I + row];
                    /* draw 8 pixels for each row */
	                for(uint8_t col=0; col < 8; col++)
	                {
                        uint8_t pixel_x = (x_coordinate + col) % EMULATOR_WIDTH;
                        uint8_t pixel_y = (y_coordinate + row) % EMULATOR_HEIGHT;

                        uint8_t current_pixel = (sprite_data & (1 << (7 - col))) >> (7 - col);


	                        if(current_pixel && display[pixel_x][pixel_y])
	                        {
                                registers[VF] = 0x01;
	                        }
                                SDL_Rect rectangle;
                                rectangle.x = pixel_x;
                                rectangle.y = pixel_y;
                                rectangle.h = 1;
                                rectangle.w = 1;

                                SDL_SetRenderDrawColor(sdl_renderer, 100, 0, 0, 0);
                                SDL_RenderFillRect(sdl_renderer, &rectangle);
	                        	SDL_RenderPresent(sdl_renderer);
                                display[pixel_x][pixel_y] ^= 1;
                        /*
                        if (current_pixel)
                        {
                            if (display[pixel_x][pixel_y] == 1)
                            {
                                registers[VF] = 0x01;
                            }
                            display[pixel_x][pixel_y] ^= 1;
                        }
                        */

	                }

                    print_current_state_to_console();

                }



        		break;
	        }
        default:
	        {
            std::cout << "default case 2\n";
        		break;
	        }
        }

        register_PC += 2;
    }

    /* try to destroy the created windows, textures etc.! */
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
