use bridge::ffi::SharedData;

mod bridge;

fn print_enm(enm: &bridge::ffi::RustEnum) {
    match enm {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
        bridge::ffi::RustEnum::OpaqueRust(o) => println!("The value is Opaque RustValue {o:?}"),
        bridge::ffi::RustEnum::OpaqueCpp(o) => {
            println!("The value behind Opaque CppValue is '{}'", o.read())
        }
    }
}

fn print_ref_enm(enm: &bridge::ffi::EnumWithRef) {
    match enm {
        bridge::ffi::EnumWithRef::Str(s) => {
            println!("the value of the str ref in the enum is '{s}'")
        }
    }
}

fn main() {
    println!("Hello, world!");

    let s = bridge::ffi::make_shared_data();
    println!("Shared Data: {:?}", s);
    let f = bridge::ffi::RustEnum::String(String::from("I'm a Rust String"));
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Empty;
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Num(42);
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Shared(SharedData {
        size: 2,
        tags: vec!["rust_tag_1".into(), "rust_tag_2".into()],
    });
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::OpaqueRust(bridge::RustValue::new(String::from(
        "String from Rust behind RustValue",
    )));
    bridge::ffi::take_enum(&f);
    cxx::let_cxx_string!(s = "String form Rust behind CppValue");
    let v = bridge::ffi::make_cpp_value(&s);
    let f = bridge::ffi::RustEnum::OpaqueCpp(v);
    bridge::ffi::take_enum(&f);

    let s = "A Rust Str";
    let f = bridge::ffi::EnumWithRef::Str(s);
    bridge::ffi::take_ref_enum(&f);

    let f = bridge::ffi::make_ref_enum();
    print_ref_enm(&f);

    let mut f = bridge::ffi::make_enum();
    print_enm(&f);
    bridge::ffi::take_mut_enum(&mut f);
    print_enm(&f);
    let f = bridge::ffi::make_enum_str();
    print_enm(&f);
    let f = bridge::ffi::make_enum_shared();
    print_enm(&f);
    let f = bridge::ffi::make_enum_opaque_rust();
    print_enm(&f);
    let f = bridge::ffi::make_enum_opaque_cpp();
    print_enm(&f);
}
