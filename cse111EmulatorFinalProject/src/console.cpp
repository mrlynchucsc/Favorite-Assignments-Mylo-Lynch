#include "console.h"

#include <algorithm>
#include <bitset>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#define CONTROLLER_A_MASK ((uint8_t)0x80)
#define CONTROLLER_B_MASK ((uint8_t)0x40)
#define CONTROLLER_SELECT_MASK ((uint8_t)0x20)
#define CONTROLLER_START_MASK ((uint8_t)0x10)
#define CONTROLLER_UP_MASK ((uint8_t)0x08)
#define CONTROLLER_DOWN_MASK ((uint8_t)0x04)
#define CONTROLLER_LEFT_MASK ((uint8_t)0x02)
#define CONTROLLER_RIGHT_MASK ((uint8_t)0x01)

Console::Console(const std::string &filename,
                 const std::vector<std::string> &allowed_extensions)
    : filename_(filename),
      RAM_(0x8000 + 0x8000, 0),
      CPU_(*this, RAM_),
      GPU_(*this, RAM_) {
  bool valid_extension = 0;
  valid_extension |= (allowed_extensions.size() == 0);
  for (const auto &extension : allowed_extensions) {
    valid_extension |= hasExtension(filename, extension);
  }
  if (!valid_extension) {
    std::cerr << "Unsupported file extension." << std::endl;
    exit(1);
  }

  // Open slug file
  std::ifstream file(filename, std::ios::binary | std::ios::ate);

  // Check if file is opened successfully
  if (!file.is_open()) {
    std::cerr << "Error opening file: " << filename << std::endl;
    exit(1);
  }

  // Get the size of the file
  file_size_ = file.tellg();

  // Reset file pointer to the beginning
  file.seekg(0, std::ios::beg);

  // Allocate memory to store the contents of the file
  contents_ = std::make_unique<char[]>(file_size_);

  // Read the contents of the file into the allocated memory
  file.read(contents_.get(), file_size_);

  // Close the file
  file.close();

  // First fill RAM with .slugFile
  std::memcpy(RAM_.data() + 0x8000, contents_.get(), file_size_);
}

bool Console::hasExtension(const std::string &filename,
                           const std::string &extension) {
  return (filename.size() >= extension.size()) &&
         (filename.compare(filename.size() - extension.size(), extension.size(),
                           extension) == 0);
}

void Console::reset() {  // Reset Sequence
  // 1. Clear all of RAM with zeros
  std::fill(RAM_.begin(), RAM_.begin() + kRAMSize, 0);

  // 2. Copy data section to RAM
  std::memcpy(RAM_.data(), &RAM_[read32(kLoadDataAddress)],
              read32(kDataSizeAddress));

  // 3. Initialize stack pointer register to the end of the stack (0x5200)
  CPU_.registers_[29] = kVRAMAddress;

  // 4. Call setup()
  setup();

  // Use high-resolution clock for accurate timing
  using namespace std::chrono;  // Limited scope for chrono
  high_resolution_clock::time_point start_time = high_resolution_clock::now();
  high_resolution_clock::time_point frame_start;
  int frame_count = 0;
  double elapsed_seconds = 0.0;

  // 5. Begin Game Loop Sequence
  while (CPU_.PC_ == 0x0000 && event_.type != SDL_QUIT) {
    frame_start = high_resolution_clock::now();

    controllerInput();
    if (!paused_) {
      // Run game loop once
      loop();
      GPU_.render();
    }
    // GPU Buffer Displayed
    GPU_.display();

    // Update elapsed seconds and frame count
    elapsed_seconds = duration_cast<duration<double>>(
                          high_resolution_clock::now() - start_time)
                          .count();
    ++frame_count;

    // Print FPS every second
    if (elapsed_seconds >= 1.0) {
      if (show_fps_) {
        std::cout << frame_count << " FPS\n";
      }
      frame_count = 0;
      start_time = high_resolution_clock::now();
    }

    // Limit FPS using sleep (more accurate than SDL_Delay)
    double remaining_time =
        frame_time_ - duration_cast<duration<double>>(
                          high_resolution_clock::now() - frame_start)
                          .count();
    if (remaining_time > 0.0) {
      std::this_thread::sleep_for(duration<double>(remaining_time));
    }
  }
}

