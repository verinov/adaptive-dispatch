#pragma once

#include "compile_switch.h"

// #include <chrono>
#include <limits>
#include <tuple>
#include <x86intrin.h>

using OptionIntegralT = int8_t;

/*
RAII type to optionally run timer and call `Selector::FinishTimed` on
destruction.
*/
template <typename Selector> class TimedOption {
  Selector *selector_; // nullptr for disabled
  // start_time_ is only set when selector_ != nullptr.
  decltype(__rdtsc()) start_time_;
  OptionIntegralT option_; // must be set to a valid value

public:
  using DurationT = decltype(__rdtsc());
  TimedOption(Selector *selector, OptionIntegralT option)
      : selector_{selector}, start_time_{0}, option_{option} {}

  ~TimedOption() {
    if (selector_) {
      selector_->FinishTimed(__rdtsc() - start_time_, option_);
    }
  }

  // can't use rvalue for timing
  operator int() const && = delete;
  operator int() & {
    if (selector_) {
      start_time_ = __rdtsc();
    }
    return option_;
  }
};

template <int N> class TimingSelector {
  static_assert(N < std::numeric_limits<OptionIntegralT>::max(),
                "Need large OptionIntegralT");
  static_assert(-1 > std::numeric_limits<OptionIntegralT>::min(),
                "OptionIntegralT must support negative numbers");

  using Class = TimingSelector<N>;
  using TimerT = TimedOption<Class>;

  static constexpr OptionIntegralT n_options{N};
  static constexpr int n_warmup{4};
  static constexpr int n_measure{4};
  static constexpr int n_exploit{128};
  static_assert(n_warmup > 0);
  static_assert(n_measure > 0);
  static_assert(n_exploit > 0);
  static constexpr auto max_duration{
      std::numeric_limits<typename TimerT::DurationT>::max()};

  // Hot memory
  int phase_left_{0};
  OptionIntegralT current_option_{0};
  enum class Phase : char { exploit, warmup, measure } phase_ = Phase::exploit;

  // Cold memory
  OptionIntegralT best_option_{0};
  OptionIntegralT last_best_option_{-1};
  int best_option_streak_{0};
  typename TimerT::DurationT current_duration_;
  typename TimerT::DurationT best_duration_ = max_duration;

  void ExitWarmup() {
    phase_ = Phase::measure;
    phase_left_ = n_measure;
    current_duration_ = 0;
  }

  void ExitMeasure() {
    // finalize the measurement
    if (current_duration_ < best_duration_) {
      best_duration_ = current_duration_;
      best_option_ = current_option_;
    }

    // transition to the next phase
    current_option_ += 1;
    if (current_option_ != n_options) {
      phase_ = Phase::warmup;
      phase_left_ = n_warmup;
      return;
    }

    phase_ = Phase::exploit;
    if (best_option_ == last_best_option_) {
      best_option_streak_ += (best_option_streak_ < 10);
    } else {
      best_option_streak_ = 0;
    }
    phase_left_ = n_exploit << best_option_streak_;
    current_option_ = best_option_;
    last_best_option_ = best_option_;
  }

  void ExitExploit() {
    best_duration_ = max_duration;
    current_option_ = 0;
    phase_ = Phase::warmup;
    phase_left_ = n_warmup;
  }

  void NewPhase() {
    switch (phase_) {
    case Phase::warmup:
      return ExitWarmup();
    case Phase::measure:
      return ExitMeasure();
    case Phase::exploit:
      return ExitExploit();
    }
    __builtin_unreachable();
  }

public:
  int GetOption() const { return current_option_; }
  TimerT GetOptionTimed() {
    if (phase_left_ == 0) {
      NewPhase();
    } else {
      phase_left_ -= 1;
    }

    switch (phase_) {
    case Phase::warmup:
      return TimerT(nullptr, current_option_);
    case Phase::measure:
      return TimerT(this, current_option_);
    case Phase::exploit:
      return TimerT(nullptr, current_option_);
    }
    __builtin_unreachable();
  }

  void FinishTimed(typename TimerT::DurationT duration, int option) {
    assert(current_option_ == option);
    assert(phase_ == Phase::measure);

    current_duration_ += duration;
  }
};

template <typename... Args, template <int> typename Selector = TimingSelector,
          typename FuncTuple>
inline auto Dispatch(FuncTuple &&funcs) {
  constexpr int N =
      std::tuple_size<typename std::decay<FuncTuple>::type>::value;

  struct F {
    typename std::decay<FuncTuple>::type funcs;
    Selector<N> selector{};

    auto operator()(Args &&... args) {
      if constexpr (1 == N) {
        // When there is just one option, just call it.
        return std::get<0>(funcs)(std::forward<Args>(args)...);
      }

      // To allow for both void and non-void return type, it's easier to
      // directly return the result. To also stop the timer after this,
      // we use the destructor of TimedOption helper class.
      auto timed_option{selector.GetOptionTimed()};
      return CompileSwitch(timed_option, funcs, std::forward<Args>(args)...);
    }
  };
  return F{funcs};
}
