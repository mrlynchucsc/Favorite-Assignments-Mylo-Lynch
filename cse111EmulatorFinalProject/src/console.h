// console.h
// Copyright (c) 2024 Ethan Sifferman.
// All rights reserved. Distribution Prohibited.

#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "cpu.h"
#include "gpu.h"

class Console {
 private:
  std::unique_ptr<char[]> contents_;  // Memory buffer for file contents

  std::string filename_;           // Name of the file
  size_t file_size_;               // Size of the file
  bool file_opened_successfully_;  // Flag to check if file opened successfully
  std::vector<uint8_t> RAM_;
  BananaCpu CPU_;
  BananaGpu GPU_;

  SDL_Event event_;
  bool show_fps_ = false;
  bool paused_ = false;
  int target_fps_ = 60;
  double frame_time_ = 1.0 / target_fps_;

  // Helper function to check file extension
  static bool hasExtension(const std::string &filename,
                           const std::string &extension);

  // Decompiler
  void PrintMenu();
  void Decode(int, int);

 public:
  // Constructors
  Console(const std::string &filename,
          const std::vector<std::string> &allowed_extensions = {".slug"});

  // Accessor methods
  std::string filename() const { return filename_; }
  size_t file_size() const { return file_size_; }
  bool isFileOpen() const { return file_opened_successfully_; }

  void reset();
  void setup();
  void loop();
  void Disassemble();

  void controllerInput();
  void settingsChanger();
  void savesChanger();
  bool readable(uint16_t addr) const;
  bool writable(uint16_t addr, int size) const;

  // Read
  uint8_t read8(uint16_t addr) const;
  uint16_t read16(uint16_t addr) const;
  uint32_t read32(uint16_t addr) const;

  // Write
  void write8(uint16_t addr, uint8_t data);
  void write16(uint16_t addr, uint16_t data);
  void write32(uint16_t addr, uint32_t data);

  // Save
  void saveState(const std::string &filename);
  void loadState(const std::string &filename);
  std::string getSaveStateFilename(int slot) const;

  enum AddressSpace {
    // RAM Address Space     // Permissions
    kRAMAddress = 0x0000,    // rw
    kStackAddress = 0x3200,  // rw
    kVRAMAddress = 0x5200,   // rw

    kRAMSize = 0x7000,
    kStackSize = 0x2000,
    kVRAMSize = 0x1e00,

    // IO Address Space               // Permissions
    kControllerDataAddress = 0x7000,  // r
    kDebugstdinAddress = 0x7100,      // r
    kDebugstdoutAddress = 0x7110,     // w
    kDebugstderrAddress = 0x7120,     // w
    kStopExecutionAddress = 0x7200,   // w

    // SLUG Address Space          // Permissions
    kSLUGFileAddress = 0x8000,     // rx
    kSetupAddress = 0x81e0,        // rx
    kLoopAddress = 0x81e4,         // rx
    kLoadDataAddress = 0x81e8,     // rx
    kProgramDataAddress = 0x81ec,  // rx
    kDataSizeAddress = 0x81f0,     // rx

    kSLUGFileSize = 0x8000,
  };
};