void Console::setup() {
  CPU_.immediate_ = read32(kSetupAddress) / 4;
  CPU_.PC_ = 0xfffc;
  CPU_.JAL();
  while (CPU_.PC_ >= 0x8000) {
    CPU_.ExecuteInstruction(read32(CPU_.PC_));
  }  // Stops when PC wraps back to 0
  CPU_.registers_[29] = kVRAMAddress;
}

void Console::loop() {
  CPU_.immediate_ = read32(kLoopAddress) / 4;
  CPU_.PC_ = 0xfffc;
  CPU_.JAL();
  while (CPU_.PC_ >= 0x8000) {
    CPU_.ExecuteInstruction(read32(CPU_.PC_));
  }  // Stops when PC wraps back to 0
  CPU_.registers_[29] = kVRAMAddress;
}

// Save Functions
void Console::saveState(const std::string &filename) {
  std::ofstream out(filename, std::ios::binary);
  if (out.is_open()) {
    CPU_.saveState(out);
    GPU_.saveState(out);
    // Save RAM
    out.write(reinterpret_cast<char *>(&RAM_[0]),
              RAM_.size() * sizeof(RAM_[0]));
    // Save Console state variables
    out.write(reinterpret_cast<char *>(&show_fps_), sizeof(show_fps_));
    out.write(reinterpret_cast<char *>(&paused_), sizeof(paused_));
    out.write(reinterpret_cast<char *>(&target_fps_), sizeof(target_fps_));
    out.write(reinterpret_cast<char *>(&frame_time_), sizeof(frame_time_));
    out.close();
    std::cout << "State saved to " << filename << std::endl;
  } else {
    std::cerr << "Failed to save state to " << filename << std::endl;
  }
}

void Console::loadState(const std::string &filename) {
  std::ifstream in(filename, std::ios::binary);
  if (in.is_open()) {
    CPU_.loadState(in);
    GPU_.loadState(in);
    // Load RAM
    in.read(reinterpret_cast<char *>(&RAM_[0]), RAM_.size() * sizeof(RAM_[0]));
    // Load Console state variables
    in.read(reinterpret_cast<char *>(&show_fps_), sizeof(show_fps_));
    in.read(reinterpret_cast<char *>(&paused_), sizeof(paused_));
    in.read(reinterpret_cast<char *>(&target_fps_), sizeof(target_fps_));
    in.read(reinterpret_cast<char *>(&frame_time_), sizeof(frame_time_));
    in.close();
    std::cout << "State loaded from " << filename << std::endl;
  } else {
    std::cerr << "Failed to load state from " << filename << std::endl;
  }
}
std::string Console::getSaveStateFilename(int slot) const {
  std::ostringstream filename;
  filename << filename_.substr(0, filename_.find_last_of('.')) << "savestate"
           << slot << ".bin";
  return filename.str();
}

