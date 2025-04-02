
#include "cxxenum-test/include/bridge.h"
#include "cxxenum-test/src/bridge.rs.h"
#include "rust/cxx.h"
#include <iostream>
#include <memory>

SharedData make_shared_data() {
  SharedData d;
  d.size = 4;
  d.tags = ::rust::Vec<::rust::String>({
      rust::String("tag_a"),
      rust::String("tag_b"),
      rust::String("tag_c"),
      rust::String("tag_d"),
  });

  return d;
}

std::unique_ptr<CppValue> make_cpp_value(const std::string &value) {
  return std::unique_ptr<CppValue>(new CppValue{value});
}

RustEnum make_enum() { return RustEnum{RustEnum::Num(1502)}; }
RustEnum make_enum_str() {
  return RustEnum(RustEnum::String("String from c++"));
}
RustEnum make_enum_shared() { return RustEnum(make_shared_data()); }
RustEnum make_enum_opaque_rust() {
  return RustEnum(
      new_opaque_rust_value(rust::String("String from C++ in RustValue")));
}
RustEnum make_enum_opaque_cpp() {
  std::string s = "String from C++ in CppValue";
  return RustEnum(make_cpp_value(s));
}

template <class... Ts> struct overload : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overload(Ts...) -> overload<Ts...>;

void take_enum(const RustEnum &enm) {
  std::cout << "The index of enum is " << enm.index() << std::endl;
  const auto visitor = overload{
      [](const RustEnum::Empty) {
        std::cout << "The value of enum is ::rust::empty" << std::endl;
      },
      [](const RustEnum::String &v) {
        std::cout << "The value of enum is string '" << v << "'" << std::endl;
      },
      [](const RustEnum::Shared &d) {
        std::cout << "The value of enum is SharedData struct { " << std::endl
                  << "\tsize: " << d.size << "," << std::endl
                  << "\ttags: [";
        for (const auto &tag : d.tags) {
          std::cout << std::endl << "\t\t\"" << tag << "\", ";
        }
        std::cout << std::endl << "\t]," << std::endl << "}" << std::endl;
      },
      [](const RustEnum::OpaqueRust &o) {
        std::cout << "The value of enum behind Opaque RustValue is '"
                  << o->read() << "'" << std::endl;
      },
      [](const RustEnum::OpaqueCpp &o) {
        std::cout << "The value of enum behind Opaque CPPValue is '"
                  << o->read() << "'" << std::endl;
      },
      [](const auto &v) {
        std::cout << "The value of enum is " << v << std::endl;
      },
  };
  rust::visit(visitor, enm);
}

const static std::string s_cpp_string = "Static std::string from C++";

EnumWithRef make_ref_enum() {
  rust::Str s(s_cpp_string);
  return EnumWithRef(s);
}

void take_ref_enum(const EnumWithRef &enm) {
  std::cout << "The index of ref enum is " << enm.index() << std::endl;
  const auto visitor = overload{
      [](const EnumWithRef::Str &v) {
        std::cout << "The value of ref enum is " << v << std::endl;
      },
  };
  rust::visit(visitor, enm);
}

void take_mut_enum(RustEnum &enm) {
  take_enum(enm);
  if (!::rust::holds_alternative<RustEnum::Bool>(enm)) {
    enm = false;
  } else {
    enm = int64_t(111);
  }
}
