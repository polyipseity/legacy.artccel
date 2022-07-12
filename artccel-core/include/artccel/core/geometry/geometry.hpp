#ifndef ARTCCEL_CORE_GEOMETRY_GEOMETRY_HPP
#define ARTCCEL_CORE_GEOMETRY_GEOMETRY_HPP
#pragma once

#include <concepts> // import std::derived_from
#include <cstdint>  // import std::int_fast8_t
#include <typeinfo> // import typeid, std::type_info

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::owner
#pragma warning(pop)

#include "../util/interval.hpp"  // import util::Nonnegative_interval
#include "../util/semantics.hpp" // import util::Observer_ptr
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core::geometry {
class ARTCCEL_CORE_EXPORT Geometry;
class ARTCCEL_CORE_EXPORT Primitive;
class ARTCCEL_CORE_EXPORT Point;

class ARTCCEL_CORE_EXPORT Quality;

class Quality {
public:
  virtual ~Quality() noexcept;
  Quality(Quality const &) = delete;
  auto operator=(Quality const &) = delete;
  Quality(Quality &&) = delete;
  auto operator=(Quality &&) = delete;

protected:
  Quality() noexcept;
};

class Geometry {
public:
  virtual auto dimension [[nodiscard]] () const
      -> util::Nonnegative_interval<std::int_fast8_t> = 0;
  template <std::derived_from<Quality> Qly>
  auto try_get_quality [[nodiscard]] () {
    return dynamic_cast<util::Observer_ptr<Qly>>(try_get_quality(typeid(Qly)));
  }
  template <std::derived_from<Quality> Qly>
  auto try_get_quality [[nodiscard]] () const {
    return dynamic_cast<util::Observer_ptr<Qly const>>(
        try_get_quality(typeid(Qly)));
  }
  auto clone [[nodiscard]] () const -> gsl::owner<Geometry *>;
  virtual ~Geometry() noexcept;
  Geometry(Geometry const &) = delete;
  auto operator=(Geometry const &) = delete;
  Geometry(Geometry &&) = delete;
  auto operator=(Geometry &&) = delete;

protected:
  Geometry() noexcept;
  virtual auto try_get_quality
      [[nodiscard]] (std::type_info const &quality_type)
      -> util::Observer_ptr<Quality> = 0;
  virtual auto try_get_quality
      [[nodiscard]] (std::type_info const &quality_type) const
      -> util::Observer_ptr<Quality const> = 0;
};

#pragma warning(suppress : 4435)
class Primitive : public virtual Geometry {
public:
  auto clone [[nodiscard]] () const -> gsl::owner<Primitive *>;
  ~Primitive() noexcept override;
  Primitive(Primitive const &) = delete;
  auto operator=(Primitive const &) = delete;
  Primitive(Primitive &&) = delete;
  auto operator=(Primitive &&) = delete;

protected:
#pragma warning(suppress : 4589)
  using Geometry::Geometry;
};

#pragma warning(suppress : 4435) // NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Point : public virtual Primitive {
public:
  auto clone [[nodiscard]] () const -> gsl::owner<Point *>;
  ~Point() noexcept override;
  Point(Point const &) = delete;
  auto operator=(Point const &) = delete;
  Point(Point &&) = delete;
  auto operator=(Point &&) = delete;

protected:
#pragma warning(suppress : 4589)
  using Primitive::Primitive;
};
} // namespace artccel::core::geometry

#endif
