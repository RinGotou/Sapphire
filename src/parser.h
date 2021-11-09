#pragma once
#include "trace.h"
#include "filestream.h"

namespace sapphire {
  using IndexedToken = pair<size_t, Token>;
  using IndexedString = pair<size_t, string>;

  bool LexicalProcess_1stStage(IndexedString str, deque<IndexedToken> &output, 
    StandardLogger *logger);
  bool LexicalProcess_2ndStage(deque<IndexedToken> &input, deque<IndexedToken> &output, 
    StandardLogger *logger);
  bool TryParsing(string_view file, string_view log, AnnotatedAST &output);
}