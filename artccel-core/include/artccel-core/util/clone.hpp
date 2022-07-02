#ifndef ARTCCEL_CORE_UTIL_CLONE_HPP
#define ARTCCEL_CORE_UTIL_CLONE_HPP
#pragma once

#include "meta.hpp" // import Replace_all_t, Replace_all_t_t, Replace_target
#include "utility_extras.hpp" // import dependent_false_v, f::unify_ptr_to_ref, f::unify_ref_to_ptr
#include <artccel-core/export.h> // import ARTCCEL_CORE_EXPORT_DECLARATION
#include <cassert>               // import assert
#include <concepts> // import std::constructible_from, std::convertible_to, std::derived_from, std::invocable, std::same_as
#include <functional> // import std::invoke
#include <memory> // import std::enable_shared_from_this, std::pointer_traits, std::shared_ptr, std::to_address, std::unique_pto_addresstr
#include <type_traits> // import std::invoke_result_t, std::is_lvalue_reference_v, std::is_rvalue_reference_v, std::is_pointer_v, std::is_reference_v, std::remove_cv_t, std::remove_pointer_t
#include <utility>     // import std::forward, std::move

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr_name, expr)                      \
  [](auto const &(ptr_name)) -> decltype(auto) {                               \
    constexpr static struct {                                                  \
      constexpr auto operator()                                                \
          [[nodiscard]] (decltype(ptr_name)(ptr_name)) const                   \
          -> decltype(auto) {                                                  \
        return expr;                                                           \
      }                                                                        \
    } func{};                                                                  \
    return func(ptr_name);                                                     \
  }
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION(ptr_name, expr)                        \
  ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr_name, expr)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ARTCCEL_GENERATE_CLONE_FUNCTION_EXPR(expr)                             \
  ARTCCEL_GENERATE_CLONE_FUNCTION_0(ptr, expr)

namespace artccel::core::util {
template <typename P, typename F>
concept Cloneable_by = requires(P const &ptr, F &&func) {
  requires std::convertible_to <
      std::remove_cv_t<std::remove_pointer_t<decltype(std::to_address(ptr))>>
  *, decltype(std::to_address(f::unify_ref_to_ptr(
         std::invoke(std::forward<F>(func), *std::to_address(ptr))))) > ;
};
template <typename F, typename P>
concept Cloner_of = Cloneable_by<P, F>;
template <Replace_target = Replace_target::self> struct Clone_auto_element_t {};
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION
    Clone_auto_element_t<Replace_target::self>;
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION
    Clone_auto_element_t<Replace_target::container>;
template <Replace_target = Replace_target::self> struct Clone_auto_deleter_t {};
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION
    Clone_auto_deleter_t<Replace_target::self>;
extern template struct ARTCCEL_CORE_EXPORT_DECLARATION
    Clone_auto_deleter_t<Replace_target::container>;
template <typename P>
constexpr auto default_clone_function{[]() noexcept {
  using ptr_element_type = typename std::pointer_traits<P>::element_type;
  constexpr struct {
    constexpr auto operator() [[nodiscard]] (ptr_element_type const &ptr) const
        -> decltype(auto) {
      // use std::invoke so that this function can be constexpr on MSVC
      return std::invoke(&ptr_element_type::clone, ptr);
    }
  } init{};
  return init;
}()};
template <typename P>
concept Cloneable_by_default_clone_function =
    Cloneable_by<P, decltype(default_clone_function<P>)>;

namespace detail {
template <typename P, Cloner_of<P> F>
constexpr auto clone_raw [[nodiscard]] (P const &ptr, F &&func) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  assert(ptr && u8"ptr == nullptr");
  auto ret{[&ptr, &func] {
    decltype(auto) init{
        std::invoke(std::forward<F>(func), *std::to_address(ptr))};
    // T/smart_pointer<T>, T*, T&, T&&
    static_assert(!std::is_rvalue_reference_v<decltype(init)>,
                  u8"Clone function should not return a xvalue");
    // T/smart_pointer<T> -> T/smart_pointer<T>, T* -> T*, T& -> T*; T&& -> T
    return f::unify_ref_to_ptr(std::forward<decltype(init)>(init));
  }()};

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  assert(ret && u8"ret == nullptr");
  using ret_type = decltype(ret);
  static_assert(!std::is_reference_v<ret_type>, u8"Unexpected");
  if constexpr (std::is_pointer_v<ret_type>) {
    return std::unique_ptr<std::remove_pointer_t<ret_type>>{ret};
  } else if constexpr (!std::same_as<decltype(ret.release()), void>) {
    using raw_type =
        std::remove_pointer_t<decltype(f::unify_ref_to_ptr(ret.release()))>;
    if constexpr (requires {
                    typename ret_type::deleter_type;
                    requires !std::same_as<decltype(ret.get_deleter()), void>;
                  }) {
      auto &&deleter{f::unify_ptr_to_ref(ret.get_deleter())};
      using deleter_type = typename ret_type::deleter_type;
      if constexpr (std::is_reference_v<deleter_type>) {
        static_assert(std::is_lvalue_reference_v<decltype(deleter)>,
                      u8"get_deleter() should return a lvalue or pointer");
        return std::unique_ptr<raw_type, deleter_type>{
            f::unify_ref_to_ptr(ret.release()), deleter};
      } else {
        auto const released{f::unify_ref_to_ptr(
            ret.release())}; // happens before stealing the deleter
        return std::unique_ptr<raw_type, deleter_type>{released,
                                                       std::move(deleter)};
      }
    } else {
      return std::unique_ptr<raw_type>{f::unify_ref_to_ptr(ret.release())};
    }
  } else {
    return ret; // fallback
  }
}
template <typename P, Cloner_of<P> F>
using clone_element_t = typename std::pointer_traits<
    std::invoke_result_t<decltype(clone_raw<P, F>), P, F>>::element_type;
template <typename P, Cloner_of<P> F>
using clone_deleter_t = typename std::invoke_result_t<decltype(clone_raw<P, F>),
                                                      P, F>::deleter_type;
template <typename P, Cloner_of<P> F,
          typename Return =
              Replace_all_t<P, clone_element_t<P, F>, Clone_auto_element_t<>>>
requires(
    !std::derived_from<clone_element_t<P, F>,
                       std::enable_shared_from_this<clone_element_t<P, F>>> ||
    std::derived_from<Replace_all_t_t<Return, Clone_auto_element_t, int>,
                      std::shared_ptr<int>>) constexpr auto clone
    [[nodiscard]] (P const &ptr, F &&func) -> decltype(auto) {
  using return_type = Replace_all_t_t<
      Replace_all_t_t<Return, Clone_auto_deleter_t, clone_deleter_t<P, F>>,
      Clone_auto_element_t, clone_element_t<P, F>>;
  auto ret{
      clone_raw(ptr, func)}; // std::shared_ptr<?>, std::unique_ptr<?, ?>, T
  if constexpr (std::is_reference_v<return_type>) {
    return return_type{*ret.release()};
  } else if constexpr (std::is_pointer_v<return_type>) {
    return return_type{ret.release()};
  } else if constexpr (std::constructible_from<return_type,
                                               decltype(std::move(ret))> ||
                       std::same_as<typename std::pointer_traits<
                                        decltype(ret)>::template rebind<int>,
                                    std::shared_ptr<int>>) {
    return return_type{std::move(ret)};
  } else {
    auto &deleter{
        ret.get_deleter()}; // std::unique_ptr<T, D>::get_deleter() -> D&
    constexpr auto is_deleter_reference{
        std::is_reference_v<clone_deleter_t<P, F>>};
    if constexpr (is_deleter_reference &&
                  std::constructible_from<return_type, decltype(ret.release()),
                                          decltype(deleter)>) {
      return return_type{ret.release(), deleter};
    } else if constexpr (is_deleter_reference &&
                         std::constructible_from<return_type,
                                                 decltype(*ret.release()),
                                                 decltype(deleter)>) {
      return return_type{*ret.release(), deleter};
    } else if constexpr (std::constructible_from<
                             return_type, decltype(ret.release()),
                             decltype(std::move(deleter))>) {
      return return_type{ret.release(), std::move(deleter)};
    } else if constexpr (std::constructible_from<
                             return_type, decltype(*ret.release()),
                             decltype(std::move(deleter))>) {
      return return_type{*ret.release(), std::move(deleter)};
    } else if constexpr (std::invocable<decltype(std::pointer_traits<
                                                 return_type>::pointer_to),
                                        decltype(*ret.release())>) {
      return std::pointer_traits<return_type>::pointer_to(*ret.release());
    } else if constexpr (std::constructible_from<return_type,
                                                 decltype(ret.release())>) {
      return return_type{ret.release()};
    } else if constexpr (std::constructible_from<return_type,
                                                 decltype(*ret.release())>) {
      return return_type{*ret.release()};
    } else {
      static_assert(dependent_false_v<return_type>,
                    u8"Cannot convert ret to return_type");
    }
  }
}
} // namespace detail

