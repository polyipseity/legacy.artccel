#pragma once
#ifndef GUARD_A112443D_3FC2_4920_B2D9_8EE0D25C37C1
#define GUARD_A112443D_3FC2_4920_B2D9_8EE0D25C37C1

#include <cassert>  // import assert
#include <concepts> // import std::constructible_from, std::convertible_to, std::derived_from, std::invocable, std::same_as
#include <functional> // import std::invoke
#include <memory> // import std::enable_shared_from_this, std::pointer_traits, std::shared_ptr, std::to_address, std::unique_ptr
#include <tuple>  // import std::tuple_cat
#include <type_traits> // import std::invoke_result_t, std::is_lvalue_reference_v, std::is_rvalue_reference_v, std::is_pointer_v, std::is_reference_v, std::remove_cv_t, std::remove_pointer_t
#include <utility>     // import std::declval, std::forward, std::move

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::owner
#pragma warning(pop)

#include "concepts_extras.hpp" // import Differ_from, Not_derived_from, Nonempty_pack
#include "meta.hpp" // import Replace_all_t, Replace_all_t_t, Replace_target, Transform_list_identity_t
#include "utility_extras.hpp" // import f::unify_ptr_to_ref, f::unify_ref_to_ptr
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT_DECLARATION

namespace artccel::core::util {
template <typename Type> struct Cloneable_bases;
template <typename Type, typename... Bases> class Cloneable_0;
template <typename Type>
using Cloneable = Transform_list_identity_t<
    decltype(std::tuple_cat(
        std::declval<std::tuple<Type>>(),
        std::declval<typename Cloneable_bases<Type>::type>())),
    Cloneable_0>;
template <typename Type, typename... Bases> class Cloneable_impl_0;
template <typename Type>
using Cloneable_impl = Transform_list_identity_t<
    decltype(std::tuple_cat(
        std::declval<std::tuple<Type>>(),
        std::declval<typename Cloneable_bases<Type>::impl_type>())),
    Cloneable_impl_0>;

template <typename Ptr, typename Func>
concept Cloneable_by = requires(Ptr const &ptr, Func &&func) {
  requires std::convertible_to <
      std::remove_cv_t<std::remove_pointer_t<decltype(std::to_address(ptr))>>
  *, decltype(std::to_address(f::unify_ref_to_ptr(
         std::invoke(std::forward<Func>(func), *std::to_address(ptr))))) > ;
};
template <typename Func, typename Ptr>
concept Cloner_of = Cloneable_by<Ptr, Func>;
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
template <typename Ptr>
constexpr auto default_clone_function{
    &std::pointer_traits<Ptr>::element_type::clone};
template <typename Ptr>
concept Cloneable_by_default_clone_function =
    Cloneable_by<Ptr, decltype(default_clone_function<Ptr>)>;

namespace detail {
template <typename Ptr, Cloner_of<Ptr> Func>
constexpr auto clone_raw [[nodiscard]] (Ptr const &ptr, Func &&func) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  assert(ptr && u8"ptr == nullptr");
  auto ret{[&ptr, &func] {
    decltype(auto) init{
        std::invoke(std::forward<Func>(func), *std::to_address(ptr))};
    // T/smart_pointer<T>, T*, T&, T&&
    static_assert(!std::is_rvalue_reference_v<decltype(init)>,
                  u8"Clone function should not return a xvalue");
    // T/smart_pointer<T> -> T/smart_pointer<T>, T* -> T*, T& -> T*; T&& -> T
    return f::unify_ref_to_ptr(std::forward<decltype(init)>(init));
  }()};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
  assert(ret && u8"ret == nullptr");

