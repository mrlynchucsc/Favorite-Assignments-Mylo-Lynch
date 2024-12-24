#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "duck.h"

class Console;

class BananaGpu {
 protected:
 private:
  Console& console_;
  std::vector<uint8_t>& RAM_;

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  SDL_Texture* image_ = NULL;

  void renderPixel(int x, int y, uint32_t color) const;

 public:
  // Constructor / Destructor
  BananaGpu(Console& OS, std::vector<uint8_t>& RAM);
  ~BananaGpu();

  // State Variables
  int current_line_;
  int current_column_;
  bool img_enabled_ = false;
  bool duck_enabled_ = false;
  bool crt_filter_enabled_ = false;
  bool grayscale_enabled_ = false;
  bool invert_enabled_ = false;
  bool pastel_enabled_ = false;

  int hue_speed_ = 0;
  int hue_rotation_ = 0;

  void toggleImage() { img_enabled_ = !img_enabled_; }
  void drawImage();
  void toggleDuckSkin() { duck_enabled_ = !duck_enabled_; }

  // CRT Filter Functions
  void toggleCRTFilter() { crt_filter_enabled_ = !crt_filter_enabled_; }
  void applyCRTFilter(uint8_t& r, uint8_t& g, uint8_t& b, int currentLine);
  // GrayScale Filter Functions
  void toggleGrayScaleFilter() { grayscale_enabled_ = !grayscale_enabled_; }
  void applyGrayScaleFilter(uint8_t& r, uint8_t& g, uint8_t& b);
  // Invert Filter Functions
  void toggleInvertFilter() { invert_enabled_ = !invert_enabled_; }
  void applyInvertFilter(uint8_t& r, uint8_t& g, uint8_t& b);
  // Pastel Filter Functions
  void togglePastelFilter() { pastel_enabled_ = !pastel_enabled_; }
  void applyPastelFilter(uint8_t& r, uint8_t& g, uint8_t& b);
  // Hue Filter Functions
  void toggleHueFilter() {
    hue_speed_ = ++hue_speed_ % 5;
    std::cout << "Hue Shift Speed: " << hue_speed_ << std::endl;
  }
  void applyHueFilter(uint8_t& r, uint8_t& g, uint8_t& b);

  // Pixel Data Handling Functions
  int getPixelAddress(int width, int height) const;
  uint32_t decodePixel(uint16_t pixelData);

  void render();
  void display();
  void loadImage();

  // Enumeration to define display parameters
  enum Display {
    kDisplayWidth = 64,
    kDisplayHeight = 60,
    kScaleFactor = 10,
  };

  // Enumeration to define RGB bit masks
  enum ColorMasks {
    kRedMask = 0x7C00,
    kGreenMask = 0x03E0,
    kBlueMask = 0x001F,
  };

  // Constants for the CRT Filter Effect
  static constexpr double kDarkenFactor = 0.9;
  static constexpr int kNoiseRange = 11;
  static constexpr int kNoiseOffset = 5;

  void saveState(std::ofstream& out);
  void loadState(std::ifstream& in);
};