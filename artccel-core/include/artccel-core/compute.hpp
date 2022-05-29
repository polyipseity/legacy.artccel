#ifndef ARTCCEL_CORE_COMPUTE_HPP
#define ARTCCEL_CORE_COMPUTE_HPP
#pragma once

#include <concepts>   // import std::copyable, std::invocable
#include <cstddef>    // import std::nullptr_t
#include <functional> // import std::function
#include <future>     // import std::packaged_task, std::shared_future
#include <memory> // import std::enable_shared_from_this, std::make_shared, std::shared_ptr, std::static_pointer_cast, std::weak_ptr
#include <mutex>  // import std::lock_guard, std::mutex
#include <optional>    // import std::optional
#include <type_traits> // import std::is_function_v, std::is_nothrow_copy_constructible_v, std::is_nothrow_move_constructible_v
#include <utility>     // import std::forward, std::move, std::swap

namespace artccel::core::compute {
template <std::copyable R> class Compute_io;
template <typename Signature>
requires std::is_function_v<Signature>
class Compute_in;
template <std::copyable R> class Compute_out;
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Reset_tag {
  consteval Reset_tag() noexcept = default;
  constexpr ~Reset_tag() noexcept = default;
  constexpr Reset_tag(Reset_tag const &) noexcept = default;
  constexpr auto operator=(Reset_tag const &) noexcept -> Reset_tag & = default;
  constexpr Reset_tag(Reset_tag &&) noexcept = default;
  constexpr auto operator=(Reset_tag &&) noexcept -> Reset_tag & = default;
};
// NOLINTNEXTLINE(altera-struct-pack-align)
struct Extract_tag {
  consteval Extract_tag() noexcept = default;
  constexpr ~Extract_tag() noexcept = default;
  constexpr Extract_tag(Extract_tag const &) noexcept = default;
  constexpr auto operator=(Extract_tag const &) noexcept
      -> Extract_tag & = default;
  constexpr Extract_tag(Extract_tag &&) noexcept = default;
  constexpr auto operator=(Extract_tag &&) noexcept -> Extract_tag & = default;
};

template <std::copyable R> class Compute_io {
public:
  using return_type = R;
  virtual auto operator()() const -> return_type = 0;
  virtual ~Compute_io() noexcept = default;
  Compute_io(Compute_io<R> const &) = delete;
  auto operator=(Compute_io<R> const &) = delete;
  Compute_io(Compute_io<R> &&) = delete;
  auto operator=(Compute_io<R> &&) = delete;

protected:
  Compute_io() noexcept = default;
};

template <std::copyable R, typename... Args>
class Compute_in<R(Args...)>
    : public Compute_io<R>,
      public std::enable_shared_from_this<Compute_in<R(Args...)>> {
public:
  using return_type = typename Compute_io<R>::return_type;
  using signature_type = return_type(Args...);
  using std::enable_shared_from_this<
      Compute_in<signature_type>>::shared_from_this;
  using std::enable_shared_from_this<
      Compute_in<signature_type>>::weak_from_this;

private:
  std::function<signature_type> const function_;
  std::mutex mutable mutex_{};
  std::packaged_task<return_type()> mutable task_;
  std::shared_future<return_type> mutable future_;
  bool mutable invoked_{false};

protected:
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  explicit Compute_in(std::function<signature_type> function,
                      ForwardArgs &&...args)
      : Compute_in{true, function, std::forward<ForwardArgs>(args)...} {}
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  Compute_in(bool defer, std::function<signature_type> function,
             ForwardArgs &&...args)
      : function_{std::move(function)},
        task_{[this, ... args = std::forward<ForwardArgs>(args)]() mutable {
          return function_(std::forward<ForwardArgs>(args)...);
        }},
        future_{task_.get_future()} {
    if (!defer) {
      invoke();
    }
  }

public:
  template <typename... ForwardArgs> static auto create(ForwardArgs &&...args) {
    return std::shared_ptr<Compute_in<signature_type>>{
        new Compute_in{std::forward<ForwardArgs>(args)...}};
  }
  auto invoke() const {
    std::lock_guard<std::mutex> const guard{mutex_};
    if (!invoked_) {
      task_();
      invoked_ = true;
    }
    return std::shared_future{future_}.get();
  }
  template <bool Defer = true, typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  auto bind(ForwardArgs &&...args) -> std::optional<return_type> {
    std::lock_guard<std::mutex> const guard{mutex_};
    task_ = [this, ... args = std::forward<ForwardArgs>(args)]() mutable {
      return function_(std::forward<ForwardArgs>(args)...);
    };
    future_ = task_.get_future();
    if constexpr (Defer) {
      invoked_ = false;
      return {};
    } else {
      task_();
      invoked_ = true;
      return std::shared_future{future_}.get();
    }
  }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  auto bind(bool defer, ForwardArgs &&...args) {
    return defer ? bind<true>(std::forward<ForwardArgs>(args)...)
                 : bind<false>(std::forward<ForwardArgs>(args)...);
  }
  template <bool Defer = true> auto reset() -> std::optional<return_type> {
    std::lock_guard<std::mutex> const guard{mutex_};
    task_.reset();
    future_ = task_.get_future();
    if constexpr (Defer) {
      invoked_ = false;
      return {};
    } else {
      task_();
      invoked_ = true;
      return std::shared_future{future_}.get();
    }
  }
  auto reset(bool defer) { return defer ? reset<true>() : reset<false>(); }
  auto out [[nodiscard]] () const { return Compute_out{*this}; }

  auto operator()() const -> return_type override { return invoke(); }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  void operator<<(ForwardArgs &&...args) {
    bind<true>(std::forward<ForwardArgs>(args)...);
  }
  template <typename... ForwardArgs>
  requires std::invocable<std::function<signature_type>, ForwardArgs...>
  auto operator<<=(ForwardArgs &&...args) {
    return bind<false>(std::forward<ForwardArgs>(args)...).value();
  }
  void operator<<([[maybe_unused]] Reset_tag /*unused*/) { reset<true>(); }
  auto operator<<=([[maybe_unused]] Reset_tag /*unused*/) {
    return reset<false>().value();
  }

  ~Compute_in() noexcept override = default;
  Compute_in(Compute_in<R(Args...)> const &) = delete;
  auto operator=(Compute_in<R(Args...)> const &) = delete;
  Compute_in(Compute_in<R(Args...)> &&) = delete;
  auto operator=(Compute_in<R(Args...)> &&) = delete;
};

template <std::copyable R, typename... Args>
explicit Compute_in(std::function<R(Args...)> function, auto &&...args)
    -> Compute_in<R(Args...)>;
template <std::copyable R, typename... Args>
Compute_in(bool, std::function<R(Args...)> function, auto &&...args)
    -> Compute_in<R(Args...)>;

template <std::copyable R> class Compute_out : private Compute_io<R> {
public:
  using return_type = typename Compute_io<R>::return_type;

private:
  std::weak_ptr<Compute_io<R> const> in_;
  return_type return_;

public:
  template <typename... Args>
  explicit Compute_out(Compute_in<return_type(Args...)> const &in)
      : in_{std::static_pointer_cast<Compute_io<return_type> const>(
            in.shared_from_this())},
        return_{in()} {}

  auto get [[nodiscard]] () const
      noexcept(noexcept(return_) &&
               std::is_nothrow_move_constructible_v<return_type>) {
    return return_;
  }
  auto extract() {
    if (auto const in{in_.lock()}) {
      return_ = (*in)();
    }
    return return_;
  }

  auto operator() [[nodiscard]] () const
      noexcept(noexcept(get()) &&
               std::is_nothrow_move_constructible_v<return_type>)
          -> return_type override {
    return get();
  }
  auto operator()([[maybe_unused]] Extract_tag /*unused*/) { return extract(); }
  auto operator>>(return_type &right) noexcept(
      noexcept(right = get()) &&
      std::is_nothrow_move_constructible_v<return_type>) {
    return right = get();
  }
  auto operator>>=(return_type &right) { return right = extract(); }

  ~Compute_out() noexcept override = default;
  void swap(Compute_out<return_type> &other) noexcept {
    using std::swap;
    swap(in_, other.in_);
    swap(return_, other.return_);
  }
  Compute_out(Compute_out<R> const &other) noexcept(
      std::is_nothrow_copy_constructible_v<decltype(in_)>
          &&std::is_nothrow_copy_constructible_v<decltype(return_)>)
      : in_{other.in_}, return_{other.return_} {}
  auto operator=(Compute_out<R> const &right) noexcept(noexcept(
      Compute_out{right}.swap(*this)) &&noexcept(*this)) -> Compute_out<R> & {
    Compute_out{right}.swap(*this);
    return *this;
  };
  Compute_out(Compute_out<R> &&other) noexcept
      : in_{std::move(other.in_)}, return_{std::move(other.return_)} {}
  auto operator=(Compute_out<R> &&right) noexcept -> Compute_out<R> & {
    Compute_out{std::move(right)}.swap(*this);
    return *this;
  };
};

template <std::copyable R, typename... Args>
explicit Compute_out(Compute_in<R(Args...)> const &in) -> Compute_out<R>;
template <std::copyable R>
void swap(Compute_out<R> &left, Compute_out<R> &right) noexcept {
  left.swap(right);
}
} // namespace artccel::core::compute

#endif
