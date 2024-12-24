#pragma once
#include <iostream>

inline void applyDuckSkin(uint8_t& r, uint8_t& g, uint8_t& b, int& currentLine,
                          int& currentColumn) {
  // // Apply duck skin colors based on line and pixel location
  if (currentLine >= 12 && currentLine <= 55) {
    if (currentColumn >= 26 && currentColumn <= 32 && currentLine == 12) {
      r = g = b = 0;
    } else if (currentLine == 13 && currentColumn >= 24 &&
               currentColumn <= 34) {
      if (currentColumn >= 24 && currentColumn <= 27 ||
          currentColumn >= 32 && currentColumn <= 34) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 23 && currentColumn <= 35 &&
               currentLine == 14) {
      if (currentColumn >= 23 && currentColumn <= 24 ||
          currentColumn >= 34 && currentColumn <= 35) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 22 && currentColumn <= 36 &&
               currentLine == 15) {
      if (currentColumn >= 22 && currentColumn <= 23 ||
          currentColumn >= 35 && currentColumn <= 36) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 22 && currentColumn <= 37 &&
               currentLine == 16) {
      if (currentColumn == 22 || currentColumn >= 36 && currentColumn <= 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 21 && currentColumn <= 37 &&
               currentLine == 17) {
      if (currentColumn >= 21 && currentColumn <= 22 || currentColumn == 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 20 && currentColumn <= 37 &&
               currentLine == 18) {
      if (currentColumn == 20 || currentColumn == 37 ||
          currentColumn >= 32 && currentColumn <= 35) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 19 && currentColumn <= 37 &&
               currentLine == 19) {
      if (currentColumn >= 19 && currentColumn <= 20 || currentColumn == 37 ||
          currentColumn == 31 || currentColumn >= 23 && currentColumn <= 26 ||
          currentColumn >= 33 && currentColumn <= 35) {
        r = g = b = 0;
      } else if (currentColumn == 32) {
        r = g = b = 255;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 19 && currentColumn <= 37 &&
               currentLine == 20) {
      if (currentColumn == 19 || currentColumn == 23 || currentColumn == 37 ||
          currentColumn >= 25 && currentColumn <= 27 ||
          currentColumn >= 33 && currentColumn <= 35 || currentColumn == 31) {
        r = g = b = 0;
      } else if (currentColumn == 24 || currentColumn == 32) {
        r = g = b = 255;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 38 &&
               currentLine == 21) {
      if (currentColumn >= 18 && currentColumn <= 19 || currentColumn == 22 ||
          currentColumn >= 25 && currentColumn <= 27 ||
          currentColumn >= 31 && currentColumn <= 35 ||
          currentColumn >= 37 && currentColumn <= 38) {
        r = g = b = 0;
      } else if (currentColumn >= 23 && currentColumn <= 24) {
        r = g = b = 255;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 38 &&
               currentLine == 22) {
      if (currentColumn == 18 || currentColumn == 38 ||
          currentColumn >= 22 && currentColumn <= 23 ||
          currentColumn >= 25 && currentColumn <= 26 ||
          currentColumn >= 32 && currentColumn <= 35) {
        r = g = b = 0;
      } else if (currentColumn == 24) {
        r = g = b = 255;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 41 &&
               currentLine == 23) {
      if (currentColumn == 18 || currentColumn >= 23 && currentColumn <= 26 ||
          currentColumn >= 37 && currentColumn <= 41) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 47 &&
               currentLine == 24) {
      if (currentColumn == 18 || currentColumn >= 35 && currentColumn <= 36 ||
          currentColumn >= 42 && currentColumn <= 47) {
        r = g = b = 0;
      } else if (currentColumn >= 37 && currentColumn <= 41) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 49 &&
               currentLine == 25) {
      if (currentColumn == 18 || currentColumn >= 34 && currentColumn <= 35 ||
          currentColumn >= 48 && currentColumn <= 49) {
        r = g = b = 0;
      } else if (currentColumn >= 36 && currentColumn <= 47) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 50 &&
               currentLine == 26) {
      if (currentColumn == 18 || currentColumn >= 32 && currentColumn <= 34 ||
          currentColumn == 38 || currentColumn == 50) {
        r = g = b = 0;
      } else if (currentColumn >= 35 && currentColumn <= 37 ||
                 currentColumn >= 39 && currentColumn <= 49) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 19 && currentColumn <= 51 &&
               currentLine == 27) {
      if (currentColumn == 19 || currentColumn >= 50 && currentColumn <= 51 ||
          currentColumn == 32 || currentColumn == 37) {
        r = g = b = 0;
      } else if (currentColumn >= 33 && currentColumn <= 36 ||
                 currentColumn >= 38 && currentColumn <= 49) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 19 && currentColumn <= 51 &&
               currentLine >= 28 && currentLine <= 29) {
      if (currentColumn == 19 || currentColumn == 32 || currentColumn == 51) {
        r = g = b = 0;
      } else if (currentColumn >= 33 && currentColumn <= 50) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 20 && currentColumn <= 51 &&
               currentLine == 30) {
      if (currentColumn == 20 || currentColumn == 32 || currentColumn == 51) {
        r = g = b = 0;
      } else if (currentColumn >= 33 && currentColumn <= 50) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 20 && currentColumn <= 51 &&
               currentLine == 31) {
      if (currentColumn >= 20 && currentColumn <= 21 || currentColumn == 32 ||
          currentColumn == 51) {
        r = g = b = 0;
      } else if (currentColumn >= 33 && currentColumn <= 50) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 20 && currentColumn <= 51 &&
               currentLine == 32) {
      if (currentColumn >= 20 && currentColumn <= 21 ||
          currentColumn >= 32 && currentColumn <= 35 ||
          currentColumn >= 48 && currentColumn <= 51) {
        r = g = b = 0;
      } else if (currentColumn >= 36 && currentColumn <= 50) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 48 &&
               currentLine == 33) {
      if (currentColumn >= 18 && currentColumn <= 19 ||
          currentColumn >= 36 && currentColumn <= 37 ||
          currentColumn >= 42 && currentColumn <= 48) {
        r = g = b = 0;
      } else if (currentColumn >= 38 && currentColumn <= 41) {
        r = 255;
        g = 128;
        b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 42 &&
               currentLine == 34) {
      if (currentColumn >= 17 && currentColumn <= 18 ||
          currentColumn >= 37 && currentColumn <= 42) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 37 &&
               currentLine == 35) {
      if (currentColumn == 17 || currentColumn >= 36 && currentColumn <= 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 36 &&
               currentLine == 36) {
      if (currentColumn == 17 || currentColumn == 36) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 37 &&
               currentLine == 37) {
      if (currentColumn >= 16 && currentColumn <= 17 ||
          currentColumn >= 36 && currentColumn <= 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 38 &&
               currentLine == 38) {
      if (currentColumn == 16 || currentColumn >= 35 && currentColumn <= 38) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 38 &&
               currentLine == 39) {
      if (currentColumn == 16 || currentColumn == 38 ||
          currentColumn >= 35 && currentColumn <= 36) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 38 &&
               currentLine >= 40 && currentLine <= 42) {
      if (currentColumn == 16 || currentColumn == 22 || currentColumn == 35 ||
          currentColumn == 38) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 38 &&
               currentLine == 43) {
      if (currentColumn == 16 || currentColumn == 22 || currentColumn == 35 ||
          currentColumn >= 37 && currentColumn <= 38) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 16 && currentColumn <= 37 &&
               currentLine == 44) {
      if (currentColumn >= 16 && currentColumn <= 17 || currentColumn == 22 ||
          currentColumn == 35 || currentColumn == 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 37 &&
               currentLine == 45) {
      if (currentColumn >= 21 && currentColumn <= 22 || currentColumn == 17 ||
          currentColumn >= 35 && currentColumn <= 37) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 35 &&
               currentLine == 46) {
      if (currentColumn == 17 || currentColumn == 21 || currentColumn == 35) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 17 && currentColumn <= 34 &&
               currentLine == 47) {
      if (currentColumn >= 17 && currentColumn <= 18 ||
          currentColumn >= 21 && currentColumn <= 24 ||
          currentColumn >= 33 && currentColumn <= 34) {
        r = g = b = 0;
      } else {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 18 && currentColumn <= 32 &&
               currentLine == 48) {
      if (currentColumn >= 18 && currentColumn <= 19 || currentColumn == 21 ||
          currentColumn >= 24 && currentColumn <= 32) {
        r = g = b = 0;
      } else if (currentColumn == 20) {
        r = g = 255;
        b = 0;
      }
    } else if (currentColumn >= 20 && currentColumn <= 31 &&
               currentLine == 49) {
      if (currentColumn >= 20 && currentColumn <= 21 || currentColumn == 26 ||
          currentColumn == 31) {
        r = g = b = 0;
      }
    } else if (currentColumn >= 25 && currentColumn <= 31 &&
               currentLine == 50) {
      if (currentColumn >= 25 && currentColumn <= 26 || currentColumn == 31) {
        r = g = b = 0;
      }
    } else if (currentColumn >= 25 && currentColumn <= 31 &&
               currentLine >= 51 && currentLine <= 52) {
      if (currentColumn >= 20 && currentColumn <= 21 || currentColumn == 26 ||
          currentColumn == 31) {
        r = g = b = 0;
      }
    } else if (currentColumn >= 25 && currentColumn <= 32 &&
               currentLine == 53) {
      if (currentColumn >= 26 && currentColumn <= 27 ||
          currentColumn >= 31 && currentColumn <= 32) {
        r = g = b = 0;
      }
    }
  }
}