namespace f {
template <typename Return = void, typename P, Cloner_of<P> F>
constexpr auto clone [[nodiscard]] (P const &ptr, F &&func) -> decltype(auto) {
  if constexpr (std::same_as<Return, void>) {
    return detail::clone(ptr, std::forward<F>(func));
  } else {
    return detail::clone<P, F, Return>(ptr, std::forward<F>(func));
  }
}
template <typename Return = void, Cloneable_by_default_clone_function P>
constexpr auto clone [[nodiscard]] (P const &ptr) -> decltype(auto) {
  return clone<Return>(ptr, default_clone_function<P>);
}
template <typename RElement = Clone_auto_element_t<>,
          typename RDeleter = Clone_auto_deleter_t<>, typename P,
          Cloner_of<P> F>
constexpr auto clone_unique [[nodiscard]] (P const &ptr, F &&func)
-> decltype(auto) {
  return clone<std::unique_ptr<RElement, RDeleter>>(ptr, std::forward<F>(func));
}
template <typename RElement = Clone_auto_element_t<>,
          typename RDeleter = Clone_auto_deleter_t<>,
          Cloneable_by_default_clone_function P>
constexpr auto clone_unique [[nodiscard]] (P const &ptr) -> decltype(auto) {
  return clone_unique<RElement, RDeleter>(ptr, default_clone_function<P>);
}
template <typename RElement = Clone_auto_element_t<>, typename P,
          Cloner_of<P> F>
constexpr auto clone_shared [[nodiscard]] (P const &ptr, F &&func)
-> decltype(auto) {
  return clone<std::shared_ptr<RElement>>(ptr, std::forward<F>(func));
}
template <typename RElement = Clone_auto_element_t<>,
          Cloneable_by_default_clone_function P>
constexpr auto clone_shared [[nodiscard]] (P const &ptr) -> decltype(auto) {
  return clone_shared<RElement>(ptr, default_clone_function<P>);
}
} // namespace f
} // namespace artccel::core::util

#endif
