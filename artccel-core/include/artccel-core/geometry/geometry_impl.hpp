#ifndef ARTCCEL_CORE_GEOMETRY_GEOMETRY_IMPL_HPP
#define ARTCCEL_CORE_GEOMETRY_GEOMETRY_IMPL_HPP
#pragma once

#include "../util/interval.hpp"  // import util::Nonnegative_interval
#include "../util/semantics.hpp" // import util::Observer_ptr
#include "geometry.hpp"          // interface
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
class Geometry : public virtual interface::Geometry {
public:
  // NOLINTNEXTLINE(fuchsia-statically-constructed-objects): constexpr ctor
  constexpr static auto dimension_{D};
  auto dimension [[nodiscard]] () const
      -> util::Nonnegative_interval<std::int8_t> final {
    return D;
  }
  ~Geometry() noexcept override = default;

protected:
  using interface::Geometry::Geometry;
  void swap(Geometry &other [[maybe_unused]]) noexcept { using std::swap; }
  Geometry(Geometry const &other [[maybe_unused]]) noexcept {};
  auto operator=(Geometry const &right [[maybe_unused]]) noexcept(
      noexcept(Geometry{right}.swap(*this), *this)) -> Geometry & {
    Geometry{right}.swap(*this);
    return *this;
  }
  Geometry(Geometry &&) noexcept = default;
  auto operator=(Geometry &&right [[maybe_unused]]) noexcept -> Geometry & {
    Geometry{std::move(right)}.swap(*this);
    return *this;
  }
};

template <util::Nonnegative_interval<std::int8_t> D>
class Primitive : public virtual interface::Primitive, public Geometry<D> {
public:
  using Primitive::Geometry::dimension_;
  ~Primitive() noexcept override = default;

protected:
  using Primitive::Geometry::Geometry;
  void swap(Primitive &other) noexcept {
    using std::swap;
    Primitive::Geometry::swap(other);
  }
  Primitive(Primitive const &other) noexcept(
      std::is_nothrow_copy_constructible_v<typename Primitive::Geometry>)
      : Primitive::Geometry{other} {};
  auto operator=(Primitive const &right) noexcept(
      noexcept(Primitive{right}.swap(*this), *this)) -> Primitive & {
    Primitive{right}.swap(*this);
    return *this;
  }
  Primitive(Primitive &&) noexcept = default;
  auto operator=(Primitive &&right) noexcept -> Primitive & {
    Primitive{std::move(right)}.swap(*this);
    return *this;
  }
};

template <util::Nonnegative_interval<std::int8_t> D>
class Point : public virtual interface::Point, public Primitive<D> {
public:
  using Point::Primitive::dimension_;

private:
  std::array<compute::Compute_out<double>, D> position_{};

public:
  explicit Point() noexcept = default;
  template <
      std::convertible_to<typename decltype(position_)::value_type>... Position>
  requires(sizeof...(Position) == D) explicit Point(Position... position)
      : position_{std::move(position)...} {}
  auto clone [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> gsl::owner<interface::Point *> override {
    return new Point{*this};
  }
  ~Point() noexcept override = default;

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

  void swap(Point &other) noexcept {
    using std::swap;
    Point::Primitive::swap(other);
    swap(position_, other.position_);
  }
  Point(Point const &other) noexcept(
      std::is_nothrow_copy_constructible_v<typename Point::Primitive>
          &&noexcept(decltype(position_){other.position_}))
      : Point::Primitive{other}, position_{other.position_} {}
  auto operator=(Point const &right) noexcept(noexcept(Point{right}.swap(*this),
                                                       *this)) -> Point & {
    Point{right}.swap(*this);
    return *this;
  }
  Point(Point &&other) noexcept
      : Point::Primitive{std::move(other)}, position_{
                                                std::move(other.position_)} {}
  auto operator=(Point &&right) noexcept -> Point & {
    Point{std::move(right)}.swap(*this);
    return *this;
  }
};
} // namespace artccel::core::geometry::impl

#endif