  using ret_type = decltype(ret);
  if constexpr (std::is_pointer_v<ret_type>) {
    return std::unique_ptr<std::remove_pointer_t<ret_type>>{ret};
  } else if constexpr (Differ_from<decltype(ret.release()), void>) {
    using raw_type =
        std::remove_pointer_t<decltype(f::unify_ref_to_ptr(ret.release()))>;
    if constexpr (requires {
                    typename ret_type::deleter_type;
                    { ret.get_deleter() } -> Differ_from<void>;
                  }) {
      auto &&deleter{f::unify_ptr_to_ref(ret.get_deleter())};
      using deleter_type = typename ret_type::deleter_type;
      if constexpr (std::is_reference_v<deleter_type>) {
        static_assert(std::is_lvalue_reference_v<decltype(deleter)>,
                      u8"get_deleter() should return a lvalue or pointer");
        return std::unique_ptr<raw_type, deleter_type>{
            f::unify_ref_to_ptr(ret.release()), deleter};
      } else {
        auto *const released{f::unify_ref_to_ptr(
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
template <typename Ptr, Cloner_of<Ptr> Func>
using clone_element_t = typename std::pointer_traits<std::invoke_result_t<
    decltype(clone_raw<Ptr, Func>), Ptr, Func>>::element_type;
template <typename Ptr, Cloner_of<Ptr> Func>
using clone_deleter_t =
    typename std::invoke_result_t<decltype(clone_raw<Ptr, Func>), Ptr,
                                  Func>::deleter_type;
template <typename Ptr, Cloner_of<Ptr> Func,
          typename Return = Replace_all_t<Ptr, clone_element_t<Ptr, Func>,
                                          Clone_auto_element_t<>>>
requires Not_derived_from<
    clone_element_t<Ptr, Func>,
    std::enable_shared_from_this<clone_element_t<Ptr, Func>>> ||
    std::derived_from<Replace_all_t_t<Return, Clone_auto_element_t, int>,
                      std::shared_ptr<int>>
constexpr auto clone [[nodiscard]] (Ptr const &ptr, Func &&func)
-> decltype(auto) {
  using return_type = Replace_all_t_t<
      Replace_all_t_t<Return, Clone_auto_deleter_t, clone_deleter_t<Ptr, Func>>,
      Clone_auto_element_t, clone_element_t<Ptr, Func>>;
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
        ret.get_deleter()}; // std::unique_ptr<Type, Del>::get_deleter() -> Del&
    constexpr auto is_deleter_reference{
        std::is_reference_v<clone_deleter_t<Ptr, Func>>};
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
    } else {
      static_assert(
          std::constructible_from<return_type, decltype(*ret.release())>,
          u8"Cannot convert ret to return_type");
      return return_type{*ret.release()};
    }
  }
}
} // namespace detail

namespace f {
template <typename Return = void, typename Ptr, Cloner_of<Ptr> Func>
constexpr auto clone [[nodiscard]] (Ptr const &ptr, Func &&func)
-> decltype(auto) {
  if constexpr (std::same_as<Return, void>) {
    return detail::clone(ptr, std::forward<Func>(func));
  } else {
    return detail::clone<Ptr, Func, Return>(ptr, std::forward<Func>(func));
  }
}
template <typename Return = void, Cloneable_by_default_clone_function Ptr>
constexpr auto clone [[nodiscard]] (Ptr const &ptr) -> decltype(auto) {
  return clone<Return>(ptr, default_clone_function<Ptr>);
}
template <typename RElement = Clone_auto_element_t<>,
          typename RDeleter = Clone_auto_deleter_t<>, typename Ptr,
          Cloner_of<Ptr> Func>
constexpr auto clone_unique [[nodiscard]] (Ptr const &ptr, Func &&func)
-> decltype(auto) {
  return clone<std::unique_ptr<RElement, RDeleter>>(ptr,
                                                    std::forward<Func>(func));
}
template <typename RElement = Clone_auto_element_t<>,
          typename RDeleter = Clone_auto_deleter_t<>,
          Cloneable_by_default_clone_function Ptr>
constexpr auto clone_unique [[nodiscard]] (Ptr const &ptr) -> decltype(auto) {
  return clone_unique<RElement, RDeleter>(ptr, default_clone_function<Ptr>);
}
template <typename RElement = Clone_auto_element_t<>, typename Ptr,
          Cloner_of<Ptr> Func>
constexpr auto clone_shared [[nodiscard]] (Ptr const &ptr, Func &&func)
-> decltype(auto) {
  return clone<std::shared_ptr<RElement>>(ptr, std::forward<Func>(func));
}
template <typename RElement = Clone_auto_element_t<>,
          Cloneable_by_default_clone_function Ptr>
constexpr auto clone_shared [[nodiscard]] (Ptr const &ptr) -> decltype(auto) {
  return clone_shared<RElement>(ptr, default_clone_function<Ptr>);
}
} // namespace f

template <typename Type> class Cloneable_0<Type> {
public:
  constexpr auto clone [[nodiscard]] () const {
    return std::unique_ptr<Type>{static_cast<gsl::owner<Type *>>(clone_impl())};
  }
  constexpr virtual ~Cloneable_0() noexcept = default;
  Cloneable_0(Cloneable_0 const &) = delete;
  auto operator=(Cloneable_0 const &) = delete;
  Cloneable_0(Cloneable_0 &&) = delete;
  auto operator=(Cloneable_0 &&) = delete;

protected:
  constexpr Cloneable_0() noexcept = default;

private:
  constexpr virtual auto clone_impl [[nodiscard]] () const
      -> gsl::owner<void *> = 0;
};
template <typename Type, typename... Bases>
requires Nonempty_type_pack<Bases...>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Cloneable_0<Type, Bases...> : public virtual Cloneable<Bases>... {
public:
  constexpr auto clone [[nodiscard]] () const {
    return std::unique_ptr<Type>{static_cast<gsl::owner<Type *>>(clone_impl())};
  }

protected:
  using Cloneable<Bases>::Cloneable...;

private:
  constexpr auto clone_impl [[nodiscard]] () const
      -> gsl::owner<void *> override = 0;
#pragma warning(suppress : 4626 5027)
};
template <typename Type, typename... Bases>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Cloneable_impl_0 : public virtual Cloneable<Type>,
                         public virtual Cloneable_impl<Bases>... {
public:
  using Cloneable<Type>::clone;

protected:
  using Cloneable<Type>::Cloneable;
  using Cloneable_impl<Bases>::Cloneable_impl...;

private:
  auto clone_impl [[nodiscard]] () const
#pragma warning(suppress : 4437)
      -> gsl::owner<void *> override {
    return new Type{dynamic_cast<Type const &>(*this)};
  }
#pragma warning(suppress : 4625 4626 5026 5027)
};
} // namespace artccel::core::util

#endif
