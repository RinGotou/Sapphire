#pragma once
#include "common.h"

namespace sapphire {
  using ComponentLoader = void(*)();

  void InitBaseUtils();
  void InitConsoleComponents();
  void InitStringTypes();
  void InitStringComponents();
  void InitContainerComponents();
  void InitFunctionType();
  void InitStreamComponents();
  //void InitExtensionComponents();
  void InitStructComponents();

  const ComponentLoader kEmbeddedComponents[] = {
    InitBaseUtils,
    InitConsoleComponents,
    InitStringTypes,
    InitStringComponents,
    InitContainerComponents,
    InitFunctionType,
    InitStreamComponents,
    //InitExtensionComponents,
    InitStructComponents
  };
}
