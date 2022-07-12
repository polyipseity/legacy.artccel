#include <algorithm> // import std::max, std::min, std::ranges::for_each, std::ranges::transform
#include <cassert>    // import assert
#include <concepts>   // import std::integral, std::same_as
#include <cstring>    // import std::memcpy, std::memmove, std::size_t
#include <cwchar>     // import std::mbsinit, std::mbstate_t, std::wcslen
#include <exception>  // import std::current_exception, std::exception_ptr
#include <functional> // import std::function
#include <ios> // import std::ios_base::openmode, std::ios_base::seekdir, std::streamsize
#include <iostream> // import std::cin, std::clog, std::cout, std::ios_base::sync_with_stdio
#include <locale> // import std::codecvt_base::result, std::locale, std::locale::global
#include <memory> // import std::make_shared, std::make_unique, std::make_unique_for_overwrite, std::unique_ptr, std::weak_ptr
#include <span> // import std::begin, std::data, std::empty, std::size, std::span
#include <streambuf>   // import std::streambuf
#include <string>      // import std::u16string, std::u8string
#include <string_view> // import std::string_view, std::u8string_view
#include <utility>     // import std::move
#include <vector>      // import std::vector

#pragma warning(push)
#pragma warning(disable : 4626 4820)
#include <gsl/gsl> // import gsl::cwzstring, gsl::czstring, gsl::final_action, gsl::not_null
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4582 4583 4625 4626 4820 5026 5027)
#include <tl/expected.hpp> // import tl::expected
#pragma warning(pop)
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4668 5039)
#include <windows.h> // import ::FlushConsoleInputBuffer, ::GetConsoleCP, ::GetConsoleMode, ::GetConsoleOutputCP, ::GetStdHandle, ::ReadConsoleW, ::SetConsoleCP, ::SetConsoleOutputCP, CP_UTF8, DWORD, HANDLE, INVALID_HANDLE_VALUE, STD_INPUT_HANDLE
#pragma warning(pop)
#endif

#include <artccel/core/main_hooks.hpp> // interface

#include <artccel/core/export.h> // import ARTCCEL_CORE_NO_EXPORT
#include <artccel/core/platform/windows_error.hpp> // import platform::windows::f::throw_last_error, platform::windows::f::print_last_error
#include <artccel/core/util/codecvt_extras.hpp> // import util::Codecvt_utf16_utf8
#include <artccel/core/util/containers_extras.hpp> // import util::f::atad, util::f::const_span
#include <artccel/core/util/conversions.hpp> // import util::f::int_clamp_cast, util::f::int_modulo_cast, util::f::int_unsigned_cast, util::f::int_unsigned_clamp_cast, util::f::int_unsigned_exact_cast
#include <artccel/core/util/encoding.hpp> // import util::f::loc_enc_to_utf8, util::f::utf16_to_utf8
#include <artccel/core/util/error_handling.hpp> // import util::Exception_error, util::f::expect_noninvalid, util::f::expect_nonzero
#include <artccel/core/util/polyfill.hpp>       // import util::f::unreachable
#include <artccel/core/util/utility_extras.hpp> // import util::Semiregularize

