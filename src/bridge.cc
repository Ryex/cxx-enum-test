
#include "cxxenum-test/include/bridge.h"
#include <atomic>
#include <functional>
#include <iostream>
RustEnum make_enum() { return RustEnum{RustEnum::Num(1502)}; }
RustEnum make_enum_str() {
  return RustEnum(RustEnum::String("String from c++"));
}
RustEnum make_enum_shared() {
  SharedData d;
  d.size = 4;
  d.tags = ::rust::Vec<::rust::String>({
      RustEnum::String("tag_a"),
      RustEnum::String("tag_b"),
      RustEnum::String("tag_c"),
      RustEnum::String("tag_d"),
  });

  return RustEnum(d);
}

RustEnum make_enum_shared_ref() {
  static std::atomic_bool once;
  static SharedData shared_s;
  if (!once) {
    once = true;
    shared_s = {5, ::rust::Vec<::rust::String>({
                       RustEnum::String("tag_a"),
                       RustEnum::String("tag_b"),
                       RustEnum::String("tag_c"),
                       RustEnum::String("tag_d"),
                       RustEnum::String("tag_e"),
                   })};
  }

  return RustEnum(std::reference_wrapper<SharedData>(shared_s));
}

RustEnum make_enum_opaque() { return RustEnum(new_rust_value()); }

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
      [](const RustEnum::SharedRef &d) {
        std::cout << "The value of enum is SharedDataRef struct { " << std::endl
                  << "\tsize: " << d.get().size << "," << std::endl
                  << "\ttags: [";
        for (const auto &tag : d.get().tags) {
          std::cout << std::endl << "\t\t\"" << tag << "\", ";
        }
        std::cout << std::endl << "\t]," << std::endl << "}" << std::endl;
      },
      [](const RustEnum::Opaque &d) {
        std::cout << "The value of enum is Opaque '" << d->read() << "'"
                  << std::endl;
      },
      [](const RustEnum::OpaqueRef &d) {
        std::cout << "The value of enum is OpaqueRef '" << d.get().read() << "'"
                  << std::endl;
      },
      [](const RustEnum::Tuple &v) {
        std::cout << "The value of enum is Tuple (" << v._0 << ", " << v._1
                  << ")" << std::endl;
      },
      [](const RustEnum::Struct &v) {
        std::cout << "The value of enum is Struct { \n\tval: " << v.val
                  << ",\n\tstr: " << v.str << "\n}" << std::endl;
      },
      [](const RustEnum::Unit1) {
        std::cout << "The value of enum is Unit1" << std::endl;
      },
      [](const RustEnum::Unit2) {
        std::cout << "The value of enum is Unit2" << std::endl;
      },
      [](const auto &v) {
        std::cout << "The value of enum is " << v << std::endl;
      },
  };
  rust::variant::visit(visitor, enm);
}

void take_mut_enum(RustEnum &enm) {
  take_enum(enm);
  if (!::rust::variant::holds_alternative<RustEnum::Bool>(enm)) {
    enm = false;
  } else {
    enm = int64_t(111);
  }
}
