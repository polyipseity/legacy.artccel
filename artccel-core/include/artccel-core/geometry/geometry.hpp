#ifndef ARTCCEL_CORE_GEOMETRY_GEOMETRY_HPP
#define ARTCCEL_CORE_GEOMETRY_GEOMETRY_HPP
#pragma once

#include "../util/interval.hpp"  // import util::Nonnegative_interval
#include "../util/semantics.hpp" // import util::Observer_ptr
#include <cinttypes>             // import std::int8_t
#include <concepts>              // import std::derived_from
#include <gsl/gsl>               // import gsl::owner
#include <typeinfo>              // import typeid, std::type_info

namespace artccel::core::geometry {
class Geometry;
class Primitive;
class Point;

class Quality;

class Quality {
public:
  virtual ~Quality() noexcept = default;
  Quality(Quality const &) = delete;
  auto operator=(Quality const &) = delete;
  Quality(Quality &&) = delete;
  auto operator=(Quality &&) = delete;

protected:
  explicit Quality() noexcept = default;
};

class Geometry {
public:
  virtual auto dimension [[nodiscard]] () const
      -> util::Nonnegative_interval<std::int8_t> = 0;
  template <std::derived_from<Quality> T>
  auto try_get_quality [[nodiscard]] () {
    return dynamic_cast<util::Observer_ptr<T>>(try_get_quality(typeid(T)));
  }
  template <std::derived_from<Quality> T>
  auto try_get_quality [[nodiscard]] () const {
    return dynamic_cast<util::Observer_ptr<T const>>(
        try_get_quality(typeid(T)));
  }
  virtual auto clone [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> gsl::owner<Geometry *> = 0;
  virtual ~Geometry() noexcept = default;
  Geometry(Geometry const &) = delete;
  auto operator=(Geometry const &) = delete;
  Geometry(Geometry &&) = delete;
  auto operator=(Geometry &&) = delete;

protected:
  explicit Geometry() noexcept = default;
  virtual auto try_get_quality
      [[nodiscard]] (std::type_info const &quality_type)
      -> util::Observer_ptr<Quality> = 0;
  virtual auto try_get_quality
      [[nodiscard]] (std::type_info const &quality_type) const
      -> util::Observer_ptr<Quality const> = 0;
};

class Primitive : public virtual Geometry {
public:
  auto clone [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> gsl::owner<Primitive *> override = 0;
  ~Primitive() noexcept override = default;
  Primitive(Primitive const &) = delete;
  auto operator=(Primitive const &) = delete;
  Primitive(Primitive &&) = delete;
  auto operator=(Primitive &&) = delete;

protected:
  using Geometry::Geometry;
};

class Point : public virtual Primitive {
public:
  auto clone [[deprecated(/*u8*/ "Unsafe"), nodiscard]] () const
      -> gsl::owner<Point *> override = 0;
  ~Point() noexcept override = default;
  Point(Point const &) = delete;
  auto operator=(Point const &) = delete;
  Point(Point &&) = delete;
  auto operator=(Point &&) = delete;

protected:
  using Primitive::Primitive;
};
} // namespace artccel::core::geometry

#endif
