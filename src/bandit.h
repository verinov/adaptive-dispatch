#include <cassert>
#include <chrono>
#include <tuple>
#include <utility>

#define COMPILE_SWITCH_CASE(index)                                             \
  case index:                                                                  \
    if constexpr ((index) < n_options) {                                       \
      return std::get<index>(t)(std::forward<Args>(args)...);                  \
    }

template <typename... Args, typename FuncTuple>
auto CompileSwitch(int index, FuncTuple &&t, Args &&... args) {
  constexpr int n_options =
      std::tuple_size<typename std::decay<decltype(t)>::type>::value;
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

template <typename Selector> struct SelectorTimer {
  Selector *selector_;
  std::chrono::time_point<std::chrono::system_clock> start_time_;
  int option;

  using DurationT = std::chrono::nanoseconds;

  SelectorTimer(Selector *selector, int opt)
      : selector_{selector}, option{opt} {
    if (selector_) {
      start_time_ = std::chrono::system_clock::now();
    }
  }

  ~SelectorTimer() {
    if (selector_) {
      selector_->FinishTimed(
          std::chrono::duration_cast<DurationT>(
              std::chrono::system_clock::now() - start_time_),
          option);
    }
  }
};

template <int N> struct EpsGreedySelector {
  static constexpr int n_options = N;
  using TimerT = SelectorTimer<EpsGreedySelector<N>>;

  int best_index_ = 0;
  int left_exploiting_ = 1;
  int next_explore_ = 1;

  typename TimerT::DurationT prev_time_;

  static int NextIndex(int index) {
    index++;
    return (index != n_options) * index;
  }

  auto GetOptionTimed() {
    switch (left_exploiting_) {
    case 0:
      left_exploiting_ = 100;
      return TimerT{this, next_explore_};
    case 1: // last before exploration
      --left_exploiting_;
      return TimerT{this, best_index_};
    default: // exploit
      --left_exploiting_;
      return TimerT{nullptr, best_index_};
    }
  }

  void FinishTimed(typename TimerT::DurationT time, int option) {
    if (option != best_index_) {
      // exploration
      if (prev_time_ > time) {
        best_index_ = option;
      }
      next_explore_ = NextIndex(next_explore_);
      if (next_explore_ == best_index_) {
        next_explore_ = NextIndex(next_explore_);
      }
    } else {
      // last exploitation
      prev_time_ = time;
    }
  }
};

template <typename... Args, typename FuncTuple,
          typename Selector = EpsGreedySelector<
              std::tuple_size<typename std::decay<FuncTuple>::type>::value>>
auto Dispatch(FuncTuple &&funcs) {
  struct F {
    typename std::decay<FuncTuple>::type funcs;
    Selector selector{};

    auto operator()(Args &&... args) {
      if constexpr (std::tuple_size<
                        typename std::decay<FuncTuple>::type>::value < 2) {
        return std::get<0>(funcs)(std::forward<Args>(args)...);
      }

      auto timed_option{selector.GetOptionTimed()};
      return CompileSwitch(timed_option.option, funcs,
                           std::forward<Args>(args)...);
    }
  };
  return F{funcs};
}
