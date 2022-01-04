#include "parser.h"
#include "lexical.h"
#include "log.h"

namespace sapphire {
  struct LexicalFirstStepState {
    bool inside_string: 1;
    bool leave_string: 1;
    bool enter_string: 1;
    bool esc_flag: 1;
    bool exempt_esc_flag: 1;
  };

  void LexicalProcess_1stStage(IndexedString str, deque<IndexedToken> &output, 
    StandardLogger *logger) {
      LexicalFirstStepState state{ false, false, false, false, false };
      char current = 0, next = 0, last = 0;
      string current_valid, buf;

    for (size_t i = 0, size = str.second.size(); i < size; i += 1) {
      current = str.second[i];
      next = (i < size - 1) ? str.second[i + 1] : 0;

      if (state.leave_string) {
        state.leave_string = false;
        state.inside_string = false;
      }

      state.esc_flag = (state.inside_string && last == '\\' 
        && !state.exempt_esc_flag);
      
      if (state.exempt_esc_flag) state.exempt_esc_flag = false;

      if (current == '\'' && !state.esc_flag) {
        if (!state.inside_string &&
          lexical::GetStringType(current_valid) == TokenType::Whitespace) {
            current_valid.clear();
        }        
      }

      if (!state.inside_string || state.enter_string) {
        (buf = current_valid).append(1, current);

        auto type = lexical::GetStringType(buf);
        auto current_type = lexical::GetStringType(current_valid);

        if (type != TokenType::Invalid) {
          //If we meet digit after +/-, spilt them as seperate first-step token.
          if (type == TokenType::Int && compare(buf, "+", "-")) {
            output.emplace_back(IndexedToken(str.first, 
              Token(string().append(1, buf[0]),TokenType::Symbol)));
            current_valid = buf.substr(1, buf.size() - 1);
          }
          else {
            current_valid.swap(buf);
          }
        }
        else {
          switch (current_type) {
          case TokenType::Whitespace:
            //cut the whitespace characters in front of the token.
            current_valid.clear();
            current_valid.append(1, current);
            break;
          case TokenType::Int:
            //seems like a decimal?
            //TODO: change detection method?
            if (current == '.' && lexical::IsDigit(next)) {
              current_valid.append(1, current);
            }
            else {
              //spilt tokens.
              output.emplace_back(IndexedToken(str.first, Token(current_valid, lexical::GetStringType(current_valid))));
              //output.emplace_back(Token(current_valid, lexical::GetStringType(current_valid)));
              current_valid.clear();
              current_valid.append(1, current);
            }
            break;
          default:
              //spilt tokens. same as above.
              output.emplace_back(IndexedToken(str.first, Token(current_valid, lexical::GetStringType(current_valid))));
              current_valid.clear();
              current_valid.append(1, current);
            break;
          }
        }

        if (state.enter_string) state.enter_string = false;
      }
      else {
        if (state.esc_flag) {
          current = lexical::GetEscapeChar(current);
        }

        state.exempt_esc_flag = (current == '\\' && last == '\\');
        current_valid.append(1, current);
      }

      last = str.second[i];
    }

    //final cleanup.
    if (lexical::GetStringType(current_valid) != TokenType::Whitespace) {
      output.emplace_back(IndexedToken(str.first, Token(current_valid, lexical::GetStringType(current_valid))));
    }

    // TODO: Insert linebreak placeholder after every processing  
  }

