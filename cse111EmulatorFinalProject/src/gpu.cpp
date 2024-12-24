#include "gpu.h"

#include "console.h"

BananaGpu::BananaGpu(Console& console, std::vector<uint8_t>& RAM)
    : console_(console), RAM_(RAM) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    exit(-1);
  }

  // Create a window
  window_ =
      SDL_CreateWindow("Banana", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, kDisplayWidth * kScaleFactor,
                       kDisplayHeight * kScaleFactor, SDL_WINDOW_SHOWN);
  if (!window_) {
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    SDL_Quit();
    exit(-1);
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);

  // Scale the window to be bigger
  SDL_RenderSetLogicalSize(renderer_, kDisplayWidth, kDisplayHeight);

  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB565,
                               SDL_TEXTUREACCESS_STREAMING, kDisplayWidth,
                               kDisplayHeight);

  // Image setup
  SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);
  loadImage();
}

BananaGpu::~BananaGpu() {
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_DestroyTexture(texture_);
  SDL_DestroyTexture(image_);
  SDL_Quit();
}

int BananaGpu::getPixelAddress(int width, int height) const {
  int pixelIndex = width + (height * kDisplayWidth);
  int pixelOffset = 2 * pixelIndex;
  return 0x5200 + pixelOffset;
}
struct RGB {
  double r, g, b;
};

struct HSV {
  double h, s, v;
};

HSV rgb2hsv(RGB in) {
  HSV out;
  double min, max, delta;

  // Normalize values between 0 and 1
  in.r /= 255.0;
  in.g /= 255.0;
  in.b /= 255.0;

  // Find min and max values
  min = std::min({in.r, in.g, in.b});
  max = std::max({in.r, in.g, in.b});

  out.v = max;
  delta = max - min;

  // Handle gray colors (undefined hue)
  if (delta < 1e-6) {
    out.s = 0;
    out.h = 0;
    return out;
  }

  out.s = delta / max;

  // Calculate hue
  double temp;
  if (in.r == max) {
    temp = (in.g - in.b) / delta;
  } else if (in.g == max) {
    temp = 2 + (in.b - in.r) / delta;
  } else {
    temp = 4 + (in.r - in.g) / delta;
  }

  out.h = std::fmod(temp * 60, 360);  // Wrap hue to 0-360 degrees

  return out;
}

RGB hsv2rgb(HSV in) {
  RGB out;
  double c = in.v * in.s;
  double x = c * (1 - std::abs(fmod(in.h / 60.0, 2) - 1));
  double m = in.v - c;

  // Map colors based on hue sector
  if (in.h >= 0 && in.h < 60) {
    out.r = c + m;
    out.g = x + m;
    out.b = m;
  } else if (in.h >= 60 && in.h < 120) {
    out.r = x + m;
    out.g = c + m;
    out.b = m;
  } else if (in.h >= 120 && in.h < 180) {
    out.r = m;
    out.g = c + m;
    out.b = x + m;
  } else if (in.h >= 180 && in.h < 240) {
    out.r = m;
    out.g = x + m;
    out.b = c + m;
  } else if (in.h >= 240 && in.h < 300) {
    out.r = x + m;
    out.g = m;
    out.b = c + m;
  } else {
    out.r = c + m;
    out.g = m;
    out.b = x + m;
  }

  // Denormalize values back to 0-255 range
  out.r = out.r * 255;
  out.g = out.g * 255;
  out.b = out.b * 255;

  return out;
}

HSV rotate_hue(HSV in, double degrees) {
  in.h = fmod(in.h + degrees, 360.0);  // Wrap hue to 0-360 degrees
  return in;
}

void BananaGpu::applyCRTFilter(uint8_t& r, uint8_t& g, uint8_t& b,
                               int currentLine) {
  // Darken the pixels in every other line
  if (currentLine % 2 == 0) {
    r = r * kDarkenFactor;
    g = g * kDarkenFactor;
    b = b * kDarkenFactor;
  }
  // Add random noise
  int noise = (rand() % kNoiseRange) - kNoiseOffset;

  r = std::clamp(r + noise, 0, 255);
  g = std::clamp(g + noise, 0, 255);
  b = std::clamp(b + noise, 0, 255);
}

void BananaGpu::applyGrayScaleFilter(uint8_t& r, uint8_t& g, uint8_t& b) {
  float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
  r = luminance;
  g = luminance;
  b = luminance;
}

void BananaGpu::applyInvertFilter(uint8_t& r, uint8_t& g, uint8_t& b) {
  r = 255 - r;
  g = 255 - g;
  b = 255 - b;
}

void BananaGpu::applyPastelFilter(uint8_t& r, uint8_t& g, uint8_t& b) {
  RGB rgb;
  HSV hsv;
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  hsv = rgb2hsv(rgb);
  hsv.s *= 0.6;
  rgb = hsv2rgb(hsv);
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
}

