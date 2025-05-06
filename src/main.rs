use bridge::ffi::SharedData;

mod bridge;

use bridge::{RustEnum, RustValue};

fn print_enum(enm: &RustEnum) {
    match &enm {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
        _ => println!("The value is {enm:?}"),
    }
}

fn main() {
    println!("Hello, world!");
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
    {
        let shared = SharedData {
            size: 2,
            tags: vec!["rust_tag_1".into(), "rust_tag_2".into(), "rust_tag_3".into()],
        };
        let f = bridge::ffi::RustEnum::SharedRef(&shared);
        bridge::ffi::take_enum(&f);
    }

    let f = RustEnum::Opaque(Box::new(RustValue::new("A Hidden Rust String from Rust")));
    bridge::ffi::take_enum(&f);
    {
        let opaque = RustValue::new("A Hidden Rust String from Rust behind ref");
        let f = RustEnum::OpaqueRef(&opaque);

        bridge::ffi::take_enum(&f);
    }

    let f = bridge::ffi::RustEnum::Tuple(42, 1377);
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Struct {
        val: 70,
        str: "Also a Rust String".into(),
    };
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Unit1;
    bridge::ffi::take_enum(&f);
    let f = bridge::ffi::RustEnum::Unit2;
    bridge::ffi::take_enum(&f);
    let mut f = bridge::ffi::make_enum();
    print_enum(&f);
    bridge::ffi::take_mut_enum(&mut f);
    print_enum(&f);
    let f = bridge::ffi::make_enum_str();
    print_enum(&f);
    let f = bridge::ffi::make_enum_shared();
    print_enum(&f);
    let f = bridge::ffi::make_enum_shared_ref();
    print_enum(&f);
    let f = bridge::ffi::make_enum_opaque();
    print_enum(&f);
}
