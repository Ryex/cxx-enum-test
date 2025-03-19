#pragma once
#include "rust/cxx.h"
#include <cstring>
#include <memory>
#include <string>

// If you're using enums and variants on windows, you need to pass also
// `/Zc:__cplusplus` as a compiler to make __cplusplus work correctly. If users
// ever report that they have a too old compiler to `/Zc:__cplusplus` we may
// fallback to the `_MSVC_LANG` define.
//
// Sources:
// https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
// https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
#if __cplusplus >= 201703L
#include <variant>
#endif

namespace rust {
namespace variant {

// Adjusted from std::variant_alternative. Standard selects always the most
// specialized template specialization. See
// https://timsong-cpp.github.io/cppwp/n4140/temp.class.spec.match and
// https://timsong-cpp.github.io/cppwp/n4140/temp.class.order.
template <std::size_t I, typename... Ts> struct variant_alternative;

// Specialization for gracefully handling invalid indices.
template <std::size_t I> struct variant_alternative<I> {};

template <std::size_t I, typename First, typename... Remainder>
struct variant_alternative<I, First, Remainder...>
    : variant_alternative<I - 1, Remainder...> {};

template <typename First, typename... Remainder>
struct variant_alternative<0, First, Remainder...> {
  using type = First;
};

template <std::size_t I, typename... Ts>
using variant_alternative_t = typename variant_alternative<I, Ts...>::type;

template <bool... Values> constexpr size_t compile_time_count() noexcept {
  return 0 + (static_cast<std::size_t>(Values) + ...);
}

template <bool... Values>
struct count
    : std::integral_constant<std::size_t, compile_time_count<Values...>()> {};

template <bool... Values>
struct exactly_once : std::conditional_t<count<Values...>::value == 1,
                                         std::true_type, std::false_type> {};

template <std::size_t I, bool... Values> struct index_from_booleans;

template <std::size_t I> struct index_from_booleans<I> {};

template <std::size_t I, bool First, bool... Remainder>
struct index_from_booleans<I, First, Remainder...>
    : index_from_booleans<I + 1, Remainder...> {};

template <std::size_t I, bool... Remainder>
struct index_from_booleans<I, true, Remainder...>
    : std::integral_constant<std::size_t, I> {};

template <typename Type, typename... Ts>
struct index_from_type
    : index_from_booleans<0, std::is_same_v<std::decay_t<Type>, Ts>...> {
  static_assert(exactly_once<std::is_same_v<std::decay_t<Type>, Ts>...>::value,
                "Index must be unique");
};

template <typename... Ts> struct visitor_type;

template <typename... Ts> struct variant_base;

template <std::size_t I, typename... Ts>
constexpr decltype(auto) get(variant_base<Ts...> &);

template <std::size_t I, typename... Ts>
constexpr decltype(auto) get(const variant_base<Ts...> &);

template <typename Visitor, typename... Ts>
constexpr decltype(auto) visit(Visitor &&visitor, variant_base<Ts...> &);

template <typename Visitor, typename... Ts>
constexpr decltype(auto) visit(Visitor &&visitor, const variant_base<Ts...> &);

/// @brief A std::variant like tagged union with the same memory layout as a
/// Rust Enum.
///
/// The memory layout of the Rust enum is defined under
/// https://doc.rust-lang.org/reference/type-layout.html#reprc-enums-with-fields
template <typename... Ts> struct variant_base {
  static_assert(sizeof...(Ts) > 0,
                "variant_base must hold at least one alternative");

  /// @brief Delete the default constructor since we cannot be in an
  /// uninitialized state (if the first alternative throws). Corresponds to the
  /// (1) constructor in std::variant.
  variant_base() = delete;

  constexpr static bool all_copy_constructible_v =
      std::conjunction_v<std::is_copy_constructible<Ts>...>;

  /// @brief Copy constructor. Participates only in the resolution if all types
  /// are copy constructable. Corresponds to (2) constructor of std::variant.
  variant_base(const variant_base &other) {
    static_assert(
        all_copy_constructible_v,
        "Copy constructor requires that all types are copy constructable");

    m_Index = other.m_Index;
    visit(
        [this](const auto &value) {
          using type = std::decay_t<decltype(value)>;
          new (static_cast<void *>(m_Buff)) type(value);
        },
        other);
  };

  /// @brief Delete the move constructor since if we move this container it's
  /// unclear in which state it is. Corresponds to (3) constructor of
  /// std::variant.
  variant_base(variant_base &&other) = delete;

  template <typename T>
  constexpr static bool is_unique_v =
      exactly_once<std::is_same_v<Ts, std::decay_t<T>>...>::value;

  template <typename T>
  constexpr static std::size_t index_from_type_v =
      index_from_type<T, Ts...>::value;

  /// @brief Converting constructor. Corresponds to (4) constructor of
  /// std::variant.
  template <typename T, typename D = std::decay_t<T>,
            typename = std::enable_if_t<is_unique_v<T> &&
                                        std::is_constructible_v<D, T>>>
  variant_base(T &&other) noexcept(std::is_nothrow_constructible_v<D, T>) {
    m_Index = index_from_type_v<D>;
    new (static_cast<void *>(m_Buff)) D(std::forward<T>(other));
  }

  /// @brief Participates in the resolution only if we can construct T from Args
  /// and if T is unique in Ts. Corresponds to (5) constructor of std::variant.
  template <typename T, typename... Args,
            typename = std::enable_if_t<is_unique_v<T>>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  explicit variant_base(std::in_place_type_t<T> type, Args &&...args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>)
      : variant_base{std::in_place_index<index_from_type_v<T>>,
                     std::forward<Args>(args)...} {}

  template <std::size_t I>
  using type_from_index_t = variant_alternative_t<I, Ts...>;

  /// @brief Participates in the resolution only if the index is within range
  /// and if the type can be constructor from Args. Corresponds to (7) of
  /// std::variant.
  template <std::size_t I, typename... Args, typename T = type_from_index_t<I>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  explicit variant_base(
      std::in_place_index_t<I> index,
      Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    m_Index = I;
    new (static_cast<void *>(m_Buff)) T(std::forward<Args>(args)...);
  }

  template <typename... Rs>
  constexpr static bool all_same_v =
      std::conjunction_v<std::is_same<Rs, Ts>...>;

  /// @brief Converts the std::variant to our variant. Participates only in
  /// the resolution if all types in Ts are copy constructable.
  template <typename... Rs, typename = std::enable_if_t<
                                all_same_v<Rs...> && all_copy_constructible_v>>
  variant_base(const std::variant<Rs...> &other) {
    m_Index = other.index();
    std::visit(
        [this](const auto &value) {
          using type = std::decay_t<decltype(value)>;
          new (static_cast<void *>(m_Buff)) type(value);
        },
        other);
  }

  constexpr static bool all_move_constructible_v =
      std::conjunction_v<std::is_move_constructible<Ts>...>;

  /// @brief Converts the std::variant to our variant. Participates only in
  /// the resolution if all types in Ts are move constructable.
  template <typename... Rs, typename = std::enable_if_t<
                                all_same_v<Rs...> && all_move_constructible_v>>
  variant_base(std::variant<Rs...> &&other) {
    m_Index = other.index();
    std::visit(
        [this](auto &&value) {
          using type = std::decay_t<decltype(value)>;
          new (static_cast<void *>(m_Buff)) type(std::move(value));
        },
        other);
  }

  ~variant_base() { destroy(); }

  /// @brief Copy assignment. Staticly fails if not every type in Ts is copy
  /// constructable. Corresponds to (1) assignment of std::variant.
  variant_base &operator=(const variant_base &other) {
    static_assert(
        all_copy_constructible_v,
        "Copy assignment requires that all types are copy constructable");

    visit([this](const auto &value) { *this = value; }, other);

    return *this;
  };

  /// @brief Deleted move assignment. Same as for the move constructor.
  /// Would correspond to (2) assignment of std::variant.
  variant_base &operator=(variant_base &&other) = delete;

  /// @brief Converting assignment. Corresponds to (3) assignment of
  /// std::variant.
  template <typename T, typename = std::enable_if_t<
                            is_unique_v<T> && std::is_constructible_v<T &&, T>>>
  variant_base &operator=(T &&other) {
    constexpr auto index = index_from_type_v<T>;

    if (m_Index == index) {
      if constexpr (std::is_nothrow_assignable_v<T, T &&>) {
        get<index>(*this) = std::forward<T>(other);
        return *this;
      }
    }
    this->emplace<std::decay_t<T>>(std::forward<T>(other));
    return *this;
  }

  /// @brief Converting assignment from std::variant. Participates only in the
  /// resolution if all types in Ts are copy constructable.
  template <typename... Rs, typename = std::enable_if_t<
                                all_same_v<Rs...> && all_copy_constructible_v>>
  variant_base &operator=(const std::variant<Rs...> &other) {
    // TODO this is not really clean since we fail if std::variant has
    // duplicated types.
    std::visit(
        [this](const auto &value) {
          using type = decltype(value);
          emplace<std::decay_t<type>>(value);
        },
        other);
    return *this;
  }

  /// @brief Converting assignment from std::variant. Participates only in the
  /// resolution if all types in Ts are move constructable.
  template <typename... Rs, typename = std::enable_if_t<
                                all_same_v<Rs...> && all_move_constructible_v>>
  variant_base &operator=(std::variant<Rs...> &&other) {
    // TODO this is not really clean since we fail if std::variant has
    // duplicated types.
    std::visit(
        [this](auto &&value) {
          using type = decltype(value);
          emplace<std::decay_t<type>>(std::move(value));
        },
        other);
    return *this;
  }

  /// @brief Emplace function. Participates in the resolution only if T is
  /// unique in Ts and if T can be constructed from Args. Offers strong
  /// exception guarantee. Corresponds to the (1) emplace function of
  /// std::variant.
  template <typename T, typename... Args,
            typename = std::enable_if_t<is_unique_v<T>>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  T &emplace(Args &&...args) {
    constexpr std::size_t index = index_from_type_v<T>;
    return this->emplace<index>(std::forward<Args>(args)...);
  }

  /// @brief Emplace function. Participates in the resolution only if T can be
  /// constructed from Args. Offers strong exception guarantee. Corresponds to
  /// the (2) emplace function of std::variant.
  ///
  /// The std::variant can have no valid state if the type throws during the
  /// construction. This is represented by the `valueless_by_exception` flag.
  /// The same approach is also used in absl::variant [2].
  /// In our case we can't accept valueless enums since we can't represent this
  /// in Rust. We must therefore provide a strong exception guarantee for all
  /// operations using `emplace`. Two famous implementations of never valueless
  /// variants are Boost/variant [3] and Boost/variant2 [4]. Boost/variant2 uses
  /// two memory buffers - which would be not compatible with Rust Enum's memory
  /// layout. The Boost/variant backs up the old object and calls its d'tor
  /// before constructing the new object; It then copies the old data back to
  /// the buffer if the construction fails - which might contain garbage (since)
  /// the d'tor was already called.
  ///
  ///
  /// We take a similar approach to Boost/variant. Assuming that constructing or
  /// moving the new type can throw, we backup the old data, try to construct
  /// the new object in the final buffer, swap the buffers, such that the old
  /// object is back in its original place, detroy it and move the new object
  /// from the old buffer back to the final place.
  ///
  /// Sources
  ///
  /// [1]
  /// https://en.cppreference.com/w/cpp/utility/variant/valueless_by_exception
  /// [2]
  /// https://github.com/abseil/abseil-cpp/blob/master/absl/types/variant.h
  /// [3]
  /// https://www.boost.org/doc/libs/1_84_0/libs/variant2/doc/html/variant2.html
  /// [4]
  /// https://www.boost.org/doc/libs/1_84_0/doc/html/variant/design.html#variant.design.never-empty
  template <std::size_t I, typename... Args, typename T = type_from_index_t<I>,
            typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
  T &emplace(Args &&...args) {
    if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
      destroy();
      new (static_cast<void *>(m_Buff)) T(std::forward<Args>(args)...);
    } else if constexpr (std::is_nothrow_move_constructible_v<T>) {
      // This operation may throw, but we know that the move does not.
      const T tmp{std::forward<Args>(args)...};

      // The operations below are save.
      destroy();
      new (static_cast<void *>(m_Buff)) T(std::move(tmp));
    } else {
      // Backup the old data.
      alignas(Ts...) std::byte old_buff[std::max({sizeof(Ts)...})];
      std::memcpy(old_buff, m_Buff, sizeof(m_Buff));

      try {
        // Try to construct the new object
        new (static_cast<void *>(m_Buff)) T(std::forward<Args>(args)...);
      } catch (...) {
        // Restore the old buffer
        std::memcpy(m_Buff, old_buff, sizeof(m_Buff));
        throw;
      }
      // Fetch the old buffer and detroy it.
      std::swap_ranges(m_Buff, m_Buff + sizeof(m_Buff), old_buff);

      destroy();
      std::memcpy(m_Buff, old_buff, sizeof(m_Buff));
    }

    m_Index = I;
    return get<I>(*this);
  }

