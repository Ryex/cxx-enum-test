#pragma once

#include "cxxenum-test/include/cxxvariant.h"
#include "cxxenum-test/src/bridge.rs.h"
#include "rust/cxx.h"
#include <functional>

struct RustEnum_Tuple {
  int32_t _0;
  int32_t _1;
};

struct RustEnum_Struct {
  int32_t val;
  ::rust::String str;
};

struct RustEnum_Unit1 {};
struct RustEnum_Unit2 {};

struct RustValue;

using SharedDataRef = std::reference_wrapper<SharedData>;
using RustValueRef = std::reference_wrapper<RustValue>;

struct RustEnum final
    : public ::rust::variant::variant<
          ::rust::variant::empty, int64_t, ::rust::String, bool, SharedData,
          SharedDataRef, ::rust::Box<RustValue>, RustValueRef, RustEnum_Tuple,
          RustEnum_Struct, RustEnum_Unit1, RustEnum_Unit2> {

  using Empty = ::rust::variant::empty;
  using Num = int64_t;
  using String = ::rust::string;
  using Bool = bool;
  using Shared = SharedData;
  using SharedRef = SharedDataRef;
  using Opaque = ::rust::Box<RustValue>;
  using OpaqueRef = RustValueRef;
  using Tuple = RustEnum_Tuple;
  using Struct = RustEnum_Struct;
  using Unit1 = RustEnum_Unit1;
  using Unit2 = RustEnum_Unit2;

  using base = ::rust::variant::variant<
      ::rust::variant::empty, int64_t, ::rust::String, bool, SharedData,
      SharedDataRef, ::rust::Box<RustValue>, RustValueRef, RustEnum_Tuple,
      RustEnum_Struct, RustEnum_Unit1, RustEnum_Unit2>;

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
RustEnum make_enum_shared_ref();
RustEnum make_enum_opaque();

void take_enum(const RustEnum &enm);
void take_mut_enum(RustEnum &);
