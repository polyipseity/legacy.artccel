#ifndef ARTCCEL_CORE_GEOMETRY_GEOMETRY_IMPL_HPP
#define ARTCCEL_CORE_GEOMETRY_GEOMETRY_IMPL_HPP
#pragma once

#include "geometry.hpp" // interface

#include "../util/interval.hpp"  // import util::Nonnegative_interval
#include "../util/semantics.hpp" // import util::Observer_ptr
#include <array>                 // import std::array
#include <cinttypes>             // import std::int8_t
#include <concepts>              // import std::convertible_to
#include <gsl/gsl>               // import gsl::owner
#include <type_traits>           // import std::is_nothrow_copy_constructible_v
#include <typeinfo>              // import std::type_info
#include <utility>               // import std::swap

namespace artccel::core::geometry::impl {
namespace interface = geometry;

template <util::Nonnegative_interval<std::int8_t> D>
class Geometry_impl : public virtual interface::Geometry {
public:
  // NOLINTNEXTLINE(fuchsia-statically-constructed-objects): constexpr ctor
  constexpr static auto dimension_{D};
  auto dimension [[nodiscard]] () const
      -> util::Nonnegative_interval<std::int8_t> final {
    return D;
  }
  ~Geometry_impl() noexcept override = default;

protected:
  using interface::Geometry::Geometry;
  void swap(Geometry_impl &other [[maybe_unused]]) noexcept { using std::swap; }
  Geometry_impl(Geometry_impl const &other [[maybe_unused]]) noexcept {};
  auto operator=(Geometry_impl const &right [[maybe_unused]]) noexcept(
      noexcept(Geometry_impl{right}.swap(*this), *this)) -> Geometry_impl & {
    Geometry_impl{right}.swap(*this);
    return *this;
  }
  Geometry_impl(Geometry_impl &&) noexcept = default;
  auto operator=(Geometry_impl &&right [[maybe_unused]]) noexcept
      -> Geometry_impl & {
    Geometry_impl{std::move(right)}.swap(*this);
    return *this;
  }
};

template <util::Nonnegative_interval<std::int8_t> D>
class Primitive_impl : public virtual interface::Primitive,
                       public Geometry_impl<D> {
public:
  using Primitive_impl::Geometry_impl::dimension_;
  ~Primitive_impl() noexcept override = default;

protected:
  using Primitive_impl::Geometry_impl::Geometry_impl;
  void swap(Primitive_impl &other) noexcept {
    using std::swap;
    Primitive_impl::Geometry_impl::swap(other);
  }
  Primitive_impl(Primitive_impl const &other) noexcept(
      std::is_nothrow_copy_constructible_v<
          typename Primitive_impl::Geometry_impl>)
      : Primitive_impl::Geometry_impl{other} {};
  auto operator=(Primitive_impl const &right) noexcept(
      noexcept(Primitive_impl{right}.swap(*this), *this)) -> Primitive_impl & {
    Primitive_impl{right}.swap(*this);
    return *this;
  }
  Primitive_impl(Primitive_impl &&) noexcept = default;
  auto operator=(Primitive_impl &&right) noexcept -> Primitive_impl & {
    Primitive_impl{std::move(right)}.swap(*this);
    return *this;
  }
};

template <util::Nonnegative_interval<std::int8_t> D>
class Point_impl : public virtual interface::Point, public Primitive_impl<D> {
public:
  using Point_impl::Primitive_impl::dimension_;

private:
  std::array<compute::Compute_out<double>, D> position_{};

public:
  explicit Point_impl() noexcept = default;
  template <
      std::convertible_to<typename decltype(position_)::value_type>... Position>
  requires(sizeof...(Position) == D) explicit Point_impl(Position... position)
      : position_{{std::move(position)...}} {}
  auto clone [[nodiscard]] () const -> gsl::owner<Point_impl *> override {
    return new Point_impl{*this};
  }
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
};
} // namespace artccel::core::geometry::impl

#endif