  constexpr std::size_t index() const noexcept { return m_Index; }
  void swap(variant_base &other) {
    // TODO
  }

  struct my_bad_variant_access : std::runtime_error {
    my_bad_variant_access(std::size_t index)
        : std::runtime_error{"The index should be " + std::to_string(index)} {}
  };

private:
  template <std::size_t I> void throw_if_invalid() const {
    static_assert(I < (sizeof...(Ts)), "Invalid index");

    if (m_Index != I)
      throw my_bad_variant_access(m_Index);
  }

  void destroy() {
    visit(
        [](const auto &value) {
          using type = std::decay_t<decltype(value)>;
          value.~type();
        },
        *this);
  }

  // The underlying type is not fixed, but should be int - which we will verify
  // statically. See
  // https://timsong-cpp.github.io/cppwp/n4659/dcl.enum#7
  int m_Index;

  // std::aligned_storage is deprecated and may be replaced with the construct
  // below. See
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p1413r3.pdf
  alignas(Ts...) std::byte m_Buff[std::max({sizeof(Ts)...})];

  // The friend zone
  template <std::size_t I, typename... Rs>
  friend constexpr decltype(auto) get(variant_base<Rs...> &variant);

  template <std::size_t I, typename... Rs>
  friend constexpr decltype(auto) get(const variant_base<Rs...> &variant);

