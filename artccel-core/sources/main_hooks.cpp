#pragma warning(push) // suppress <Windows.h>
#pragma warning(disable : 4668 5039)
#pragma warning(push) // suppress <gsl/pointers>
#pragma warning(disable : 4626)
#pragma warning(push) // suppress <gsl/util>
#pragma warning(disable : 4820)
#include <artccel-core/main_hooks.hpp> // interface

#include <algorithm> // import std::min, std::ranges::for_each, std::ranges::transform
#include <artccel-core/export.h>                // import ARTCCEL_CORE_NO_EXPORT
#include <artccel-core/util/codecvt_extras.hpp> // import util::Codecvt_utf16_utf8
#include <artccel-core/util/containers_extras.hpp> // import util::f::const_span
#include <artccel-core/util/encoding.hpp> // import util::f::loc_enc_to_utf8, util::f::utf16_to_utf8
#include <artccel-core/util/numeric_conversions.hpp> // import util::f::int_clamp_cast, util::f::int_modulo_cast
#include <artccel-core/util/polyfill.hpp> // import util::f::unreachable, util::literals::operator""_UZ
#include <artccel-core/util/utility_extras.hpp> // import util::Semiregularize
#include <cassert>                              // import assert
#include <concepts>   // import std::integral, std::same_as
#include <cstring>    // import std::memcpy, std::memmove, std::size_t
#include <cwchar>     // import std::mbsinit, std::mbstate_t, std::wcslen
#include <exception>  // import std::current_exception, std::exception_ptr
#include <functional> // import std::function
#include <gsl/gsl> // import gsl::cwzstring, gsl::czstring, gsl::final_action, gsl::not_null
#include <ios> // import std::ios_base::openmode, std::ios_base::seekdir, std::streamsize
#include <iostream> // import std::cin, std::clog, std::cout, std::ios_base::sync_with_stdio
#include <locale> // import std::codecvt_base::result, std::locale, std::locale::global
#include <memory> // import std::make_shared, std::make_unique, std::make_unique_for_overwrite, std::unique_ptr, std::weak_ptr
#include <optional> // import std::nullopt, std::optional
#include <span> // import std::begin, std::data, std::empty, std::size, std::span
#include <streambuf>   // import std::streambuf
#include <string>      // import std::u16string, std::u8string
#include <string_view> // import std::string_view, std::u8string_view
#include <type_traits> /// import std::decay_t
#include <utility>     // import std::move
#include <variant>     // import std::get_if, std::variant, std::visit
#include <vector>      // import std::vector
#ifdef _WIN32
#include <Windows.h> // import ::FlushConsoleInputBuffer, ::GetConsoleCP, ::GetConsoleMode, ::GetConsoleOutputCP, ::GetStdHandle, ::ReadConsoleW, ::SetConsoleCP, ::SetConsoleOutputCP, CP_UTF8, DWORD, HANDLE, INVALID_HANDLE_VALUE, STD_INPUT_HANDLE
#include <artccel-core/platform/windows_error.hpp> // import platform::windows::f::throw_last_error, platform::windows::f::print_last_error
#endif
#pragma warning(pop)
#pragma warning(pop)
#pragma warning(pop)

