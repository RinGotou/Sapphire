#pragma once
#include "common.h"

namespace sapphire {
  using ComponentLoader = void(*)();

  void InitConsoleComponents();
  void InitStringTypes();
  void InitStringComponents();
  void InitContainerComponents();
  void InitFunctionType();
  void InitStreamComponents();
  void InitExtensionComponents();
  void InitStructComponents();

  const ComponentLoader kEmbeddedComponents[] = {
    InitConsoleComponents,
    InitStringTypes,
    InitStringComponents,
    InitContainerComponents,
    InitFunctionType,
    InitStreamComponents,
    InitExtensionComponents,
    InitStructComponents
  };
}