void Console::controllerInput() {
  while (SDL_PollEvent(&event_) && event_.type != SDL_QUIT) {
    // controller buttons
    switch (event_.type) {
      case SDL_KEYDOWN:  // ON KEY DOWN
        switch (event_.key.keysym.sym) {
          case SDLK_a:
            RAM_[kControllerDataAddress] |= CONTROLLER_A_MASK;
            break;
          case SDLK_b:
            RAM_[kControllerDataAddress] |= CONTROLLER_B_MASK;
            break;
          case SDLK_COMMA:  // SELECT
            RAM_[kControllerDataAddress] |= CONTROLLER_SELECT_MASK;
            break;
          case SDLK_PERIOD:  // START
            RAM_[kControllerDataAddress] |= CONTROLLER_START_MASK;
            break;
          case SDLK_UP:
            RAM_[kControllerDataAddress] |= CONTROLLER_UP_MASK;
            break;
          case SDLK_DOWN:
            RAM_[kControllerDataAddress] |= CONTROLLER_DOWN_MASK;
            break;
          case SDLK_LEFT:
            RAM_[kControllerDataAddress] |= CONTROLLER_LEFT_MASK;
            break;
          case SDLK_RIGHT:
            RAM_[kControllerDataAddress] |= CONTROLLER_RIGHT_MASK;
            break;
          default:
            break;
        }
        break;

      case SDL_KEYUP:  // ON KEY UP
        switch (event_.key.keysym.sym) {
          case SDLK_a:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_A_MASK;
            break;
          case SDLK_b:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_B_MASK;
            break;
          case SDLK_COMMA:  // SELECT
            RAM_[kControllerDataAddress] &= ~CONTROLLER_SELECT_MASK;
            break;
          case SDLK_PERIOD:  // START
            RAM_[kControllerDataAddress] &= ~CONTROLLER_START_MASK;
            break;
          case SDLK_UP:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_UP_MASK;
            break;
          case SDLK_DOWN:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_DOWN_MASK;
            break;
          case SDLK_LEFT:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_LEFT_MASK;
            break;
          case SDLK_RIGHT:
            RAM_[kControllerDataAddress] &= ~CONTROLLER_RIGHT_MASK;
            break;
        }
        break;
      default:
        break;
    }
    // filters/FPS keybinds
    switch (event_.type) {
      case SDL_KEYDOWN:  // ON KEY DOWN
        switch (event_.key.keysym.sym) {
          case SDLK_1:  // pressing F + 1 toggles CRT filter
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.toggleCRTFilter();
            }
            break;
          case SDLK_2:  // pressing F + 2 toggles Grayscale filter
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.toggleGrayScaleFilter();
            }
            break;
          case SDLK_3:  // pressing F + 3 toggles Invert filter
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.toggleInvertFilter();
            }
            break;
          case SDLK_4:  // pressing F + 4 changes hue filter speed
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.toggleHueFilter();
            }
            break;
          case SDLK_5:  // pressing F + 5 toggles Pastel filter
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.togglePastelFilter();
            }
            break;
          case SDLK_6:  // duck
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_F] == true) {
              GPU_.toggleDuckSkin();
            }
            break;
          case SDLK_x:  // custom image
            GPU_.toggleImage();
            break;
          case SDLK_EQUALS:  // + fps
            show_fps_ = true;
            if (target_fps_ + 5 <= 120) {
              target_fps_ += 5;
            }
            frame_time_ = 1.0 / target_fps_;
            break;
          case SDLK_MINUS:  // - fps
            show_fps_ = true;
            if (target_fps_ - 5 > 1) {
              target_fps_ -= 5;
            }
            frame_time_ = 1.0 / target_fps_;
            break;
          case SDLK_p:  // pause
            if (!paused_) {
              std::cout << "Game Paused!" << std::endl;
              paused_ = true;
            } else {
              std::cout << "Game Resumed!" << std::endl;
              paused_ = false;
            }
            break;
          default:
            break;
        }
        break;
    }
    // saves/load keybinds
    switch (event_.type) {
      case SDL_KEYDOWN:  // ON KEY DOWN
        switch (event_.key.keysym.sym) {
          case SDLK_s:
            saveState(getSaveStateFilename(0));
            break;
          case SDLK_l:
            loadState(getSaveStateFilename(0));
            break;
          case SDLK_0:
          case SDLK_1:
          case SDLK_2:
          case SDLK_3:
          case SDLK_4:
          case SDLK_5:
          case SDLK_6:
          case SDLK_7:
          case SDLK_8:
          case SDLK_9:
            if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_S] == true) {
              int save_slot = event_.key.keysym.sym - SDLK_0;
              saveState(getSaveStateFilename(save_slot));
            } else if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_L] == true) {
              int load_slot = event_.key.keysym.sym - SDLK_0;
              loadState(getSaveStateFilename(load_slot));
            }
            break;
          default:
            break;
        }
        break;
    }
  }
}

uint8_t Console::read8(uint16_t address) const {
  if (readable(address)) {
    return RAM_[address];
  }
  return 0;
}

