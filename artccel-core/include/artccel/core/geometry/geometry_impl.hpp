#pragma once
#ifndef GUARD_82E14629_C023_4D47_B2A3_D5E21CC0799C
#define GUARD_82E14629_C023_4D47_B2A3_D5E21CC0799C

#include <array>       // import std::array
#include <cstdint>     // import std::int_fast8_t
#include <tuple>       // import std::tuple
#include <type_traits> // import std::is_nothrow_copy_constructible_v
#include <typeinfo>    // import std::type_info
#include <utility>     // import std::swap

#include "geometry.hpp" // interface

#include "../util/clone.hpp" // import util::Cloneable, util::Cloneable_bases, util::Cloneable_impl
#include "../util/containers.hpp" // import util::Value_span, util::f::to_array
#include "../util/semantics.hpp"  // import util::Observer_ptr

namespace artccel::core {
namespace geometry::impl {
template <Dimension_t Dim> class Geometry_impl;
template <Dimension_t Dim> class Primitive_impl;
template <Dimension_t Dim> class Point_impl;
} // namespace geometry::impl

namespace util {
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace geometry;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace geometry::impl;

template <Dimension_t Dim> struct Cloneable_bases<Geometry_impl<Dim>> {
  using type = std::tuple<Geometry>;
};
template <Dimension_t Dim> struct Cloneable_bases<Primitive_impl<Dim>> {
  using type = std::tuple<Primitive, Geometry_impl<Dim>>;
};
template <Dimension_t Dim> struct Cloneable_bases<Point_impl<Dim>> {
  using type = std::tuple<Point, Primitive_impl<Dim>>;
  using impl_type = std::tuple<>;
};
} // namespace util

namespace geometry::impl {
template <Dimension_t Dim>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Geometry_impl : public virtual Geometry,
                      public virtual util::Cloneable<Geometry_impl<Dim>> {
public:
  // NOLINTNEXTLINE(fuchsia-statically-constructed-objects): constexpr ctor
  constexpr static auto dimension_{Dim};
  auto dimension [[nodiscard]] () const -> Dimension_t final { return Dim; }
  ~Geometry_impl() noexcept override = default;

protected:
#pragma warning(suppress : 4589)
  using Geometry::Geometry;
  void swap(Geometry_impl &other [[maybe_unused]]) noexcept { using std::swap; }
  Geometry_impl(Geometry_impl const &other [[maybe_unused]]) noexcept {}
  auto operator=(Geometry_impl const &right [[maybe_unused]]) noexcept(
      noexcept(Geometry_impl{right}.swap(*this), *this)) -> Geometry_impl & {
    Geometry_impl{right}.swap(*this);
    return *this;
  }
  Geometry_impl(Geometry_impl &&other [[maybe_unused]]) noexcept {}
  auto operator=(Geometry_impl &&right [[maybe_unused]]) noexcept
      -> Geometry_impl & {
    Geometry_impl{std::move(right)}.swap(*this);
    return *this;
  }
#pragma warning(suppress : 4250 4820)
};

template <Dimension_t Dim>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Primitive_impl : public virtual Primitive,
                       public virtual util::Cloneable<Primitive_impl<Dim>>,
                       public Geometry_impl<Dim> {
public:
  using Primitive_impl::Geometry_impl::dimension_;
  ~Primitive_impl() noexcept override = default;

protected:
#pragma warning(suppress : 4589)
  using Primitive_impl::Geometry_impl::Geometry_impl;
  void swap(Primitive_impl &other) noexcept {
    using std::swap;
    Primitive_impl::Geometry_impl::swap(other);
  }
  Primitive_impl(Primitive_impl const &other) noexcept(
      std::is_nothrow_copy_constructible_v<
          typename Primitive_impl::Geometry_impl>)
      : Primitive_impl::Geometry_impl{other} {}
  auto operator=(Primitive_impl const &right) noexcept(
      noexcept(Primitive_impl{right}.swap(*this), *this)) -> Primitive_impl & {
    Primitive_impl{right}.swap(*this);
    return *this;
  }
  Primitive_impl(Primitive_impl &&other) noexcept {
    using std::swap;
    Primitive_impl::Geometry_impl::swap(std::move(other));
  }
  auto operator=(Primitive_impl &&right) noexcept -> Primitive_impl & {
    Primitive_impl{std::move(right)}.swap(*this);
    return *this;
  }
#pragma warning(suppress : 4250 4820)
};

template <Dimension_t Dim>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Point_impl : public virtual Point,
                   public virtual util::Cloneable_impl<Point_impl<Dim>>,
                   public Primitive_impl<Dim> {
  friend util::Cloneable_impl<Point_impl>;

public:
  using Point_impl::Primitive_impl::dimension_;

private:
  std::array<compute::Compute_out<double>, Dim> position_{};

public:
  explicit Point_impl(
      util::Value_span<compute::Compute_out<double>, Dim> const &position)
      : position_{util::f::to_array(position)} {}
  ~Point_impl() noexcept override = default;

protected:
  auto try_get_quality [[nodiscard]] (std::type_info const &quality_type
                                      [[maybe_unused]]) noexcept
      -> util::Observer_ptr<Quality> override {
    return nullptr;
  }
  auto try_get_quality [[nodiscard]] (std::type_info const &quality_type
                                      [[maybe_unused]]) const noexcept
      -> util::Observer_ptr<Quality const> override {
    return nullptr;
  }

  void swap(Point_impl &other) noexcept {
    using std::swap;
    Point_impl::Primitive_impl::swap(other);
    swap(position_, other.position_);
  }
  Point_impl(Point_impl const &other) noexcept(
      std::is_nothrow_copy_constructible_v<typename Point_impl::Primitive_impl>
          &&noexcept(decltype(position_){other.position_}))
      : Point_impl::Primitive_impl{other}, position_{other.position_} {}
  auto operator=(Point_impl const &right) noexcept(
      noexcept(Point_impl{right}.swap(*this), *this)) -> Point_impl & {
    Point_impl{right}.swap(*this);
    return *this;
  }
  Point_impl(Point_impl &&other) noexcept
      : Point_impl::Primitive_impl{std::move(other)}, position_{std::move(
                                                          other.position_)} {}
  auto operator=(Point_impl &&right) noexcept -> Point_impl & {
    Point_impl{std::move(right)}.swap(*this);
    return *this;
  }
#pragma warning(suppress : 4250 4820)
};
} // namespace geometry::impl
} // namespace artccel::core

#endif
