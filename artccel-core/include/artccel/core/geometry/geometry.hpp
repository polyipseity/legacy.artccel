#ifndef GUARD_6933EE58_E1D0_488A_9D33_8E0F8B10533C
#define GUARD_6933EE58_E1D0_488A_9D33_8E0F8B10533C
#pragma once

#include <concepts> // import std::derived_from
#include <cstdint>  // import std::int_fast8_t
#include <tuple>    // import std::tuple
#include <typeinfo> // import typeid, std::type_info

#include "../util/clone.hpp" // import util::Cloneable, util::Cloneable_bases
#include "../util/contracts.hpp" // import util::Validate
#include "../util/interval.hpp"  // import util::nonnegative_interval
#include "../util/semantics.hpp" // import util::Observer_ptr
#include <artccel/core/export.h> // import ARTCCEL_CORE_EXPORT

namespace artccel::core {
namespace geometry {
using Dimension_t =
    util::Validate<util::nonnegative_interval<std::int_fast8_t>>;

class ARTCCEL_CORE_EXPORT Geometry;
class ARTCCEL_CORE_EXPORT Primitive;
class ARTCCEL_CORE_EXPORT Point;

class ARTCCEL_CORE_EXPORT Quality;
} // namespace geometry

namespace util {
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace geometry;

template <> struct Cloneable_bases<Geometry> { using type = std::tuple<>; };
template <> struct Cloneable_bases<Primitive> {
  using type = std::tuple<Geometry>;
};
template <> struct Cloneable_bases<Point> {
  using type = std::tuple<Primitive>;
};
} // namespace util

namespace geometry {
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

#pragma warning(suppress : 4435)
class Geometry : public virtual util::Cloneable<Geometry> {
public:
  virtual auto dimension [[nodiscard]] () const -> Dimension_t = 0;
  template <std::derived_from<Quality> Qly>
  auto try_get_quality [[nodiscard]] () {
    return dynamic_cast<util::Observer_ptr<Qly>>(try_get_quality(typeid(Qly)));
  }
  template <std::derived_from<Quality> Qly>
  auto try_get_quality [[nodiscard]] () const {
    return dynamic_cast<util::Observer_ptr<Qly const>>(
        try_get_quality(typeid(Qly)));
  }
  ~Geometry() noexcept override;
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

#pragma warning(suppress : 4435) // NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Primitive : public virtual Geometry,
#pragma warning(suppress : 4435)
                  public virtual util::Cloneable<Primitive> {
public:
  ~Primitive() noexcept override;
  Primitive(Primitive const &) = delete;
  auto operator=(Primitive const &) = delete;
  Primitive(Primitive &&) = delete;
  auto operator=(Primitive &&) = delete;

protected:
#pragma warning(suppress : 4589)
  using Geometry::Geometry;
#pragma warning(suppress : 4250)
};

#pragma warning(suppress : 4435) // NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Point : public virtual Primitive, public virtual util::Cloneable<Point> {
public:
  ~Point() noexcept override;
  Point(Point const &) = delete;
  auto operator=(Point const &) = delete;
  Point(Point &&) = delete;
  auto operator=(Point &&) = delete;

protected:
#pragma warning(suppress : 4589)
  using Primitive::Primitive;
#pragma warning(suppress : 4250)
};
} // namespace geometry
} // namespace artccel::core

#endif