uint16_t Console::read16(uint16_t address) const {
  uint16_t data = 0;
  for (int i = 0; i < 2; i++) {
    if (readable(address)) {
      data = (data << 8) | RAM_[address + i];
    } else {
      return 0;
    }
  }
  return data;
}

uint32_t Console::read32(uint16_t address) const {
  uint32_t data = 0;
  for (int i = 0; i < 4; i++) {
    if (readable(address)) {
      data = (data << 8) | RAM_[address + i];
    } else {
      return 0;
    }
  }
  return data;
}

void Console::write8(uint16_t address, uint8_t data) {
  if (writable(address, 1)) {
    RAM_[address] = data;
  }
}

void Console::write16(uint16_t address, uint16_t data) {
  if (writable(address, 2)) {
    for (int i = 0; i < 2; i++) {
      RAM_[address + i] = static_cast<uint8_t>(data >> 8 * (1 - i));
    }
  }
}

void Console::write32(uint16_t address, uint32_t data) {
  if (writable(address, 4)) {
    for (int i = 0; i < 4; i++) {
      RAM_[address + i] = static_cast<uint8_t>(data >> 8 * (3 - i));
    }
  }
}

bool Console::readable(uint16_t address) const {
  return ((address >= kRAMAddress && address <= kRAMAddress + kRAMSize) ||
          (address == kControllerDataAddress) ||
          (address >= kSLUGFileAddress &&
           address <= kSLUGFileAddress + kSLUGFileSize));
}

bool Console::writable(uint16_t address, int bytes) const {
  for (int i = 0; i < bytes; i++) {
    if ((address + i >= kRAMAddress && address + i <= kRAMAddress + kRAMSize) ||
        (address + i == kDebugstdoutAddress) ||
        (address + i == kDebugstderrAddress) ||
        (address + i == kStopExecutionAddress)) {
    } else {
      return false;
    }
  }
  return true;
}

void Console::PrintMenu() {
  std::cout << "Please select an option.\n";
  std::cout << std::string(24, '-') << std::endl;
  std::cout << "(1) Disassemble\n(2) Exit\n\n";
}
void Console::Disassemble() {
  // 1. Clear all of RAM with zeros
  std::fill(RAM_.begin(), RAM_.begin() + kRAMSize, 0);

  // 2. Copy data section to RAM
  std::memcpy(RAM_.data(), &RAM_[read32(kLoadDataAddress)],
              read32(kDataSizeAddress));

  // 3. Initialize stack pointer register to the end of the stack (0x5200)
  CPU_.registers_[29] = kVRAMAddress;

  std::cout << std::string(22, '*') << std::endl;
  std::cout << " Launching Disassembler\n";
  std::cout << std::string(22, '*') << std::endl << std::endl;
  int userInput;
  std::string startAddress;
  int intAddress;
  int numToDecode;
  while (true) {
    PrintMenu();
    std::cin >> userInput;
    if (userInput == 1) {
      while (true) {
        std::cout << "Choose a starting address between 0x8000 - 0x10000: ";
        std::cin >> startAddress;
        intAddress = std::stoi(startAddress, nullptr, 16);
        if ((intAddress % 4 != 0) ||
            (intAddress < 0x8000 || intAddress > 0x10000)) {
          std::cout << "Invalid start address\n";
        } else {
          break;
        }
      }

      while (true) {
        std::cout
            << "How many instructions do you want to decode you lazy bum? ";
        std::cin >> numToDecode;
        if (numToDecode > 0) {
          break;
        }
      }
      std::cout << std::endl;
      Decode(intAddress, numToDecode);
    } else if (userInput == 2) {
      std::cout << "exit\n";
      break;
    }
    std::cout << std::endl;
  }
}

void Console::Decode(int startAddress, int numToDecode) {
  std::cout << "Decoding rn fr fr no cap\n";
  uint16_t nextInstruction = static_cast<uint16_t>(startAddress);
  for (int i = 0; i < numToDecode; i++) {
    uint32_t instruction = read32(static_cast<uint16_t>(nextInstruction));
    CPU_.DecodeInstruction(instruction);
    CPU_.PrintInstruction();
    nextInstruction += 0x0004;
  }
}