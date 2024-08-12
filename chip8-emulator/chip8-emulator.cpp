#include <array>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <stack>
#include <vector>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

#include <SDL2/SDL.h>

uint8_t delay_timer = 0;
uint8_t sound_timer = 0;
uint8_t sound_on = 0;

uint8_t logging_enabled = 1;

const int SCALE_FACTOR = 10;
const int EMULATOR_WIDTH = 64;
const int EMULATOR_HEIGHT = 32;

const int default_color_r = 0;
const int default_color_g = 5;
const int default_color_b = 82;

const int default_bg_color_r = 0;
const int default_bg_color_g = 0;
const int default_bg_color_b = 0;

const int entry_point = 0x200;
const int font_loaded_at = 0x050;

const uint16_t delay_steps = 20;

const int sound_frequency = 44100;
const int sound_amplitude = 500;
const int sound_sample_rate = 44100;
const int sound_samples = 2048;

SDL_AudioDeviceID audio_device;

uint8_t display[EMULATOR_WIDTH][EMULATOR_HEIGHT] = { 0 };

uint8_t keys[322] = {0};

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

void log(const char* logging_text)
{
    if (logging_enabled) std::printf("[LOG] %s\n", logging_text);
}

uint16_t get_keycode_for_chipkey(uint8_t chipkey)
{
	switch(chipkey)
	{
        /* row 1 */
	case 0x01:return SDL_SCANCODE_1;
	case 0x02:return SDL_SCANCODE_2;
	case 0x03:return SDL_SCANCODE_3;
	case 0x0c:return SDL_SCANCODE_4;
		/* row 2 */
	case 0x04:return SDL_SCANCODE_Q;
	case 0x05:return SDL_SCANCODE_W;
	case 0x06:return SDL_SCANCODE_E;
	case 0x0d:return SDL_SCANCODE_R;
        /* row 3 */
	case 0x07:return SDL_SCANCODE_A;
	case 0x08:return SDL_SCANCODE_S;
	case 0x09:return SDL_SCANCODE_D;
	case 0x0e:return SDL_SCANCODE_F;
        /* row 4 */
	case 0x0a:return SDL_SCANCODE_Z;
	case 0x00:return SDL_SCANCODE_X;
	case 0x0b:return SDL_SCANCODE_C;
	case 0x0f:return SDL_SCANCODE_V;
	default:
        log("requested key that does not exist");
		return 0;
	}
}

uint8_t get_chipkey_for_keycode(uint16_t keycode)
{
	switch(keycode)
	{
	case SDL_SCANCODE_1:return 0x01;
	case SDL_SCANCODE_2:return 0x02;
	case SDL_SCANCODE_3:return 0x03;
	case SDL_SCANCODE_4:return 0x0c;
	case SDL_SCANCODE_Q:return 0x04;
	case SDL_SCANCODE_W:return 0x05;
	case SDL_SCANCODE_E:return 0x06;
	case SDL_SCANCODE_R:return 0x0d;
	case SDL_SCANCODE_A:return 0x07;
	case SDL_SCANCODE_S:return 0x08;
	case SDL_SCANCODE_D:return 0x09;
	case SDL_SCANCODE_F:return 0x0e;
	case SDL_SCANCODE_Z:return 0x0a;
	case SDL_SCANCODE_X:return 0x00;
	case SDL_SCANCODE_C:return 0x0b;
	case SDL_SCANCODE_V:return 0x0f;
	default:return 20; /* return 20 if pressed key is not covered by emulator */
	}
}

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

void audio_callback(void* userdata, Uint8* stream, int len) {
    Sint16* buf = (Sint16*)stream;
    const int length = len / 2;

    for (int i = 0; i < length; i++) {
        if (sound_on) {
            buf[i] = sound_amplitude * (i % (sound_sample_rate / 440) < (sound_sample_rate / 880) ? 1 : -1);
        }
        else {
            buf[i] = 0;
        }
    }
}