namespace artccel::core {
namespace detail {
// NOLINTNEXTLINE(readability-function-cognitive-complexity): 26/25
static auto run_finalizer_save_excepts(
    std::vector<Main_program::copyable_finalizer_type> finalizers,
    std::weak_ptr<Main_program::destructor_exceptions_out_type>
        dtor_excs_out) noexcept {
  return [finalizers{std::move(finalizers)},
          dtor_excs_out{std::move(dtor_excs_out)}]() mutable noexcept {
    if (auto const dtor_excs{dtor_excs_out.lock()}) {
      std::ranges::for_each(finalizers, [&dtor_excs](auto &finalizer) noexcept {
        try {
          // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
          assert(finalizer.use_count() == 1 && u8"Non-unique finalizer");
          finalizer.reset();
        } catch (...) {
          try {
            dtor_excs->emplace_back(std::current_exception());
          } catch (...) {
            // NOOP
          }
        }
      });
    } else {
      std::ranges::for_each(finalizers, [](auto &finalizer) noexcept {
        try {
          // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
          assert(finalizer.use_count() == 1 && u8"Non-unique finalizer");
          finalizer.reset();
        } catch (...) {
          // NOOP
        }
      });
    }
  };
}

#ifdef _WIN32
// TODO: await https://github.com/microsoft/terminal/issues/7777
class ARTCCEL_CORE_NO_EXPORT Windows_console_input_buffer
    : public std::streambuf {
public:
  using codecvt_type = util::Semiregularize<util::Codecvt_utf16_utf8>;
  constexpr static std::size_t default_buffer_code_point_size_{
      4096}; // TODO: C++23: UZ

private:
  std::unique_ptr<codecvt_type> codecvt_{std::make_unique<codecvt_type>()};
  std::mbstate_t mbstate_{};

protected:
  auto codecvt_max_length() noexcept -> decltype(assert_success(
      util::f::int_unsigned_exact_cast(codecvt_->max_length()))) {
    return assert_success(
        util::f::int_unsigned_exact_cast(codecvt_->max_length()));
  }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
  std::unique_ptr<char_type[]> default_buffer_{
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
      std::make_unique_for_overwrite<char_type[]>(
          codecvt_max_length() * default_buffer_code_point_size_ *
          2)}; // double-buffered
  std::span<char_type> buffer_{default_buffer_.get(),
                               codecvt_max_length() *
                                   default_buffer_code_point_size_ * 2};
  pos_type converted_pos_{};
  HANDLE console_;

protected:
  static auto section_size(std::integral auto size) noexcept {
    return size / 2;
  }
  static auto section_splitter(std::span<char_type> buffer) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return util::f::atad(buffer) - section_size(std::size(buffer));
  }
  static auto copy_front_to_back(std::span<char_type> front_buffer) {
    auto *const dest{std::data(front_buffer) - std::size(front_buffer)};
    std::memcpy(dest, std::data(front_buffer), std::size(front_buffer));
    return std::span{dest, std::size(front_buffer)};
  }

public:
  explicit Windows_console_input_buffer(HANDLE console) : console_{console} {
    auto *const splitter{section_splitter(buffer_)};
    setg(splitter, splitter, splitter);
  }

