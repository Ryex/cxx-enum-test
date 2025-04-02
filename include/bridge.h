#pragma once

#include <memory>
#include <string>

struct SharedData;
struct RustEnum;
struct EnumWithRef;

struct CppValue {
  std::string value;
  std::string const &read() const { return value; }
};

std::unique_ptr<CppValue> make_cpp_value(const std::string &value);

SharedData make_shared_data();
RustEnum make_enum();
RustEnum make_enum_str();
RustEnum make_enum_shared();
RustEnum make_enum_opaque_rust();
RustEnum make_enum_opaque_cpp();
void take_enum(const RustEnum &enm);
void take_mut_enum(RustEnum &);

EnumWithRef make_ref_enum();
void take_ref_enum(const EnumWithRef &enm);
