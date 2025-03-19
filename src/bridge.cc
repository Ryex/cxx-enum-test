
#include "cxx_enum_test/include/bridge.h"
#include "rust/cxx.h"
#include <iostream>
RustEnum make_enum() { return RustEnum{int64_t(1502)}; }
RustEnum make_enum_str() { return RustEnum(::rust::String("String from c++")); }
RustEnum make_enum_shared() {
  SharedData d;
  d.size = 4;
  d.tags = ::rust::Vec<::rust::String>({
      ::rust::String("tag_a"),
      ::rust::String("tag_b"),
      ::rust::String("tag_c"),
      ::rust::String("tag_d"),
  });

  return RustEnum(d);
}

void take_enum(const RustEnum &enm) {
  std::cout << "The index of enum is " << enm.index() << std::endl;
  const auto visitor = overload{
      [](const ::rust::variant::empty) {
        std::cout << "The value of enum is ::rust::empty" << std::endl;
      },
      [](const ::rust::string &v) {
        std::cout << "The value of enum is string '" << v << "'" << std::endl;
      },
      [](const SharedData &d) {
        std::cout << "The value of enum is SharedData struct { " << std::endl
                  << "\tsize: " << d.size << "," << std::endl
                  << "\ttags: [";
        for (const auto &tag : d.tags) {
          std::cout << std::endl << "\t\t\"" << tag << "\", ";
        }
        std::cout << std::endl << "\t]," << std::endl << "}" << std::endl;
      },
      [](const auto &v) {
        std::cout << "The value of enum is " << v << std::endl;
      },
  };
  rust::variant::visit(visitor, enm);
}

void take_mut_enum(RustEnum &enm) {
  take_enum(enm);
  if (!::rust::variant::holds_alternative<bool>(enm)) {
    enm = false;
  } else {
    enm = int64_t(111);
  }
}