namespace artccel::core {
namespace detail {
using util::literals::operator""_UZ;

static auto run_finalizer_save_excepts(
    std::vector<Main_program::copyable_finalizer_type> finalizers,
    std::weak_ptr<Main_program::destructor_exceptions_out_type>
        dtor_excs_out) noexcept {
  return [finalizers{std::move(finalizers)},
          dtor_excs_out{std::move(dtor_excs_out)}]() mutable noexcept {
    if (auto const dtor_excs{dtor_excs_out.lock()}) {
      std::ranges::for_each(finalizers, [&dtor_excs](auto &finalizer) noexcept {
        try {
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

  static codecvt_type const helper_codecvt_;
  constexpr static auto default_buffer_code_point_size_{4096_UZ};
  static std::size_t const default_buffer_size_;
  static auto
  calculate_buffer_size(std::integral auto code_point_size) noexcept {
    return code_point_size * helper_codecvt_.max_length() *
           2; // double-buffered
  }

private:
  std::unique_ptr<char_type[]> default_buffer_{
      std::make_unique_for_overwrite<char_type[]>(default_buffer_size_)};
  HANDLE console_;
  std::span<char_type> buffer_{default_buffer_.get(), default_buffer_size_};
  pos_type converted_pos_{};
  std::unique_ptr<codecvt_type> codecvt_{std::make_unique<codecvt_type>()};
  std::mbstate_t mbstate_{};

protected:
  static auto section_splitter(std::span<char_type> buffer) noexcept {
    return std::data(buffer) + std::size(buffer) -
           section_size(std::size(buffer));
  }
  static auto section_size(std::integral auto size) noexcept {
    return size / 2;
  }
  static auto copy_front_to_back(std::span<char_type> front_buffer) {
    auto const dest{std::data(front_buffer) - std::size(front_buffer)};
    std::memcpy(dest, std::data(front_buffer), std::size(front_buffer));
    return std::span{dest, std::size(front_buffer)};
  }

public:
  Windows_console_input_buffer(HANDLE console) : console_{console} {
    assert(codecvt_->encoding() != -1 &&
           u8"State-dependent encoding unsupported");
    auto const splitter{section_splitter(buffer_)};
    setg(splitter, splitter, splitter);
  }

  Windows_console_input_buffer(Windows_console_input_buffer const &) = delete;
  auto operator=(Windows_console_input_buffer const &) = delete;
  Windows_console_input_buffer(Windows_console_input_buffer &&) noexcept =
      default;
  auto operator=(Windows_console_input_buffer &&) noexcept
      -> Windows_console_input_buffer & = default;

protected:
  auto setbuf(char_type *buffer, std::streamsize size)
      -> Windows_console_input_buffer * override {
    if (!buffer)
      return this;
    auto const clamped_size{
        util::f::int_clamp_cast<typename decltype(buffer_)::size_type>(size)};
    if (auto const sect_size{section_size(clamped_size)};
        sect_size >=
        util::f::int_clamp_cast<decltype(sect_size)>(codecvt_->max_length())) {
      auto const prev_egptr{egptr()};
      auto const transfer_size{std::min(
          sect_size,
          util::f::int_clamp_cast<decltype(sect_size)>(prev_egptr - eback()))};

      buffer_ = {buffer, clamped_size};
      auto const splitter{section_splitter(buffer_)};
      std::memmove(splitter - transfer_size, prev_egptr - transfer_size,
                   transfer_size);
      setg(splitter, splitter, splitter);

      default_buffer_.reset();
    }
    return this;
  }
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
  auto seekpos(pos_type pos, std::ios_base::openmode which)
      -> pos_type override {
    if (!(which & std::ios_base::in)) {
      return off_type{-1};
    }
    auto const roff{converted_pos_ - pos};
    if (roff < 0 || roff > egptr() - eback()) {
      return off_type{-1};
    }
    setg(eback(), egptr() - roff, egptr());
    return pos;
  }
  auto sync() -> int override {
    if (!::FlushConsoleInputBuffer(console_)) {
      platform::windows::f::print_last_error();
      return -1;
    }
    auto const splitter{section_splitter(buffer_)};
    setg(std::data(copy_front_to_back({splitter, std::min(gptr(), egptr())})),
         splitter, splitter);
    return 0;
  }
  auto underflow() -> int_type override {
    if (gptr() >= egptr()) {
      auto const read{[this] {
        auto const read_size_max{
            util::f::int_clamp_cast<DWORD>([this]() noexcept {
              auto const init{section_size(std::size(buffer_)) /
                              codecvt_->max_length()};
              assert(init > 0 && u8"Buffer is too small");
              return init;
            }())};
        std::u16string init(read_size_max, u'\0');
        if (DWORD read_size{};
            ::ReadConsoleW(console_, std::data(init), read_size_max, &read_size,
                           nullptr)) {
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
        std::u8string init(codecvt_->max_length() * std::size(read), u8'\0');
        char8_t *write_ptr{std::data(init)};
        for (auto const *read_ptr{std::data(read)};
             read_ptr != std::data(read) + std::size(read);) {
          switch (codecvt_->out(
              mbstate_, read_ptr, std::data(read) + std::size(read), read_ptr,
              write_ptr, std::data(init) + std::size(init), write_ptr)) {
          case std::codecvt_base::ok:
            [[fallthrough]];
          case std::codecvt_base::partial:
            [[fallthrough]];
            [[unlikely]] case std::codecvt_base::noconv
                : assert(read_ptr == std::data(read) + std::size(read) &&
                         u8"Not all read characters are converted at once");
            break;
          case std::codecvt_base::error:
            if (std::mbsinit(&mbstate_)) {
              ++read_ptr; // skip
            } else {
              mbstate_ = {}; // perhaps unmatched surrogate pair
            }
            break;
          }
        }
        init.resize(util::f::int_clamp_cast<typename decltype(init)::size_type>(
            write_ptr - std::data(init)));
        return init;
      }()};
      auto const splitter{section_splitter(buffer_)};
      auto const back_buf{copy_front_to_back({splitter, egptr()})};
      std::memcpy(splitter, std::data(converted), std::size(converted));
      setg(std::data(back_buf), splitter, splitter + std::size(converted));
      converted_pos_ +=
          util::f::int_modulo_cast<off_type>(std::size(converted));

      if (gptr() >= egptr()) {
        return underflow(); // handles surrogate pairs
      }
    }
    return *gptr();
  }
  auto xsgetn(char_type *out, std::streamsize count)
      -> std::streamsize override {
    auto const initial_count{count};
    while (count > 0 && underflow() != traits_type::eof()) {
      auto const copy_count{util::f::int_clamp_cast<std::size_t>(
          std::min(count, egptr() - gptr()))};
      std::memcpy(out, gptr(), copy_count);
      setg(eback(), gptr() + copy_count, egptr());
      out += copy_count;
      count -= copy_count;
    }
    return initial_count - count;
  }
};
Windows_console_input_buffer::codecvt_type const
    Windows_console_input_buffer::helper_codecvt_{};
std::size_t const Windows_console_input_buffer::default_buffer_size_{
    Windows_console_input_buffer::calculate_buffer_size(
        default_buffer_code_point_size_)};
#endif
} // namespace detail

namespace f {
auto safe_main(
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
          return util::f::utf16_to_utf8(
              std::u16string{arg.get(), arg.get() + std::wcslen(arg)});
        });
    return init;
  }()};
  auto const argv_compatible_storage{[&utf8_args_storage] {
    std::vector<gsl::czstring> init(std::size(utf8_args_storage));
    std::ranges::transform(
        utf8_args_storage, std::begin(init), [](auto const &utf8_arg) {
          // defined behavior
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

        if (auto const prev_console_output_cp{::GetConsoleOutputCP()};
            prev_console_output_cp && ::SetConsoleOutputCP(CP_UTF8)) {
          finalizers.emplace_back(
              make_copyable_finalizer([prev_console_output_cp] {
                if (!::SetConsoleOutputCP(prev_console_output_cp)) {
                  platform::windows::f::throw_last_error();
                }
              }));
        } else {
          platform::windows::f::print_last_error();
        }
        if (auto const prev_console_cp{::GetConsoleCP()};
            prev_console_cp && ::SetConsoleCP(CP_UTF8)) {
          finalizers.emplace_back(make_copyable_finalizer([prev_console_cp] {
            if (!::SetConsoleCP(prev_console_cp)) {
              platform::windows::f::throw_last_error();
            }
          }));
        } else {
          platform::windows::f::print_last_error();
        }
        if (auto const std_handle{::GetStdHandle(STD_INPUT_HANDLE)};
            std_handle != INVALID_HANDLE_VALUE) {
          if (DWORD console_mode{};
              std_handle && ::GetConsoleMode(std_handle, &console_mode)) {
            auto new_rdbuf{
                std::make_shared<detail::Windows_console_input_buffer>(
                    std_handle)}; // TODO: C++23: std::make_unique
            auto const prev_rdbuf{std::cin.rdbuf(new_rdbuf.get())};
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
    : verbatim_{argument}, utf8_{[argument]() -> decltype(utf8_) {
        std::u8string init{};
        try {
          init = util::f::loc_enc_to_utf8(argument);
        } catch (...) {
          return std::current_exception();
        }
        return init;
      }()} {}
auto Argument::verbatim [[nodiscard]] () const noexcept -> std::string_view {
  return verbatim_;
}
auto Argument::utf8 [[nodiscard]] () const
    -> std::optional<std::u8string_view> {
  if (auto const *val{std::get_if<std::u8string>(&utf8_)}) {
    return std::u8string_view{*val};
  }
  return std::nullopt;
}
auto Argument::utf8_or_exc [[nodiscard]] () const
    -> std::variant<std::u8string_view, gsl::not_null<std::exception_ptr>> {
  using return_type = decltype(utf8_or_exc());
  return std::visit([](auto &&var) -> return_type { return var; }, utf8_);
}
} // namespace artccel::core
