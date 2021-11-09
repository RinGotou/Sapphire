#pragma once
#include "annotatedAST.h"
 
namespace sapphire {
  enum class StateLevel { Normal, Error, Warning };

  void AppendMessage(string msg, StateLevel level, 
    StandardLogger *std_logger, size_t index = 0);
  void AppendMessage(string msg, StandardLogger *std_logger);
}