  template <typename... Rs> friend struct visitor_type;
};

template <typename First, typename... Remainder>
struct visitor_type<First, Remainder...> {
  template <typename Visitor, typename Variant>
  constexpr static decltype(auto) visit(Visitor &&visitor, Variant &&variant) {
    return visit(std::forward<Visitor>(visitor), variant.m_Index,
                 variant.m_Buff);
  }

  /// @brief The visit method which will pick the right type depending on the
  /// `index` value.
  template <typename Visitor>
  constexpr static auto visit(Visitor &&visitor, std::size_t index,
                              std::byte *data)
      -> decltype(visitor(*reinterpret_cast<First *>(data))) {
    if (index == 0) {
      return visitor(*reinterpret_cast<First *>(data));
    }
    if constexpr (sizeof...(Remainder) != 0) {
      return visitor_type<Remainder...>::visit(std::forward<Visitor>(visitor),
                                               --index, data);
    }
    throw std::out_of_range("invalid");
  }

  template <typename Visitor>
  constexpr static auto visit(Visitor &&visitor, std::size_t index,
                              const std::byte *data)
      -> decltype(visitor(*reinterpret_cast<const First *>(data))) {
    if (index == 0) {
      return visitor(*reinterpret_cast<const First *>(data));
    }
    if constexpr (sizeof...(Remainder) != 0) {
      return visitor_type<Remainder...>::visit(std::forward<Visitor>(visitor),
                                               --index, data);
    }
    throw std::out_of_range("invalid");
  }
};

/// @brief Applies the visitor to the variant. Corresponds to the (3)
/// std::visit defintion.
template <typename Visitor, typename... Ts>
constexpr decltype(auto) visit(Visitor &&visitor,
                               variant_base<Ts...> &variant) {
  return visitor_type<Ts...>::visit(std::forward<Visitor>(visitor), variant);
}

/// @brief Applies the visitor to the variant. Corresponds to the (4)
/// std::visit defintion.
template <typename Visitor, typename... Ts>
constexpr decltype(auto) visit(Visitor &&visitor,
                               const variant_base<Ts...> &variant) {
  return visitor_type<Ts...>::visit(std::forward<Visitor>(visitor), variant);
}

template <std::size_t I, typename... Ts>
constexpr decltype(auto) get(variant_base<Ts...> &variant) {
  variant.template throw_if_invalid<I>();
  return *reinterpret_cast<variant_alternative_t<I, Ts...> *>(variant.m_Buff);
}

template <std::size_t I, typename... Ts>
constexpr decltype(auto) get(const variant_base<Ts...> &variant) {
  variant.template throw_if_invalid<I>();
  return *reinterpret_cast<const variant_alternative_t<I, Ts...> *>(
      variant.m_Buff);
}

template <typename T, typename... Ts,
          typename = std::enable_if_t<
              exactly_once<std::is_same_v<Ts, std::decay_t<T>>...>::value>>
constexpr const T &get(const variant_base<Ts...> &variant) {
  constexpr auto index = index_from_type<T, Ts...>::value;
  return get<index>(variant);
}

template <typename T, typename... Ts,
          typename = std::enable_if_t<
              exactly_once<std::is_same_v<Ts, std::decay_t<T>>...>::value>>
constexpr T &get(variant_base<Ts...> &variant) {
  constexpr auto index = index_from_type<T, Ts...>::value;
  return get<index>(variant);
}

template <std::size_t I, typename... Ts>
constexpr bool holds_alternative(const variant_base<Ts...> &variant) {
  return variant.index() == I;
}

template <typename T, typename... Ts,
          typename = std::enable_if_t<
              exactly_once<std::is_same_v<Ts, std::decay_t<T>>...>::value>>
constexpr bool holds_alternative(const variant_base<Ts...> &variant) {
  return variant.index() == index_from_type<T, Ts...>::value;
}

template <bool> struct copy_control;

template <> struct copy_control<true> {
  copy_control() = default;
  copy_control(const copy_control &other) = default;
  copy_control &operator=(const copy_control &) = default;
};

template <> struct copy_control<false> {
  copy_control() = default;
  copy_control(const copy_control &other) = delete;
  copy_control &operator=(const copy_control &) = delete;
};

template <typename... Ts>
using allow_copy =
    copy_control<std::conjunction_v<std::is_copy_constructible<Ts>...>>;

template <typename... Ts>
struct variant : public variant_base<Ts...>, private allow_copy<Ts...> {
  using base = variant_base<Ts...>;

  variant() = delete;
  variant(const variant &) = default;
  variant(variant &&) = delete;

  using base::base;

  variant &operator=(const variant &) = default;
  variant &operator=(variant &&) = delete;

  using base::operator=;
};

/// An empty type used for unit variants from Rust.
struct empty {};


} // namespace variant
} // namespace rust
