#pragma once
#include "trace.h"
#include "filestream.h"

namespace sapphire {
  using IndexedToken = pair<size_t, Token>;
  using IndexedString = pair<size_t, string>;

  enum class ParsingState { Fine, Warning, Error };

  //TODO: Using bool for error processing in future?
  void LexicalProcess_1stStage(IndexedString str, deque<IndexedToken> &output, 
    StandardLogger *logger);
  ParsingState LexicalProcess_2ndStage(deque<IndexedToken> &input, deque<IndexedToken> &output, 
    string &msg);
  bool TryParsing(string_view file, string_view log, AnnotatedAST &output);
}
