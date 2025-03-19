#pragma once

#include "cxx_enum_test/include/cxxvariant.h"
#include <iostream>

struct RustEnum final
    : public ::rust::variant::variant<
        ::rust::variant::empty,
        int64_t,
        ::rust::string,
        bool
      > {
  using base =
      ::rust::variant::variant<
          ::rust::variant::empty,
          int64_t,
          ::rust::string,
          bool
      >;
  RustEnum() = delete;
  RustEnum(const RustEnum &) = default;
  RustEnum(RustEnum &&) = delete;
  using base::base;
  using base::operator=;

  using IsRelocatable = ::std::true_type;
};

template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};template<class... Ts> overload(Ts...) -> overload<Ts...>;

RustEnum make_enum();
RustEnum make_enum_str();
void take_enum(const RustEnum &enm);
void take_mut_enum(RustEnum&);
