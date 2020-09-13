#include "argument2.h"

namespace sapphire::commandline {
  void ArgumentProcessor::GeneratePlainData(int argc, char **argv) {
    std::string temp;

    binary_ = std::string(argv[0]);

    // note: blank characters are not allowed for multi-value argument
    for (int idx = 1; idx < argc; idx += 1) {

    }
  }
}