  Windows_console_input_buffer(Windows_console_input_buffer const &) = delete;
  auto operator=(Windows_console_input_buffer const &) = delete;
  Windows_console_input_buffer(Windows_console_input_buffer &&) noexcept =
      default;
  auto operator=(Windows_console_input_buffer &&) noexcept
      -> Windows_console_input_buffer & = default;
  ~Windows_console_input_buffer() noexcept override = default;

protected:
  auto setbuf(char_type *buffer, std::streamsize size)
      -> Windows_console_input_buffer * override {
    if (buffer == nullptr) {
      return this;
    }
    auto const clamped_size{
        util::f::int_clamp_cast<typename decltype(buffer_)::size_type>(size)};
    if (auto const sect_size{section_size(clamped_size)};
        sect_size >= codecvt_max_length()) {
      auto *const prev_egptr{egptr()};
      auto const transfer_size{std::min(
          sect_size, util::f::int_unsigned_clamp_cast(prev_egptr - eback()))};

      buffer_ = {buffer, clamped_size};
      auto *const splitter{section_splitter(buffer_)};
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      std::memmove(splitter - transfer_size, prev_egptr - transfer_size,
                   transfer_size);
      setg(splitter, splitter, splitter);

      default_buffer_.reset();
    }
    return this;
  }
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto seekoff(off_type offset, std::ios_base::seekdir direction,
               std::ios_base::openmode which) -> pos_type override {
    return seekpos(
        [direction, this, offset] {
          switch (direction) {
          case std::ios_base::beg:
            return pos_type{offset};
          case std::ios_base::end:
            return converted_pos_ + offset;
          case std::ios_base::cur:
            return converted_pos_ - (egptr() - gptr()) + offset;
          default:
            util::f::unreachable();
          }
#pragma warning(suppress : 4820)
        }(),
        which);
  }
  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  auto seekpos(pos_type pos, std::ios_base::openmode which)
      -> pos_type override {
    if ((util::f::int_unsigned_cast(which) & std::ios_base::in) == 0) {
      return off_type{-1};
    }
    auto const roff{converted_pos_ - pos};
    if (roff < 0 || roff > egptr() - eback()) {
      return off_type{-1};
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    setg(eback(), egptr() - roff, egptr());
    return pos;
  }
  auto sync() -> int override {
    if (!util::f::expect_nonzero(::FlushConsoleInputBuffer(console_))) {
      platform::windows::f::print_last_error();
      return -1;
    }
    auto *const splitter{section_splitter(buffer_)};
    setg(std::data(copy_front_to_back({splitter, std::min(gptr(), egptr())})),
         splitter, splitter);
    return 0;
  }
  auto underflow() -> int_type override {
    while (gptr() >= egptr()) {
      auto const read{[this] {
        auto const read_size_max{util::f::int_clamp_cast<DWORD>([this]() {
          auto const init{section_size(std::size(buffer_)) /
                          codecvt_max_length()};
          assert(init > 0 && u8"Buffer is too small");
          return init;
        }())};
        std::u16string init(read_size_max, u'\0');
        if (DWORD read_size{}; util::f::expect_nonzero(
                ::ReadConsoleW(console_, std::data(init), read_size_max,
                               &read_size, nullptr))) {
          init.resize(read_size);
        } else {
          platform::windows::f::print_last_error();
          init.clear();
        }
        return init;
      }()};
      if (std::empty(read)) {
        return traits_type::eof();
      }
      auto const converted{[this, &read] {
        std::u8string init(codecvt_max_length() * std::size(read), u8'\0');
        char8_t *write_ptr{std::data(init)};
        for (auto const *read_ptr{std::data(read)};
             read_ptr != util::f::atad(read);) {
          switch (codecvt_->out(mbstate_, read_ptr, util::f::atad(read),
                                read_ptr, write_ptr, util::f::atad(init),
                                write_ptr)) {
          case std::codecvt_base::ok:
            [[fallthrough]];
          case std::codecvt_base::partial:
            [[fallthrough]];
            [[unlikely]] case std::codecvt_base::noconv
                : assert(read_ptr == util::f::atad(read) &&
                         u8"Not all read characters are converted at once");
            break;
          case std::codecvt_base::error:
            if (std::mbsinit(&mbstate_) == 0) {
              mbstate_ = {}; // perhaps unmatched surrogate pair
            } else {
              // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
              ++read_ptr; // skip
            }
            break;
          default:
            util::f::unreachable();
          }
        }
        init.resize(assert_success(
            util::f::int_unsigned_exact_cast(write_ptr - std::data(init))));
        return init;
      }()};
      auto *const splitter{section_splitter(buffer_)};
      auto const back_buf{copy_front_to_back({splitter, egptr()})};
      std::memcpy(splitter, std::data(converted), std::size(converted));
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      setg(std::data(back_buf), splitter, splitter + std::size(converted));
      converted_pos_ +=
          util::f::int_modulo_cast<off_type>(std::size(converted));
    }
    return *gptr();
  }
  auto xsgetn(char_type *out, std::streamsize count)
      -> std::streamsize override {
    if (out == nullptr) {
      return 0;
    }
    count = std::max(count, decltype(count){0});
    auto const init_count{count};
    while (count > 0 && underflow() != traits_type::eof()) {
      auto const copy_count{std::min(
          count, util::f::int_clamp_cast<decltype(count)>(egptr() - gptr()))};
      std::memcpy(out, gptr(),
                  assert_success(util::f::int_unsigned_exact_cast(copy_count)));
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      setg(eback(), gptr() + copy_count, egptr());
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      out += copy_count;
      count -= copy_count;
    }
    return init_count - count;
  }
};
#endif
} // namespace detail

namespace f {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"
auto safe_main(
#pragma clang diagnostic pop
    std::function<int(Raw_arguments)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    gsl::czstring const argv[]) -> int {
  return main_func([args{util::f::const_span(argv, argv + argc)}] {
    std::vector<std::string_view> init(std::size(args));
    std::ranges::transform(args, std::begin(init),
                           [](gsl::not_null<gsl::czstring const> arg) {
                             return std::string_view{arg};
                           });
    return init;
  }());
}
#ifdef _WIN32
auto safe_main(
    std::function<int(Raw_arguments)> const &main_func, int argc,
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    gsl::cwzstring const argv[]) -> int {
  auto const utf8_args_storage{[args{util::f::const_span(argv, argv + argc)}] {
    std::vector<std::u8string> init(std::size(args));
    std::ranges::transform(
        args, std::begin(init), [](gsl::not_null<gsl::cwzstring const> arg) {
          // copy arg as UTF-16 (no conversion) and then convert to UTF-8
          return success_or_throw(util::f::utf16_to_utf8(
              // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
              std::u16string{arg.get(), arg.get() + std::wcslen(arg)}));
        });
    return init;
  }()};
  auto const argv_compatible_storage{[&utf8_args_storage] {
    std::vector<gsl::czstring> init(std::size(utf8_args_storage));
    std::ranges::transform(
        utf8_args_storage, std::begin(init), [](auto const &utf8_arg) {
          // defined behavior
          // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
          return reinterpret_cast<gsl::czstring>(std::data(utf8_arg));
        });
    return init;
  }()};
  return safe_main(main_func, argc, std::data(argv_compatible_storage));
}
#endif
} // namespace f

Main_program::Main_program(
    std::weak_ptr<std::vector<std::exception_ptr>> destructor_excs_out,
    Raw_arguments arguments)
    : early_structor_{[&destructor_excs_out] {
        std::vector<copyable_finalizer_type> finalizers{};

        auto const prev_stdio_sync{std::ios_base::sync_with_stdio(false)};
        finalizers.emplace_back(make_copyable_finalizer([prev_stdio_sync] {
          std::ios_base::sync_with_stdio(prev_stdio_sync);
        }));
#ifdef _WIN32
        auto const prev_locale{std::locale::global(
            std::locale{/*u8*/ ".UTF-8"})}; // ACP functions -> UTF-8 functions
        finalizers.emplace_back(make_copyable_finalizer(
            [prev_locale] { std::locale::global(prev_locale); }));

        if (auto const prev_console_output_cp{
                util::f::expect_nonzero(::GetConsoleOutputCP())};
            prev_console_output_cp &&
            util::f::expect_nonzero(::SetConsoleOutputCP(CP_UTF8))) {
          finalizers.emplace_back(
              make_copyable_finalizer([pcocp{*prev_console_output_cp}] {
                if (!util::f::expect_nonzero(::SetConsoleOutputCP(pcocp))) {
                  platform::windows::f::throw_last_error();
                }
              }));
        } else {
          platform::windows::f::print_last_error();
        }
        if (auto const prev_console_cp{
                util::f::expect_nonzero(::GetConsoleCP())};
            prev_console_cp &&
            util::f::expect_nonzero(::SetConsoleCP(CP_UTF8))) {
          finalizers.emplace_back(
              make_copyable_finalizer([pccp{*prev_console_cp}] {
                if (!util::f::expect_nonzero(::SetConsoleCP(pccp))) {
                  platform::windows::f::throw_last_error();
                }
              }));
        } else {
          platform::windows::f::print_last_error();
        }

        if (auto const std_handle{util::f::expect_noninvalid(
                // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-cstyle-cast)
                ::GetStdHandle(STD_INPUT_HANDLE), INVALID_HANDLE_VALUE)}) {
          if (DWORD console_mode{}; *std_handle != nullptr &&
                                    util::f::expect_nonzero(::GetConsoleMode(
                                        *std_handle, &console_mode))) {
            auto new_rdbuf{
                std::make_shared<detail::Windows_console_input_buffer>(
                    *std_handle)}; // TODO: C++23: std::make_unique
            auto *const prev_rdbuf{std::cin.rdbuf(new_rdbuf.get())};
            finalizers.emplace_back(make_copyable_finalizer(
                [prev_rdbuf, new_rdbuf{std::move(new_rdbuf)}]() mutable {
                  std::cin.rdbuf(prev_rdbuf);
                  assert(new_rdbuf.use_count() == 1); // TODO: C++23: remove
                  new_rdbuf.reset();                  // make it explicit
                }));
          }
        } else {
          platform::windows::f::print_last_error();
        }
#endif

        return make_copyable_finalizer(detail::run_finalizer_save_excepts(
            std::move(finalizers), destructor_excs_out));
      }()},
      arguments_{[arguments] {
        decltype(arguments_) init(std::size(arguments));
        auto const prev_loc{std::locale::global(std::locale{
            /*u8*/ ""})}; // use user-preferred locale to convert args
        gsl::final_action const finalizer{
            [&prev_loc] { std::locale::global(prev_loc); }};
        std::ranges::transform(arguments, std::begin(init),
                               [](auto arg) { return Argument{arg}; });
        return init;
      }()},
      late_structor_{[&destructor_excs_out] {
        std::vector<copyable_finalizer_type> finalizers{};

        finalizers.emplace_back(make_copyable_finalizer([] {
          std::cout.flush();
          std::clog.flush();
        }));
        return make_copyable_finalizer(detail::run_finalizer_save_excepts(
            std::move(finalizers), destructor_excs_out));
      }()} {
}
Main_program::~Main_program() noexcept = default;
auto Main_program::arguments [[nodiscard]] () const
    -> std::span<Argument const> {
  return arguments_;
}

Argument::Argument(std::string_view argument)
    : verbatim_{argument}, utf8_{discard_err(
                               util::f::loc_enc_to_utf8(argument))} {}
auto Argument::verbatim [[nodiscard]] () const noexcept -> std::string_view {
  return verbatim_;
}
auto Argument::utf8 [[nodiscard]] () const
    -> tl::expected<std::u8string_view, util::Exception_error> {
  return utf8_;
}
} // namespace artccel::core
