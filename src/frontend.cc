#include "frontend.h"

#define ERROR_MSG(_Msg) Message(_Msg, StateLevel::Error)

// !!! This module is deprecated and will be destroyed in the future.

namespace sapphire {
  inline bool IsBranchKeyword(Operation operation) {
    return operation == Operation::Elif || operation == Operation::Else || operation == Operation::When;
  }

  inline bool IsReservedKeyword(Operation operation) {
    return operation == Operation::If ||
      operation == Operation::Elif ||
      operation == Operation::While ||
      operation == Operation::When ||
      operation == Operation::Struct ||
      operation == Operation::Module ||
      operation == Operation::Include ||
      operation == Operation::Using ||
      operation == Operation::Case;
  }

  inline bool IsVariableExpression(Operation operation) {
    return operation == Operation::Return || operation == Operation::Attribute;
  }

  inline bool IsNestRoot(Operation operation) {
    return operation == Operation::If ||
      operation == Operation::While ||
      operation == Operation::Fn ||
      operation == Operation::Case ||
      operation == Operation::Struct ||
      operation == Operation::Module ||
      operation == Operation::For;
  }

  inline bool IsSingleKeyword(Operation operation) {
    return operation == Operation::End ||
      operation == Operation::Else ||
      operation == Operation::Continue ||
      operation == Operation::Break;
  }

  //temporary patch
  inline bool IgnoreVoidCall(Operation operation) {
    return operation == Operation::Return || operation == Operation::For ||
      operation == Operation::If || operation == Operation::While;
  }

  string GetLeftBracket(string rhs) {
    if (rhs == ")") return "(";
    if (rhs == "]") return "[";
    if (rhs == "}") return "{";
    return string();
  }

  string IndentationAndCommentProc(string target) {
    if (target == "") return "";
    string data;
    char current = 0, last = 0;
    size_t head = 0, tail = 0;
    bool exempt_blank_char = true;
    bool string_processing = false;
    auto toString = [](char t) ->string {return string().append(1, t); };

    for (size_t count = 0; count < target.size(); ++count) {
      current = target[count];
      auto type = lexical::GetStringType(toString(current));
      if (type != LiteralType::Whitespace && exempt_blank_char) {
        head = count;
        exempt_blank_char = false;
      }
      if (current == '\'' && last != '\\') {
        string_processing = !string_processing;
      }
      if (!string_processing && current == '#') {
        tail = count;
        break;
      }
      last = target[count];
    }
    if (tail > head) data = target.substr(head, tail - head);
    else data = target.substr(head, target.size() - head);
    if (data.front() == '#') return "";

    while (!data.empty() &&
      lexical::GetStringType(toString(data.back())) == LiteralType::Whitespace) {
      data.pop_back();
    }
    return data;
  }

  void LexicalAnalysis::Scan(deque<string> &output, string target) {
    string current_token, temp;
    bool inside_string = false;
    bool leave_string = false;
    bool enter_string = false;
    bool escape_flag = false;
    bool not_escape_char = false;
    char current = 0, next = 0, last = 0;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = target[idx];

      (idx < target.size() - 1) ?
        next = target[idx + 1] :
        next = 0;

      if (leave_string) {
        inside_string = false;
        leave_string = false;
      }

      escape_flag = (inside_string && last == '\\' && !not_escape_char);

      if (not_escape_char) not_escape_char = false;

      if (current == '\'' && !escape_flag) {
        if (!inside_string && lexical::GetStringType(current_token) == LiteralType::Whitespace) {
          current_token.clear();
        }

        inside_string ?
          leave_string = true :
          inside_string = true, enter_string = true;
      }

      if (!inside_string || enter_string) {
        temp = current_token;
        temp.append(1, current);

        auto type = lexical::GetStringType(temp);
        if (type == LiteralType::Invalid) {
          auto type = lexical::GetStringType(current_token);
          switch (type) {
          case LiteralType::Whitespace:
            current_token.clear();
            current_token.append(1, current);
            break;
          case LiteralType::Int:
            if (current == '.' && lexical::IsDigit(next)) {
              current_token.append(1, current);
            }
            else {
              output.emplace_back(current_token);
              current_token.clear();
              current_token.append(1, current);
            }
            break;
          default:
            output.emplace_back(current_token);
            current_token.clear();
            current_token.append(1, current);
            break;
          }
        }
        else {
          if (type == LiteralType::Int && (temp[0] == '+' || temp[0] == '-')) {
            output.emplace_back(string().append(1, temp[0]));
            current_token = temp.substr(1, temp.size() - 1);
          }
          else {
            current_token = temp;
          }
        }


        if (enter_string) enter_string = false;
      }
      else {
        if (escape_flag) current = lexical::GetEscapeChar(current);
        if (current == '\\' && last == '\\') not_escape_char = true;
        current_token.append(1, current);
      }

      last = target[idx];
    }