  ParsingState LexicalProcess_2ndStage(deque<IndexedToken> &input, deque<IndexedToken> &output, 
    string &msg) {
    ParsingState state = ParsingState::Fine;
    bool sign_flag = false;
    stack<char> bracket_stack;
#define INVALID_TOKEN Token(string(), TokenType::Invalid)
    Token current = INVALID_TOKEN, next = INVALID_TOKEN, last = INVALID_TOKEN;
    size_t index = 0;

    auto get_left_bracket = [](string_view src) -> char {
      if (src == ")") return '(';
      if (src == "]") return '[';
      if (src == "}") return '{';
      return '\0';
    };

    for (size_t i = 0, size = input.size(); i < size; i += 1) {
      index = input[i].first;
      current = input[i].second;
      next = i < size - 1 ? input[i + 1].second : INVALID_TOKEN;

      //invalid token.
      if (current.second == TokenType::Invalid) {
        msg = "Invalid token '" + current.first + "'";
        state = ParsingState::Error;
        break;
      }

      //semicolon.
      if (current.first == ";") {
        if (!bracket_stack.empty()) {
          msg = "Mismatched bracket";
          state = ParsingState::Error;
          break;
        }

        if (compare(next.second, TokenType::LineBreak, TokenType::Invalid)) {
          msg = "Unnecessary semicolon inside codeline";
          state = ParsingState::Warning;
        }

        last = current;
        continue;
      }

      //invalid token.
      if (current.second == TokenType::Invalid) {
        msg = "unrecognized token '" + current.first + "'";
        state = ParsingState::Error;
        break;
      }

      //positive/negative sign
      if (compare(current.first, "+", "-") && !compare(last.first, ")", "]", "}")) {
        if (compare(last.second, TokenType::Symbol, TokenType::Invalid) 
              && compare(next.second, TokenType::Int, TokenType::Float)) {
          sign_flag = true;
          //TODO: do not push into output, store it in other location
          //output.push_back(IndexedToken(index, current));
          last = current;
          continue;
        } 
      }

      //bracket (left)
      if (compare(current.first, "(", "[", "{")) {
        bracket_stack.push(current.first[0]);
      }

      //bracket (right)
      if (compare(current.first, ")", "]", "}")) {
        if (bracket_stack.empty()) {
          //error: No matching left bracket for current token
          state = ParsingState::Error;
          break;
        }

        if (get_left_bracket(current.first) != bracket_stack.top()) {
          //error: Mismatched left bracket
          state = ParsingState::Error;
          break;
        }

        //Matching succeed
        bracket_stack.pop();
      }

      if (current.first == ",") {
        if (last.second == TokenType::Symbol
          && !compare(last.first, "]", ")", "}", "'")) {
          //error: invalid comma
          state = ParsingState::Error;
          break;
        }
      }
      
      if (sign_flag) {
        auto combined_str = last.first + current.first;
        output.emplace_back(IndexedToken(index, Token(combined_str, current.second)));
        sign_flag = false;
        last = Token(combined_str, current.second);
        continue;
      }

      //append directly into output
      {
        output.emplace_back(IndexedToken(index, current));
        last = current;
      }
    }
    
    return state;
  }

  void StringTrimming(string &target) {
    if (target.empty()) return;

    char current = 0;
    size_t head = 0, tail = target.size() - 1;
    bool inside_string = false;

    auto is_whitespace = [](char src) -> bool {
      return src == ' ' || src == '\t' || src == '\r';
    };
    
    for (size_t i = 0, size = target.size(); i < size; i += 1) {
      current = target[i];

      if (!is_whitespace(current)) {
        head = i;
        break;
      }
    }

    if (target.at(head) == '#' || head == target.size() - 1) {
      target.clear();
      return;
    }

    for (size_t i = target.size() - 1; i <= 0 && i < target.size(); i -= 1) {
      current = target[i];
      
      if (!is_whitespace(current)) {
        tail = i;
        break;
      }
    }

    if (tail == 0) {
      return;
    }

    target = target.substr(head, tail - head);

    for (size_t i = 0, size = target.size(); i < size; i += 1) {
      current = target[i];
      if (current == '\'') {
        inside_string = !inside_string;
      }

      if (!inside_string && current == '#') {
        tail = i;
        break;
      }
    }

    target = target.substr(0, tail - 1);
  }

  bool TryParsing(string_view file, string_view log, AnnotatedAST &output) {
    // TODO: how to feed original script to two functions?
    // TODO: Add string_view type initializer into InStream and OutStream class
    size_t idx = 1; //tracking the indices of script lines. 
    string buf;
    bool inside_comment_block = false;
    InStream in_stream(file.data());


    if (!in_stream.Good()) return false;

    //read script file
    while (!in_stream.eof()) {
      buf = in_stream.GetLine();

      if (buf == kStrCommentBlock) {
        inside_comment_block  = !inside_comment_block;
        idx += 1;
        continue;
      }

      if (inside_comment_block) {
        //skip it.
        idx += 1;
        continue;
      }
    }
  }
}
