//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "kagamicommon.h"

namespace kagami {
  int Kit::GetDataType(string target) {
    using std::regex_match;
    int result = kTypeNull;
    auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (target == kStrNull) result = kTypeNull;
    else if (target.front() == '"' && target.back() == '"') result = kTypeString;
    else if (match(kPatternBoolean)) result = kTypeBoolean;
    else if (match(kPatternFunction)) result = kTypeFunction;
    else if (match(kPatternInteger)) result = kTypeInteger;
    else if (match(kPatternDouble)) result = kTypeDouble;
    else if (match(kPatternSymbol)) result = kTypeSymbol;
    else if (match(kPatternBlank)) result = kTypeBlank;
    else result = kTypeNull;

    return result;
  }

  bool Kit::FindInStringVector(string target, string source) {
    bool result = false;
    auto methods = this->BuildStringVector(source);
    for (auto &unit : methods) {
      if (unit == target) {
        result = true;
        break;
      }
    }
    return result;
  }

  vector<string> Kit::BuildStringVector(string source) {
    vector<string> result;
    string temp = kStrEmpty;
    for (auto unit : source) {
      if (unit == '|') {
        result.push_back(temp);
        temp.clear();
        continue;
      }
      temp.append(1, unit);
    }
    if (temp != kStrEmpty) result.push_back(temp);
    return result;
  }

  Attribute Kit::GetAttrTag(string target) {
    Attribute result;
    vector<string> base;
    string temp = kStrEmpty;

    for (auto &unit : target) {
      if (unit == '+' || unit == '%') {
        if (temp == kStrEmpty) {
          temp.append(1, unit);
        }
        else {
          base.push_back(temp);
          temp = kStrEmpty;
          temp.append(1, unit);
        }
      }
      else {
        temp.append(1, unit);
      }
    }
    if (temp != kStrEmpty) base.push_back(temp);
    temp = kStrEmpty;

    for (auto &unit : base) {
      if (unit.front() == '%') {
        temp = unit.substr(1, unit.size() - 1);
        if (temp == kStrTrue) {
          result.ro = true;
        }
        else if (temp == kStrFalse) {
          result.ro = false;
        }
      }
      else if (unit.front() == '+') {
        temp = unit.substr(1, unit.size() - 1) + "|";
        result.methods.append(temp);
      }

      temp = kStrEmpty;
    }

    if (result.methods.back() == '|') result.methods.pop_back();

    return result;
  }

  string Kit::BuildAttrStr(Attribute target) {
    string result = kStrEmpty;
    vector<string> methods = this->BuildStringVector(target.methods);
    if (target.ro)result.append("%true");
    else result.append("%false");
    if (!methods.empty()) {
      for (auto &unit : methods) {
        result.append("+" + unit);
      }
    }
    return result;
  }
}