void BananaGpu::applyHueFilter(uint8_t& r, uint8_t& g, uint8_t& b) {
  RGB rgb;
  rgb.r = r;
  rgb.g = g;
  rgb.b = b;
  rgb = hsv2rgb(rotate_hue(rgb2hsv(rgb), hue_rotation_));
  r = rgb.r;
  g = rgb.g;
  b = rgb.b;
}

uint32_t BananaGpu::decodePixel(uint16_t pixelData) {
  uint8_t r = (pixelData & kRedMask) >> 10;   // Extract bits 14-10
  uint8_t g = (pixelData & kGreenMask) >> 5;  // Extract bits 9-5
  uint8_t b = (pixelData & kBlueMask);        // Extract bits 4-0

  // Change from 5-bit value to 8 bit value
  r = (r * 255) / 31;
  g = (g * 255) / 31;
  b = (b * 255) / 31;

  // if (img_enabled_) {
  //   drawImage();
  // }
  if (duck_enabled_) {
    applyDuckSkin(r, g, b, current_line_, current_column_);
  }
  if (grayscale_enabled_) {
    applyGrayScaleFilter(r, g, b);
  }
  if (invert_enabled_) {
    applyInvertFilter(r, g, b);
  }
  if (pastel_enabled_) {
    applyPastelFilter(r, g, b);
  }
  if (hue_speed_ > 0) {
    applyHueFilter(r, g, b);
  }
  if (crt_filter_enabled_) {
    applyCRTFilter(r, g, b, current_line_);
  }
  return (r << 16) | (g << 8) | b;
}

// 6.3 Rendering
void BananaGpu::renderPixel(int x, int y, uint32_t color) const {
  SDL_SetRenderDrawColor(renderer_, (color >> 16) & 0xFF, (color >> 8) & 0xFF,
                         color & 0xFF, 0xFF);
  SDL_RenderDrawPoint(renderer_, x, y);
}

void BananaGpu::render() {
  SDL_RenderClear(renderer_);

  // Loop through each row (Height)
  for (int y = 0; y < kDisplayHeight; ++y) {
    current_line_ = y;
    // Loop through each column (Width)
    for (int x = 0; x < kDisplayWidth; ++x) {
      uint16_t address = getPixelAddress(x, y);
      uint16_t pixelData = console_.read16(address);
      uint32_t color = decodePixel(pixelData);
      current_column_ = x;
      renderPixel(x, y, color);
    }
  }

  hue_rotation_ += hue_speed_;
  if (img_enabled_) {
    drawImage();
  }
}

void BananaGpu::drawImage() {
  // If the image is enabled and it's loaded
  if (img_enabled_ && image_ != nullptr) {
    SDL_RenderCopy(renderer_, image_, NULL, NULL);
    // Draw the image to the renderer
  }
  // Update the display with the changes
  SDL_RenderPresent(renderer_);
}

void BananaGpu::display() { SDL_RenderPresent(renderer_); }

void BananaGpu::saveState(std::ofstream& out) {
  out.write(reinterpret_cast<char*>(&duck_enabled_), sizeof(duck_enabled_));
  out.write(reinterpret_cast<char*>(&crt_filter_enabled_),
            sizeof(crt_filter_enabled_));
  out.write(reinterpret_cast<char*>(&grayscale_enabled_),
            sizeof(grayscale_enabled_));
  out.write(reinterpret_cast<char*>(&invert_enabled_), sizeof(invert_enabled_));
  out.write(reinterpret_cast<char*>(&pastel_enabled_), sizeof(pastel_enabled_));
  out.write(reinterpret_cast<char*>(&hue_speed_), sizeof(hue_speed_));
  out.write(reinterpret_cast<char*>(&hue_rotation_), sizeof(current_column_));
  out.write(reinterpret_cast<char*>(&current_line_), sizeof(current_line_));
  out.write(reinterpret_cast<char*>(&current_column_), sizeof(current_column_));
  // Save other necessary state variables...
}

void BananaGpu::loadState(std::ifstream& in) {
  in.read(reinterpret_cast<char*>(&duck_enabled_), sizeof(duck_enabled_));
  in.read(reinterpret_cast<char*>(&crt_filter_enabled_),
          sizeof(crt_filter_enabled_));
  in.read(reinterpret_cast<char*>(&grayscale_enabled_),
          sizeof(grayscale_enabled_));
  in.read(reinterpret_cast<char*>(&invert_enabled_), sizeof(invert_enabled_));
  in.read(reinterpret_cast<char*>(&pastel_enabled_), sizeof(pastel_enabled_));
  in.read(reinterpret_cast<char*>(&hue_speed_), sizeof(hue_speed_));
  in.read(reinterpret_cast<char*>(&hue_rotation_), sizeof(current_column_));
  in.read(reinterpret_cast<char*>(&current_line_), sizeof(current_line_));
  in.read(reinterpret_cast<char*>(&current_column_), sizeof(current_column_));
  // Load other necessary state variables...
}

void BananaGpu::loadImage() {
  image_ = IMG_LoadTexture(renderer_, "../src/images/ethan.png");
  if (image_ == nullptr) {
    std::cerr << "Failed to load image: " << SDL_GetError() << std::endl;
  }
}