void init_audio() {
    SDL_AudioSpec want;
    SDL_AudioSpec have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = sound_frequency;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = sound_samples;
    want.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);

    if (audio_device == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << "\n";
        std::exit(1);
    }

    SDL_PauseAudioDevice(audio_device, 0);
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
    return SDL_CreateWindow("CHIP-8",
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
    std::string input_filename;
    if (argc < 2)
    {
        std::cout << "Filepath: ";
        std::cin >> input_filename;
    }else
    {
        input_filename = argv[1];
    }

    /* print filename */
    log(argv[1]);

	/* prepare file operations */
    std::ifstream input_file(input_filename, std::ios::binary | std::ios::ate);
    const std::streamsize file_size = input_file.tellg();

    /* check filesize, since memory is limited */
    if (file_size > (4096 - entry_point)) {
        std::cerr << "ROM size is too large to fit in memory\n";
        return -1;
    }

    /* later, read from the beginning of the file stream */
    input_file.seekg(0, std::ios::beg);

    /* allocate the necessary vector size */
    std::vector<uint8_t> file_content(4096, 0);

    /* read the file! */ 
    if(!input_file.read(reinterpret_cast<char*>(file_content.data() + entry_point), file_size))
    {
        std::cerr << "Error while reading file\n";
        return -1;
    }

    /* close input file */
    input_file.close();

    /* insert font data into memory */
    std::copy(font_data, font_data + 80, file_content.begin() + font_loaded_at);

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

    /* init sdl audio subsystem */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL audio: " << SDL_GetError() << "\n";
        return 1;
    }

    /* init sdl audio */
    init_audio();

	int* size = nullptr;
    const Uint8* keyboard_state = SDL_GetKeyboardState(size);

    /* set the window color to black */
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
    SDL_RenderPresent(sdl_renderer);

    /* init registers */
    std::array<uint8_t, 16> registers = {0};

    uint16_t register_PC = entry_point;
    uint16_t register_I = 0;

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    std::stack<uint16_t> program_stack;

    SDL_Event e;

    size_t instructions = 0;
    auto before_execution = std::chrono::high_resolution_clock::now();

    size_t set_delay = 3500;

    /* main loop */
    while(register_PC + 1 < file_content.size())
    {
        /* poll events (needed to quit, get keystrokes) */
        SDL_PollEvent(&e);
        SDL_PumpEvents();
        if(e.type == SDL_QUIT)
        {
            goto end;
        }

	    /* fetch instruction */
        uint16_t instruction = (file_content[register_PC] << 8) | file_content[register_PC + 1];
        uint8_t inst_first = file_content[register_PC];
        uint8_t inst_second = file_content[register_PC + 1];

        /* retrieve the nibbles */
        uint8_t nibble_1 = inst_first >> 4;
        uint8_t nibble_2 = inst_first & 15;
        uint8_t nibble_3 = inst_second >> 4;
        uint8_t nibble_4 = inst_second & 15;

        uint8_t increase_pc = 1;

    	if (keyboard_state[SDL_SCANCODE_6])
        {
         	log("");
            set_delay -= delay_steps;
        }

        if (keyboard_state[SDL_SCANCODE_7])
        {
            std::cout << "increased\n\n";
            set_delay += delay_steps;
        }

        /* switch over the first nibble */
        switch(instruction & 0xF000)
        {
        case 0x00:
	        {
                /* switch over third + fourth nibble */
		        switch(instruction & 0x00FF)
		        {
		        	/* clear content */
		        case 0x00e0:
			        {
                        /* set all bits in the array to 0 */
                        for(int i=0; i < EMULATOR_WIDTH; i++)
                        {
	                        for(int j=0; j < EMULATOR_HEIGHT; j++)
	                        {
                                display[i][j] = 0;
	                        }
                        }

		        		/* render window with color black */
						SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
                        SDL_RenderClear(sdl_renderer);
                        log("00E0");
		        		break;
			        }
                    /* return from a subroutine */
		        case 0x00ee:
			        {
						register_PC = program_stack.top();
                        program_stack.pop();
                        log("00EE");
						break;
			        }
                    /* no operation */
		        case 0x00ff:
			        {
						log("00FF");
                        break;
			        }
		        default:
			        {
						log("default case");
                        std::cout << "INSTR: " << (instruction) << "\n";
		        		break;
			        }
		        }
                break;
	        }
            /* jump to address NNN */
        case 0x1000:
	        {
        		register_PC = instruction & 0x0FFF;
                increase_pc = 0;
                log("1NNN");
                break;
	        }
            /* call subroutine at NNN, push old program counter to stack */
        case 0x2000:
	        {
				program_stack.push(register_PC);
                register_PC = instruction & 0x0FFF;
                increase_pc = 0;
                log("2NNN");
        		break;
	        }
            /* skip next instruction if reg[nb2] == inst_second */
        case 0x3000:
	        {
				if(registers[nibble_2] == inst_second)
				{
                    register_PC += 2;
				}
                log("3XNN");
        		break;
	        }
            /* skip next instruction if reg[nb2] != inst_second */
        case 0x4000:
	        {
	            if (registers[nibble_2] != inst_second)
	            {
	                register_PC += 2;
	            }
                log("4XNN");
				break;
	        }
            /* skip next instruction if reg[nb2] == reg[nb3] */
        case 0x5000:
	        {
                if(registers[nibble_2] == registers[nibble_3])
                {
                    register_PC += 2;
                }
                log("5XY0");
				break;
	        }
            /* set reg[nb2] == inst_second */
        case 0x6000:
	        {
        		registers[nibble_2] = inst_second;
                log("6XNN");
        		break;
	        }
            /* add NN to reg[nb2] */
        case 0x7000:
	        {
				registers[nibble_2] += inst_second;
				// registers[nibble_2] = (registers[nibble_2] + inst_second) & 0xFF;
                log("7XNN");
        		break;
	        }
            /* 0x8s instructions (math & bit operations) */
        case 0x8000:
	        {
                if(nibble_4 == 0x00) 
                {
                    registers[nibble_2] = registers[nibble_3];
                    log("8XY0");
                }
                else if(nibble_4 == 0x01) /* working */
                {
                    registers[nibble_2] |= registers[nibble_3];
                    log("8XY1");
                }else if(nibble_4 == 0x02) /* working */
                {
                    registers[nibble_2] &= registers[nibble_3];
                    log("8XY2");
                }
                else if (nibble_4 == 0x03) /* working */
                {
                    registers[nibble_2] ^= registers[nibble_3];
                    log("8XY3");
                }else if (nibble_4 == 0x04) /* working */
                {
                    size_t sum = registers[nibble_2] + registers[nibble_3];
                    registers[nibble_2] = sum & 0xFF;
                    registers[VF] = (sum > 255) ? 0x01 : 0x00;
                    log("8XY4");
                }else if (nibble_4 == 0x05)
                {
                    uint8_t x = registers[nibble_2];
                    uint8_t y = registers[nibble_3];
                    if (x >= y) {
                        registers[VF] = 0x01;
                    }
                    else {
                        registers[VF] = 0x00;
                    }
                	registers[nibble_2] = x - y;
                    log("8XY5");

                }else if (nibble_4 == 0x06) /* working */
                {
                    uint8_t lsb = registers[nibble_2] & 0x01;
                    registers[nibble_2] = registers[nibble_2] >> 1;
                    registers[VF] = lsb;
                    log("8XY6");
                }else if(nibble_4 == 0x07)
                {
                    registers[nibble_2] = registers[nibble_3] - registers[nibble_2];
                    registers[VF] = (registers[nibble_3] >= registers[nibble_2]) ? 0x01 : 0x00;
                    log("8XY7");
                }else if(nibble_4 == 0x0e) /* working */
                {
                    uint8_t req_bit = (registers[nibble_2] & 0x80) ? 1 : 0;
                    registers[nibble_2] <<= 1;
                    registers[VF] = req_bit;
                    log("8XYE");
                }
				break;
	        }
            /* skips instruction if reg[nb2] != reg[nb3] */
        case 0x9000:
	        {
		        if(nibble_4 == 0 && (registers[nibble_2] != registers[nibble_3]))
		        {
                    register_PC += 2;
		        }
                log("9XY0");
                break;
	        }
            /* set mem ptr to address NNN */
        case 0xA000:
	        {
				register_I = (instruction & 0x0FFF);
                log("ANNN");
        		break;
	        }
            /* jump to addr NNN plus addr in reg[V0] */
        case 0xB000:
	        {
				register_PC = registers[V0] + (instruction & 0x0FFF);
                increase_pc = 0;
                log("BNNN");
				break;
	        }
            /* random & NN */
        case 0xC000:
	        {
				registers[nibble_2] = rand() & inst_second;
                log("CNNN");
                break;
	        }
            /* draw a sprite */
        case 0xD000:
	        {
	            const uint8_t x_coordinate = registers[nibble_2];
	            const uint8_t y_coordinate = registers[nibble_3];
	            const uint8_t height = nibble_4;

	            registers[VF] = 0x00;

	            /* draw "row" rows until height is reached */
	            for (uint8_t row = 0; row < height; row++)
	            {
	                uint8_t sprite_data = file_content[register_I + row];
	                /* draw 8 pixels for each row */
	                for (uint8_t col = 0; col < 8; col++)
	                {
                        if (x_coordinate + col >= EMULATOR_WIDTH || y_coordinate + row >= EMULATOR_HEIGHT)
                            continue;

                        uint8_t pixel_x = (x_coordinate + col) % EMULATOR_WIDTH;
                        uint8_t pixel_y = (y_coordinate + row) % EMULATOR_HEIGHT;

                        uint8_t current_pixel = (sprite_data & (0x80 >> col)) >> (7 - col);

                        if (current_pixel)
                        {
                            if (display[pixel_x][pixel_y])
                            {
                                registers[VF] = 0x01;
                                display[pixel_x][pixel_y] = 0;
                                SDL_SetRenderDrawColor(sdl_renderer, default_bg_color_r, default_bg_color_g, default_bg_color_b, 255);

                            }
                            else
                            {
                                display[pixel_x][pixel_y] = 1;
                                SDL_SetRenderDrawColor(sdl_renderer, default_color_r, default_color_g, default_color_b, 255);
                            }

                            SDL_Rect rectangle;
                            rectangle.x = pixel_x * SCALE_FACTOR;
                            rectangle.y = pixel_y * SCALE_FACTOR;
                            rectangle.w = SCALE_FACTOR;
                            rectangle.h = SCALE_FACTOR;

                            SDL_RenderFillRect(sdl_renderer, &rectangle);
                        }
	                }
	            }
                SDL_RenderPresent(sdl_renderer);
	            break;
	        }
            /* key detection instructions */
        case 0xE000:
	        {
		        if(inst_second == 0x9e)
		        {
                    if(keyboard_state[get_keycode_for_chipkey(registers[nibble_2])])
                    {
                        register_PC += 2;
                    }
                    log("EX9E");
		        }else if(inst_second == 0xa1)
		        {
                    if (!keyboard_state[get_keycode_for_chipkey(registers[nibble_2])])
                    {
                        register_PC += 2;
                    }
                    log("EXA1");
		        }
                break;
	        }
            /* timers, memory, bcd */
        case 0xF000:
	        {
                if(inst_second == 0x1e)
                {
                    register_I += registers[nibble_2];
                    log("FX1E");
                }
        		else if(inst_second == 0x29)
                {
                    register_I = font_loaded_at + registers[nibble_2] * 5;
                    log("FX29");
                }else if(inst_second == 0x33)
				{
                    uint8_t reg_val = registers[nibble_2];
                    file_content[register_I + 0] = reg_val / 100;
                    file_content[register_I + 1] = (reg_val / 10) % 10;
                    file_content[register_I + 2] = reg_val % 10;
                    log("FX33");
				}else if(inst_second == 0x55)
				{
                    for(int i=0; i <= nibble_2; i++)
                    {
                        file_content[register_I + i] = registers[i];
                    }
                    log("FX55");
				}else if(inst_second == 0x65)
				{
                    for (int i = 0; i <= nibble_2; i++)
                    {
                        registers[i] = file_content[register_I + i];
                    }
                    log("FX65");
				}else if(inst_second == 0x07)
				{
                    registers[nibble_2] = delay_timer;
                    log("FX07");
                    break;
				}else if(inst_second == 0x0a)
				{
                    if(e.type == SDL_KEYDOWN)
                    {
                        uint8_t chip_keycode = get_chipkey_for_keycode(e.key.keysym.scancode);
                        if(chip_keycode != 20)
                        {
                            registers[nibble_2] = chip_keycode;
                        }else
                        {
                            increase_pc = 0;
                        }
                    }
                    log("FX0A");
                    break;
				}else if(inst_second == 0x15)
				{
                    delay_timer = registers[nibble_2];
                    log("FX15");
                    break;
				}else if(inst_second == 0x18)
				{
                    sound_timer = registers[nibble_2];
                    log("FX18");
                    break;
				}
				break;
	        }
        default:
	        {
				log("default case 2");
        		break;
	        }
        }

        /* increase program counter (default) */
        if(increase_pc) register_PC += 2;

        /* just for debugging */ 
        instructions += 1;

        /* decrease the timer */
        if(delay_timer > 0) delay_timer -= 1;

        /* handle sound */
        if (sound_timer > 0)
        {
            sound_on = 1;
            sound_timer -= 1;
        }else if(!sound_timer)
        {
            sound_on = 0;
        }

        std::this_thread::sleep_for(std::chrono::microseconds(set_delay));
    }

    /* label used when trying to quit the application (SDL_QUIT event) */
    end:

    /* timing calculations */
    auto after_execution = std::chrono::high_resolution_clock::now();
    auto final_execution_time = std::chrono::duration_cast<std::chrono::milliseconds>(after_execution - before_execution);

    std::cout << "Time to execute: " << final_execution_time.count() << "\n";
    std::cout << "Instructions: " << instructions << "\n";

    /* de-init sound device */
    SDL_CloseAudioDevice(audio_device);

    /* try to destroy the created windows, textures etc.! */
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
