#include "parser.h"

namespace sapphire {
  struct LexicalFirstStepState {
    bool inside_string: 1;
    bool leave_string: 1;
    bool enter_string: 1;
    bool esc_flag: 1;
    bool exempt_esc_flag: 1;
  };

  bool LexicalProcess_1stStage(IndexedString str, deque<IndexedToken> &output, 
    StandardLogger *logger) {
      LexicalFirstStepState state{false, false, false, false, false};
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

    //TODO: Insert linebreak after every processing
    //TODO: missing return!
    return false; //placeholder
  }

  struct LexicalSecondStageState {
    bool good: 1;
    bool sign_flag: 1;
  };

  bool LexicalProcess_2ndStage(deque<IndexedToken> &input, deque<IndexedToken> &output, 
    StandardLogger *logger) {
    LexicalSecondStageState state{true, false};
    stack<char> bracket_stack;
#define INVALID_TOKEN Token(string(), TokenType::Invalid)
    Token current = INVALID_TOKEN, next = INVALID_TOKEN, last = INVALID_TOKEN;
    size_t index = 0;
    //Token current_valid = INVALID_TOKEN;

    auto get_left_bracket = [](string_view src) -> char {
      if (src == ")") return '(';
      if (src == "]") return '[';
      if (src == "}") return '{';
      return '\0';
    };

    //TODO: Rewrite this part completely.
    for (size_t i = 0, size = input.size(); i < size; i += 1) {
      index = input[i].first;
      current = input[i].second;
      next = i < size - 1 ? input[i + 1].second : INVALID_TOKEN;

      //invalid token.
      if (current.second == TokenType::Invalid) {
        //error: (report unknown tokens)
        state.good = false;
        break;
      }

      //semicolon.
      if (current.first == ";") {
        if (!bracket_stack.empty()) {
          //error:mismatched brackets
          state.good = false;
          break;
        }

        if (compare(next.second, TokenType::LineBreak, TokenType::Invalid)) {
          //warning:unnecessary semicolon
        }

        last = current;
        continue;
      }

      //invalid token.
      if (current.second == TokenType::Invalid) {
        //error: unrecog token
        state.good = false;
        break;
      }

      //positive/negative sign
      if (compare(current.first, "+", "-") && !compare(last.first, ")", "]", "}")) {
        if (compare(last.second, TokenType::Symbol, TokenType::Invalid) 
              && compare(next.second, TokenType::Int, TokenType::Float)) {
          state.sign_flag = true;
          //TODO: do not push into output, store it in other location
          output.push_back(IndexedToken(index, current));
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
          state.good = false;
          break;
        }

        if (get_left_bracket(current.first) != bracket_stack.top()) {
          //error: Mismatched left bracket
          state.good = false;
          break;
        }

        //Matching succeed
        bracket_stack.pop();
      }
      
      if (state.sign_flag) {
        auto combined_str = last.first + current.first;
        
      }
    }
    //Just a placeholder; delete it later.
    return false;
  }

  bool TryParsing(string_view file, string_view log, AnnotatedAST &output) {

  }
}
