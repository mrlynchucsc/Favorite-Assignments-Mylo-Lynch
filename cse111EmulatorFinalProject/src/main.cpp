#include "console.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path/to/.slug_file> " << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string romfile = argv[1];

  Console console(romfile);

  console.reset();

  return EXIT_SUCCESS;
}
