#pragma once

#include "rust/cxx.h"
#include "cxxenum-test/include/cxxvariant.h"
#include "cxxenum-test/src/bridge.rs.h"

struct RustEnum final
    : public ::rust::variant::variant<::rust::variant::empty, int64_t,
                                      ::rust::String, bool, SharedData> {
  using Empty = ::rust::variant::empty;
  using Num = int64_t;
  using String = ::rust::string;
  using Bool = bool;
  using Shared = SharedData;
  using base = ::rust::variant::variant<::rust::variant::empty, int64_t,
                                        ::rust::String, bool, SharedData>;
  RustEnum() = delete;
  RustEnum(const RustEnum &) = default;
  RustEnum(RustEnum &&) = delete;
  using base::base;
  using base::operator=;

  using IsRelocatable = ::std::true_type;
};

template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

RustEnum make_enum();
RustEnum make_enum_str();
RustEnum make_enum_shared();
void take_enum(const RustEnum &enm);
void take_mut_enum(RustEnum &);

