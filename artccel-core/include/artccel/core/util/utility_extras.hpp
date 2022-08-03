#pragma once
#ifndef GUARD_9E8CA71A_8CBD_403E_BE77_D755133577B9
#define GUARD_9E8CA71A_8CBD_403E_BE77_D755133577B9

#include <array>   // import std::array
#include <cassert> // import assert
#include <concepts> // import std::copy_constructible, std::invocable, std::move_constructible
#include <cstddef>     // import std::size_t
#include <functional>  // import std::invoke
#include <memory>      // import std::addressof
#include <type_traits> // import std::is_lvalue_reference_v, std::is_pointer_v, std::remove_cvref_t, std::remove_reference_t
#include <utility> // import std::forward, std::index_sequence, std::index_sequence_for, std::move

namespace artccel::core::util {
template <typename Type> struct Semiregularize;
template <typename Type, bool Explicit = true> struct Delegate;
template <typename... Types> struct Overload;
enum struct Consteval_t : bool {};
enum struct Initialize_t : bool {};

namespace detail {
template <template <typename...> typename Tuple, typename... Args,
          std::invocable<Args...> Func, std::size_t... Idxs>
constexpr auto forward_apply_0(
    Func &&func, Tuple<Args &&...> &&t_args,
    std::index_sequence<Idxs...> idxs
    [[maybe_unused]]) noexcept(noexcept(std::invoke(std::forward<Func>(func),
                                                    std::forward<
                                                        Args>(std::get<Idxs>(
                                                        std::forward<
                                                            Tuple<Args &&...>>(
                                                            t_args)))...)))
    -> decltype(auto) {
  return std::invoke(std::forward<Func>(func),
                     std::forward<Args>(std::get<Idxs>(
                         std::forward<Tuple<Args &&...>>(t_args)))...);
}
} // namespace detail

namespace f {
template <typename Type>
constexpr auto unify_ref_to_ptr(Type &&value) noexcept {
  // (callsite) -> (return)
  if constexpr (std::is_lvalue_reference_v<Type>) {
    // t& -> t*, t*& -> t**
    return std::addressof(value);
  } else {
    // t -> t, t&& -> t, t* -> t*, t*&& -> t*
    return std::forward<Type>(value);
  }
}
template <typename Type>
constexpr auto unify_ptr_to_ref(Type &&value) noexcept -> decltype(auto) {
  // (callsite) -> (return)
  if constexpr (std::is_pointer_v<std::remove_cvref_t<Type>>) {
    // t* -> t&, t*& -> t&, t*&& -> t&
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
    assert(value && u8"value == nullptr");
    return *value; // *v is t&
  } else {
    // t -> t, t& -> t&, t&& -> t
    return static_cast<Type>(value);
  }
}

template <template <typename...> typename Tuple, typename... Args,
          std::invocable<Args...> Func>
constexpr auto forward_apply(Func &&func, Tuple<Args &&...> &&t_args) noexcept(
    noexcept(detail::forward_apply_0(std::forward<Func>(func),
                                     std::forward<Tuple<Args &&...>>(t_args),
                                     std::index_sequence_for<Args...>{})))
    -> decltype(auto) {
  return detail::forward_apply_0(std::forward<Func>(func),
                                 std::forward<Tuple<Args &&...>>(t_args),
                                 std::index_sequence_for<Args...>{});
}
} // namespace f

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <typename Type> struct Semiregularize : public Type {
#pragma clang diagnostic pop
  using Type::Type;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsuggest-destructor-override"
  // NOLINTNEXTLINE(hicpp-use-override,modernize-use-override)
  constexpr ~Semiregularize() noexcept = default;
#pragma clang diagnostic pop
  constexpr Semiregularize(Semiregularize const &) = default;
  constexpr auto operator=(Semiregularize const &)
      -> Semiregularize & = default;
  constexpr Semiregularize(Semiregularize &&) noexcept = default;
  constexpr auto operator=(Semiregularize &&) noexcept
      -> Semiregularize & = default;
#pragma warning(suppress : 4625 4626 5026 5027)
};

template <typename Type, bool Explicit> struct Delegate {
  using type = Type;
  // public members to be a structual type
  Type value_; // NOLINT(misc-non-private-member-variables-in-classes)

  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator auto &() &noexcept {
    return value_;
  }
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator auto const &() const &noexcept {
    return value_;
  }
  template <typename = void>
  requires std::move_constructible<Type>
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator Type() &&noexcept(noexcept(Type{std::move(value_)})) {
    return std::move(value_);
  }
  template <typename = void>
  requires std::copy_constructible<Type>
  [[nodiscard]] explicit(Explicit) constexpr
  // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
  operator Type() const &&noexcept(noexcept(Type{value_})) {
    return value_;
  }

protected:
  explicit constexpr Delegate(Initialize_t tag [[maybe_unused]],
                              Type &&value) noexcept(noexcept(decltype(value_){
      std::move(value)}))
      : value_{std::move(value)} {}
#pragma warning(suppress : 4625 4626)
};

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
template <typename... Types> struct Overload : Types... {
  using Types::operator()...;
};
template <typename... Types>
Overload(Types &&...args) -> Overload<std::remove_reference_t<Types>...>;
} // namespace artccel::core::util

#endif
