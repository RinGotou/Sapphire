#pragma once
#include "common.h"

namespace sapphire {
  using ComponentLoader = void(*)();

  void InitConsoleComponents();
  void InitBaseTypes();
  void InitContainerComponents();
  void InitFunctionType();
  void InitStreamComponents();
  void InitSoundComponents();
  void InitWindowComponents();
  void InitExtensionComponents();
  void InitExternalPointerComponents();
  void InitStructComponents();

  const ComponentLoader kEmbeddedComponents[] = {
    InitConsoleComponents,
    InitBaseTypes,
    InitContainerComponents,
    InitFunctionType,
    InitStreamComponents,
    InitExtensionComponents,
    InitExternalPointerComponents,
    InitStructComponents
  };
}
