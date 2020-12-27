#pragma once
/*
Just provide the function template CompileSwitch, which picks
a function out of a tuple by index, and calls it with forwarded arguments.
*/

#include <cassert>
#include <tuple>

#define COMPILE_SWITCH_CASE(index)                                             \
  case index:                                                                  \
    if constexpr ((index) < n_options) {                                       \
      return std::get<index>(funcs)(std::forward<Args>(args)...);              \
    }

template <typename... Args, typename FuncTuple>
inline auto CompileSwitch(int index, FuncTuple &&funcs, Args &&... args) {
  constexpr int n_options =
      std::tuple_size<typename std::decay<decltype(funcs)>::type>::value;
  switch (index) {
    COMPILE_SWITCH_CASE(0);
    COMPILE_SWITCH_CASE(1);
    COMPILE_SWITCH_CASE(2);
    COMPILE_SWITCH_CASE(3);
    COMPILE_SWITCH_CASE(4);
    COMPILE_SWITCH_CASE(5);
    COMPILE_SWITCH_CASE(6);
    COMPILE_SWITCH_CASE(7);
    COMPILE_SWITCH_CASE(8);
    COMPILE_SWITCH_CASE(9);
  default:
    if constexpr (10 < n_options)
      assert(false); // Too many function. Add more cases?
  }
}

#undef COMPILE_SWITCH_CASE