    if (lexical::GetStringType(current_token) != LiteralType::Whitespace) {
      output.emplace_back(current_token);
    }

  }

  bool LexicalAnalysis::Feed(CombinedCodeline &src) {
    bool good = true;
    bool negative_flag = false;
    stack<string> bracket_stack;
    deque<string> target;
    Token current = INVALID_TOKEN;
    Token next = INVALID_TOKEN;
    Token last = INVALID_TOKEN;

    Scan(target, src.second);

    dest_->emplace_back(CombinedToken(src.first, deque<Token>()));

    auto *tokens = &dest_->back().second;

    for (size_t idx = 0; idx < target.size(); idx += 1) {
      current = Token(target[idx], lexical::GetStringType(target[idx]));
      next = (idx < target.size() - 1) ?
        Token(target[idx + 1], lexical::GetStringType(target[idx + 1])) :
        INVALID_TOKEN;

      if (current.first == ";") {
        if (!bracket_stack.empty()) {
          AppendMessage("Invalid end of statment at line " +
            to_string(src.first), StateLevel::Error, logger_);
          good = false;
          break;
        }

        if (idx == target.size() - 1) {
          AppendMessage("Unnecessary semicolon at line " +
            to_string(src.first), StateLevel::Warning, logger_);
        }
        else {
          dest_->emplace_back(CombinedToken(src.first, deque<Token>()));
          tokens = &dest_->back().second;
        }

        continue;
      }

      if (current.second == LiteralType::Invalid) {
        AppendMessage("Unknown token - " + current.first +
          " at line " + to_string(src.first), StateLevel::Error, logger_);
        good = false;
        break;
      }

      if (compare(current.first, "+", "-") && !compare(last.first, ")", "]", "}")) {
        if (compare(last.second, LiteralType::Symbol, LiteralType::Invalid) &&
          compare(next.second, LiteralType::Int, LiteralType::Float)) {
          negative_flag = true;
          tokens->push_back(current);
          last = current;
          continue;
        }
      }

      if (compare(current.first, "(", "[", "{")) {
        bracket_stack.push(current.first);
      }

      if (compare(current.first, ")", "]", "}")) {
        if (bracket_stack.empty()) {
          AppendMessage("Left bracket is missing - " + current.first +
            " at line " + to_string(src.first), StateLevel::Error, logger_);
          good = false;
          break;
        }

        if (GetLeftBracket(current.first) != bracket_stack.top()) {
          AppendMessage("Left bracket is missing - " + current.first +
            " at line " + to_string(src.first), StateLevel::Error, logger_);
          good = false;
          break;
        }

        bracket_stack.pop();
      }

      if (current.first == ",") {
        if (last.second == LiteralType::Symbol &&
          !compare(last.first, "]", ")", "}", "'")) {
          AppendMessage("Invalid comma at line " + to_string(src.first), StateLevel::Error,
            logger_);
          good = false;
          break;
        }
      }

      if (compare(last.first, "+", "-") && negative_flag) {
        Token combined(last.first + current.first, current.second);
        tokens->back() = combined;
        negative_flag = false;
        last = combined;
        continue;
      }


      tokens->push_back(current);
      last = current;
    }

    return good;
  }

  void ParserFrame::Eat() {
    if (idx < tokens.size()) {
      last = current;
      current = tokens[idx];
      next = idx + 1 < tokens.size() ? tokens[idx + 1] : INVALID_TOKEN;
      next_2 = idx + 2 < tokens.size() ? tokens[idx + 2] : INVALID_TOKEN;
      idx += 1;
    }

    if (idx == tokens.size()) {
      eol = true;
    }
  }

  void FirstStageParsing::ProduceVMCode() {
    deque<Argument> arguments;
    size_t idx = 0, limit = 0;

    if (frame_->nodes.back().IsPlaceholder()) frame_->nodes.pop_back();

    bool is_bin_operator = lexical::IsBinaryOperator(frame_->nodes.back().GetOperation());
    bool is_mono_operator = lexical::IsMonoOperator(frame_->nodes.back().GetOperation());

    if (is_bin_operator) limit = 2;
    if (is_mono_operator) limit = 1;
    
    while (!frame_->args.empty() && !frame_->args.back().IsPlaceholder()) {
      if ((is_bin_operator || is_mono_operator) && idx >= limit) break;

      arguments.emplace_front(frame_->args.back());
      
      frame_->args.pop_back();
      (is_bin_operator || is_mono_operator) ? idx += 1 : idx = idx;
    }

    if (!frame_->args.empty()
      && frame_->args.back().IsPlaceholder()
      && !is_bin_operator && !is_mono_operator) {
      
      frame_->args.pop_back();
    }

    action_base_.emplace_back(Sentense(frame_->nodes.back(), arguments));
    frame_->nodes.pop_back();
    frame_->args.emplace_back(Argument("", ArgumentType::RetStack, LiteralType::Invalid));
    if (frame_->nodes.empty() && !IgnoreVoidCall(action_base_.back().first.GetOperation()) &&
      (frame_->next.first == "," || frame_->next.second == LiteralType::Invalid)) {
      action_base_.back().first.annotation.void_call = true;
    }
  }

  bool FirstStageParsing::CleanupStack() {
    bool result = true;

    while (!frame_->nodes.empty() && !frame_->nodes.back().IsPlaceholder()) {
      ProduceVMCode();
    }

    return result;
  }

  void FirstStageParsing::BindStmt() {
    if (!frame_->args.empty()) {
      ASTNode node(Operation::Bind);
      node.annotation.local_object = frame_->local_object;
      frame_->local_object = false;
      frame_->nodes.emplace_back(node);
    }
  }

  void FirstStageParsing::DeliveringStmt() {
    if (!frame_->args.empty()) {
      ASTNode node(Operation::Delivering);
      node.annotation.local_object = frame_->local_object;
      frame_->local_object = false;
      frame_->nodes.emplace_back(node);
    }
  }

  void FirstStageParsing::ScopeMemberStmt() {
    if (!frame_->seek_last_assert) {
      frame_->domain = frame_->args.back();
      frame_->args.pop_back();
    }
  }

  void FirstStageParsing::UnaryExpr() {
    auto token = lexical::GetKeywordCode(frame_->current.first);
    frame_->nodes.emplace_back(ASTNode(token));
  }

  void FirstStageParsing::Calling() {
    if (frame_->last.second != LiteralType::Identifier) {
      frame_->nodes.emplace_back(ASTNode(Operation::ExpList));
    }

    if (compare(lexical::GetKeywordCode(frame_->last.first),
      Operation::If, Operation::Elif, Operation::While,
      Operation::Case, Operation::When, Operation::Return)) {
      frame_->nodes.emplace_back(ASTNode(Operation::ExpList));
    }
    
    frame_->nodes.push_back(ASTNode());
    frame_->args.emplace_back(Argument());
  }

  bool FirstStageParsing::GetElementStmt() {
    ASTNode node(kStrAt, frame_->args.back());
    frame_->nodes.emplace_back(node);
    frame_->nodes.emplace_back(ASTNode());
    frame_->args.pop_back();
    frame_->args.emplace_back(Argument());

    return true;
  }

  bool FirstStageParsing::ArrayGeneratorStmt() {
    bool result = true;
    if (frame_->last.second == LiteralType::Symbol) {
      frame_->nodes.emplace_back(ASTNode(Operation::InitialArray));
      frame_->nodes.emplace_back(ASTNode());
      frame_->args.emplace_back(Argument());
      result = true;
    }
    else {
      result = false;
      error_string_ = "Illegal brace location.";
    }
    return result;
  }

  void FirstStageParsing::BinaryExpr() {
    auto token = lexical::GetKeywordCode(frame_->current.first);
    int current_priority = lexical::GetTokenPriority(token);
    ASTNode node(token);

    if (!frame_->nodes.empty()) {
      bool is_operator = lexical::IsBinaryOperator(frame_->nodes.back().GetOperation());
      int stack_top_priority = lexical::GetTokenPriority(frame_->nodes.back().GetOperation());

      auto checking = [&stack_top_priority, &current_priority]()->bool {
        return (stack_top_priority >= current_priority);
      };

      while (!frame_->nodes.empty() && is_operator && checking()) {
        ProduceVMCode();
        is_operator = (!frame_->nodes.empty() && lexical::IsBinaryOperator(frame_->nodes.back().GetOperation()));
        stack_top_priority = frame_->nodes.empty() ? 5 : lexical::GetTokenPriority(frame_->nodes.back().GetOperation());
      }
    }

    frame_->nodes.emplace_back(node);
  }

  bool FirstStageParsing::FunctionHeaderStmt() {
    //TODO:Preprecessing-time argument type checking


    if (frame_->last.second != LiteralType::Invalid || tokens_.size() < 4 || frame_->next_2.first != "(") {
      error_string_ = "Invalid function definition";
      return false;
    }

    bool left_paren = false;
    bool inside_params = false;
    bool variable = false;
    bool good = true;

    frame_->nodes.emplace_back(ASTNode(Operation::Fn));
    frame_->nodes.emplace_back(ASTNode());
    frame_->args.emplace_back(Argument());

    auto invalid_param_pattern = [&]()->bool {
      if (!inside_params) return true;
      return frame_->next.first == "," || frame_->next.first == kStrVariable;
    };

    //Function identifier
    frame_->Eat();
    if (auto &str = frame_->current.first; str == kStrVariable) {
      error_string_ = "Invalid function identifier";
      return false;
    }

    frame_->args.emplace_back(
      Argument(frame_->current.first, ArgumentType::Literal, LiteralType::Identifier));

    //Parameter segment
    //left parenthesis will be disposed in first loop
    while (!frame_->eol) {
      frame_->Eat();

      if (frame_->current.first == "(") {
        if (left_paren) {
          error_string_ = "Invalid nodes in function definition";
          good = false;
          break;
        }
        else {
          left_paren = true;
          inside_params = true;
          continue;
        }
      }
      else if (frame_->current.first == ")") {
        inside_params = false;
        if (frame_->next.first == kStrConstraintArrow) {
          //dispose right parenthesis
          frame_->Eat();
          if (frame_->next.second == LiteralType::Invalid) {
            error_string_ = "Invalid return value constraint";
            return false;
          }

          //dispose right arrow
          frame_->Eat();
          if (frame_->current.second != LiteralType::Identifier) {
            error_string_ = "Invalid return value constraint";
            return false;
          }

          Argument constaint_arg(frame_->current.first, ArgumentType::Literal, LiteralType::Identifier);
          constaint_arg.properties.fn.constraint = true;
          frame_->args.emplace_back(constaint_arg);
        }

        ProduceVMCode();
        break;
      }
      else if (frame_->current.first == kStrVariable) {
        if (invalid_param_pattern()) {
          error_string_ = "Invalid function parameter declaration.";
          good = false;
          break;
        }

        variable = true;
      }
      else if (frame_->current.first == ",") continue;
      else if (frame_->current.second != LiteralType::Identifier) {
        error_string_ = "Invalid value in function definition";
        good = false;
        break;
      }
      else {
        Argument arg(frame_->current.first, ArgumentType::Literal, LiteralType::Identifier);

        if (variable) {
          arg.properties.fn.variable_param = true;
          variable = false;
        }

        frame_->args.emplace_back(arg);
      }
    }

    return good;
  }

  bool FirstStageParsing::StructHeaderStmt(Terminator terminator) {
    //TODO: allow access with type identifier(struct/module)
    if (frame_->last.second != LiteralType::Invalid) {
      error_string_ = "Invalid struct/module definition";
      return false;
    }

    if (tokens_.size() < 2) {
      error_string_ = "Invalid struct/module definition head";
      return false;
    }

    switch (terminator) {
    case Terminator::Struct:
      frame_->nodes.emplace_back(ASTNode(Operation::Struct));
      break;
    case Terminator::Module:
      frame_->nodes.emplace_back(ASTNode(Operation::Module));
      break;
    default:
      break;
    }
    
    frame_->nodes.emplace_back(ASTNode());
    frame_->Eat();

    if (frame_->current.second != LiteralType::Identifier) {
      error_string_ = "Invalid struct identifier";
      return false;
    }

    frame_->args.emplace_back(Argument());
    //struct identifier
    frame_->args.emplace_back(Argument(
      frame_->current.first, ArgumentType::Literal, LiteralType::Identifier));

    if (terminator == Terminator::Module && frame_->next.second != LiteralType::Invalid) {
      error_string_ = "Invalid argument in module definition";
      return false;
    }
    else if (terminator == Terminator::Struct && frame_->next.first == "<") {
      //inheritance source struct
      frame_->Eat(); frame_->Eat();
      frame_->args.emplace_back(Argument(
        frame_->current.first, ArgumentType::Literal, LiteralType::Identifier));
    }

    return true;
  }

  bool FirstStageParsing::ForEachStmt() {
    if (frame_->last.second != LiteralType::Invalid) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    if (tokens_.size() < 4) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    bool good = true;

    frame_->nodes.emplace_back(ASTNode(Operation::For));
    frame_->nodes.emplace_back(ASTNode());
    frame_->args.emplace_back(Argument());

    if (frame_->Eat(); lexical::GetStringType(frame_->current.first) != LiteralType::Identifier) {
      error_string_ = "Invalid identifier argument in for-each expression";
      return false;
    }

    frame_->args.emplace_back(Argument(
      frame_->current.first, ArgumentType::Literal, LiteralType::Identifier));

    
    if (frame_->Eat(); lexical::GetTerminatorCode(frame_->current.first) != Terminator::In) {
      error_string_ = "Invalid for-each expression";
      return false;
    }

    return good;
  }

  bool FirstStageParsing::OtherExpressions() {
    Operation token = frame_->last.first != "." ?
      lexical::GetKeywordCode(frame_->current.first) : Operation::Null;

    if (IsSingleKeyword(token)) {
      if (frame_->next.second != LiteralType::Invalid) {
        error_string_ = "Invalid syntax after " + frame_->current.first;
        return false;
      }

      ASTNode node(token);
      frame_->nodes.emplace_back(node);
      return true;
    }

    if (token == Operation::Local) {
      if (frame_->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      frame_->local_object = true;
      return true;
    }

    if (token == Operation::Ext) {
      if (frame_->next_2.first != "=") {
        error_string_ = "Invalid 'local' token.";
        return false;
      }

      frame_->ext_object = true;
      return true;
    }

    if (token != Operation::Null) {
      if (frame_->next.first == "=" || lexical::IsOperator(token)) {
        error_string_ = "Trying to operate with reserved operation";
        return false;
      }

      if (IsReservedKeyword(token)) {
        frame_->nodes.emplace_back(ASTNode(token));
        frame_->args.emplace_back(Argument());
        return true;
      }
      else if (IsVariableExpression(token)) {
        frame_->nodes.emplace_back(ASTNode(token));
        frame_->nodes.emplace_back(ASTNode());
        frame_->args.emplace_back(Argument());
        return true;
      }
      else {
        if (frame_->next.first != "(") {
          error_string_ = "Invalid syntax after " + frame_->current.first;
          return false;
        }
      }

      ASTNode node(token);
      frame_->nodes.emplace_back(node);

      return true;
    }

    if (frame_->next.first == "(") {
      ASTNode node(frame_->current.first,
        frame_->domain.IsPlaceholder() ?
        Argument() : frame_->domain
      );
      node.annotation.use_last_assert = frame_->seek_last_assert;
      if (frame_->seek_last_assert) frame_->seek_last_assert = false;
      frame_->nodes.emplace_back(node);
      frame_->domain = Argument();
      return true;
    }
    else if ((frame_->next.first == "=" || frame_->next.first == "<-") &&
      frame_->last.first != ".") {
      frame_->args.emplace_back(Argument(
        frame_->current.first, ArgumentType::Literal, LiteralType::Identifier));
      return true;
    }
    else {
      frame_->args.emplace_back(Argument(
        frame_->current.first, ArgumentType::Pool, LiteralType::Identifier));

      if (!frame_->domain.IsPlaceholder() || frame_->seek_last_assert) {
        frame_->args.back().properties.domain.id = frame_->domain.GetData();
        frame_->args.back().properties.domain.type = frame_->domain.GetType();

        if (frame_->seek_last_assert) {
          frame_->args.back().properties.member_access.use_last_assert = true;
          frame_->seek_last_assert = false;
        }

        if (frame_->next.first == ".") {
          ASTNode node(Operation::DomainAssertCommand);
          Sentense command;

          command.first = node;
          command.second.push_back(frame_->args.back());
          frame_->args.pop_back();
          action_base_.push_back(command);
          frame_->seek_last_assert = true;
        }
        else {
          frame_->args.back().properties.member_access.is_chain_tail = true;
        }

        frame_->domain = Argument();
      }

      return true;
    }

    Argument arg(
      frame_->current.first, ArgumentType::Pool, LiteralType::Identifier);
    frame_->args.emplace_back(arg);

    return true;
  }

  void FirstStageParsing::LiteralValue() {
    frame_->args.emplace_back(
      Argument(frame_->current.first, ArgumentType::Literal, frame_->current.second));
  }


  Message FirstStageParsing::Parse() {
    auto state = true;
    const auto size = tokens_.size();
    Message result;

    frame_ = new ParserFrame(tokens_);

    while (!frame_->eol) {
      if (!state) break;
      frame_->Eat();

      if (Terminator value = lexical::GetTerminatorCode(frame_->current.first); 
        value != Terminator::Null) {
        switch (value) {
        case Terminator::Assign:
          BindStmt();
          break;
        case Terminator::Arrow:
          DeliveringStmt();
          break;
        case Terminator::Comma:
          state = CleanupStack();
          break;
        case Terminator::Dot:
          ScopeMemberStmt();
          break;
        case Terminator::LeftParen:
          Calling();
          break;
        case Terminator::LeftBracket:
          state = GetElementStmt();
          break;
        case Terminator::LeftBrace:
          state = ArrayGeneratorStmt();
          break;
        case Terminator::MonoOperator:
          UnaryExpr();
          break;
        case Terminator::BinaryOperator:
          BinaryExpr();
          break;
        case Terminator::Fn:
          state = FunctionHeaderStmt();
          break;
        case Terminator::Struct:
        case Terminator::Module:
          state = StructHeaderStmt(value);
          break;
        case Terminator::For:
          state = ForEachStmt();
          break;
        case Terminator::RightSqrBracket:
        case Terminator::RightBracket:
        case Terminator::RightCurBracket:
          state = CleanupStack();
          if (state) ProduceVMCode();
          break;
        default:
          break;
        }
      }
      else {
        auto token_type = frame_->current.second;

        if (token_type == LiteralType::Identifier) {
          state = OtherExpressions();
        }
        else {
          LiteralValue();
        }
      }
    }

    if (state) {
      while (!frame_->nodes.empty()) {
        ProduceVMCode();
      }
    }
    else {
      result = ERROR_MSG(error_string_).SetIndex(index_);
    }

    for (auto &unit : action_base_) {
      unit.first.idx = index_;
    }

    frame_->args.clear();
    frame_->args.shrink_to_fit();
    frame_->nodes.clear();
    frame_->nodes.shrink_to_fit();
    delete frame_;
    return result;
  }

  void FirstStageParsing::Clear() {
    tokens_.clear();
    tokens_.shrink_to_fit();
    action_base_.clear();
    action_base_.shrink_to_fit();
    error_string_.clear();
    index_ = 0;
  }

  Message FirstStageParsing::Make(CombinedToken &line) {
    index_ = line.first;
    tokens_.clear();
    tokens_ = line.second;
    return Parse().SetIndex(line.first);
  }

  bool GrammarAndSemanticAnalysis::ReadScript(list<CombinedCodeline> &dest) {
    bool inside_comment_block = false;
    size_t idx = 1;
    string buf;
    InStream reader(path_);

    if (!reader.Good()) return false;

    while (!reader.eof()) {
      buf = reader.GetLine();

      if (buf == kStrCommentBegin) {
        inside_comment_block = true;
        idx += 1;
        continue;
      }

      if (inside_comment_block) {
        inside_comment_block = !(buf == kStrCommentEnd);
        idx += 1;
        continue;
      }

      buf = IndentationAndCommentProc(buf);

      if (buf.empty()) {
        idx += 1;
        continue;
      }

      dest.push_back(CombinedCodeline(idx, buf));
      idx += 1;
    }

    return true;
  }

  //TODO: for-each processing still works not correctly. check it out again
  bool GrammarAndSemanticAnalysis::Start() {
    bool good = true;
    LexicalAnalysis lexer(tokens_, logger_);
    FirstStageParsing parser;
    Message msg;
    AnnotatedAST anchor;
    StateLevel level;
    Operation ast_root;

    if (!ReadScript(script_)) return false;

    for (auto it = script_.begin(); it != script_.end(); ++it) {
      good = lexer.Feed(*it);
      if (!good) break;
    }

    if (!good) return false;

    //code generating & analyzing
    for (auto it = tokens_.begin(); it != tokens_.end(); ++it) {
      if (!good) break;

      msg = parser.Make(*it);

      level = msg.GetLevel();
      ast_root = parser.GetASTRoot();

      if (level != StateLevel::Normal) {
        AppendMessage(msg.GetDetail(), level, logger_, msg.GetIndex());

        if (level == StateLevel::Error) { 
          good = false; 
          continue; 
        }
      }

      anchor.swap(parser.GetOutput());
      parser.Clear();

      if (inside_struct_) {
        if (ast_root == Operation::Fn) struct_member_fn_nest += 1;

        if (struct_member_fn_nest == 0 && 
          !compare(ast_root, Operation::Bind, Operation::End, Operation::Include, Operation::Attribute)) {
          AppendMessage("Invalid expression inside struct", StateLevel::Error, logger_, msg.GetIndex());
          good = false;
          break;
        }
      }

      if (inside_module_) {
        if (ast_root == Operation::Fn) struct_member_fn_nest += 1;

        if (struct_member_fn_nest == 0 &&
          !compare(ast_root, Operation::Bind, Operation::End, Operation::Attribute)) {
          AppendMessage("Invalid expression inside struct", StateLevel::Error, logger_, msg.GetIndex());
          good = false;
          break;
        }
      }

      if (IsNestRoot(ast_root)) {
        if (ast_root == Operation::If || ast_root == Operation::Case) {
          jump_stack_.push(JumpListFrame{ ast_root,
            dest_->size() + anchor.size() - 1 });
        }

        if (ast_root == Operation::While || ast_root == Operation::For) {
          cycle_escaper_.push(nest_.size() + 1);
        }

        if (ast_root == Operation::Struct) {
          inside_struct_ = true;
        }

        if (ast_root == Operation::Module) {
          inside_module_ = true;
        }
        
        nest_.push(ast_root == Operation::For ? dest_->size() + anchor.size() - 1 : dest_->size());
        nest_end_.push(dest_->size() + anchor.size() - 1);
        nest_origin_.push(it->first);
        nest_type_.push(ast_root);
        dest_->insert(dest_->end(), anchor.begin(), anchor.end());
        anchor.clear();
        continue;
      }

      if (IsBranchKeyword(ast_root)) {
        if (jump_stack_.empty()) {
          AppendMessage("Invalid branch operation at line " + to_string(it->first), StateLevel::Error, logger_);
          break;
        }

        if (jump_stack_.top().nest_code == Operation::If) {
          if (ast_root == Operation::Elif || ast_root == Operation::Else) {
            jump_stack_.top().jump_record.push_back(dest_->size());
          }
          else {
            AppendMessage("Invalid branch operation at line " + to_string(it->first), logger_);
            break;
          }
        }
        else if (jump_stack_.top().nest_code == Operation::Case) {
          if (ast_root == Operation::When || ast_root == Operation::Else) {
            jump_stack_.top().jump_record.push_back(dest_->size());
          }
          else {
            AppendMessage("Invalid branch operation at line " + to_string(it->first), StateLevel::Error, logger_);
            break;
          }
        }
      }

      if (ast_root == Operation::Continue || ast_root == Operation::Break) {
        if (cycle_escaper_.empty()) {
          AppendMessage("Invalid cycle escaper at line " + to_string(it->first), StateLevel::Error, logger_);
          break;
        }

        anchor.back().first.annotation.escape_depth = nest_.size() - cycle_escaper_.top();
      }

      if (ast_root == Operation::End) {
        if (nest_type_.empty()) {
          AppendMessage("Invalid 'end' token at line " + to_string(it->first), StateLevel::Error, logger_, msg.GetIndex());
          good = false;
          break;
        }

        if (!cycle_escaper_.empty() && nest_.size() == cycle_escaper_.top())
          cycle_escaper_.pop();

        (*dest_)[nest_end_.top()].first.annotation.nest_end = dest_->size();
        // Writing nest header info into 'end' command (anchor.back())
        auto &writing_dest = anchor.back();
        writing_dest.first.annotation.nest_root = nest_type_.top();
        writing_dest.first.annotation.nest = nest_.top();

        if (compare(nest_type_.top(), Operation::If, Operation::Case) && !jump_stack_.empty()){
          if (!jump_stack_.top().jump_record.empty()) {
            dest_->AddJumpRecord(jump_stack_.top().nest, jump_stack_.top().jump_record);
          }
          jump_stack_.pop();
        }

        if (compare(nest_type_.top(), Operation::Fn) && (inside_struct_ || inside_module_)) {
          struct_member_fn_nest -= 1;
        }

        if (compare(nest_type_.top(), Operation::Struct)) inside_struct_ = false;
        if (compare(nest_type_.top(), Operation::Module)) inside_module_ = false;

        nest_.pop();
        nest_end_.pop();
        nest_origin_.pop();
        nest_type_.pop();
      }

      //Insert processed vmcode into destination and dispose.
      dest_->insert(dest_->end(), anchor.begin(), anchor.end());
      anchor.clear();
    }

    if (!nest_.empty()) {
      AppendMessage("'end' token is not found for line " + to_string(nest_origin_.top()), StateLevel::Error, logger_);
      good = false;
    }

    //toke id generation
    if (good) {
      for (auto it = dest_->begin(); it != dest_->end(); ++it) {
        if (compare(it->first.GetOperation(), Operation::Bind, Operation::Delivering)
          && it->second.size() == 2 && it->second[0].GetType() == ArgumentType::Literal) {

          it->second[0].properties.token_id = TryAppendTokenId(it->second[0].GetData());
        }

        //node domain
        if (it->first.HasDomain()) {
          it->first.SetDomainTokenId(TryAppendTokenId(it->first.GetFunctionDomain().GetData()));
        }

        //argument
        for (auto &unit : it->second) {
          if (unit.GetType() == ArgumentType::Pool) {
            unit.properties.token_id = TryAppendTokenId(unit.HasDomain() ? unit.properties.domain.id : unit.GetData());
          }
        }
      }
    }

    return good;
  }
}
