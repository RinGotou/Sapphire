#include "machine.h"

namespace sapphire {
  int NewTimeClass(State &state, ObjectMap &p) {
    auto time_obj = make_shared<time_t>(time(nullptr));
    state.PushValue(Object(time_obj, kTypeIdTime));
    return 0;
  }

  int Time_ToString(State &state, ObjectMap &p) {
    auto &me = p.Cast<time_t>(kStrMe);
    string result(ctime(&me));
    state.PushValue(Object(result, kTypeIdString));
    return 0;
  }

  int Time_DiffAsEnd(State &state, ObjectMap &p) {
    auto &me = p.Cast<time_t>(kStrMe);
    auto &begin = p.Cast<time_t>("begin");
    state.PushValue(Object(difftime(me, begin), kTypeIdFloat));
    return 0;
  }

  int Time_DiffAsBegin(State &state, ObjectMap &p) {
    auto &me = p.Cast<time_t>(kStrMe);
    auto &end = p.Cast<time_t>("end");
    state.PushValue(Object(difftime(end, me), kTypeIdFloat));
    return 0;
  }

  //TODO: More methods?

  void InitTimeType() {
    using namespace components;
    CreateStruct(kTypeIdTime);
    StructMethodGenerator(kTypeIdTime).Create(
      {
        Function(NewTimeClass, "", kStrInitializer),
        Function(Time_ToString, "", "to_string"),
        Function(Time_DiffAsBegin, "end", "diff_as_begin"),
        Function(Time_DiffAsEnd, "begin", "diff_as_end")
      }
    );

    EXPORT_CONSTANT(kTypeIdTime);
  }
}

