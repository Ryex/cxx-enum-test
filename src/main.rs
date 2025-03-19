use bridge::ffi::SharedData;

mod bridge;

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
    let mut f = bridge::ffi::make_enum();
    match &f {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
    }
    bridge::ffi::take_mut_enum(&mut f);
    match &f {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
    }
    let f = bridge::ffi::make_enum_str();
    match &f {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
    }
    let f = bridge::ffi::make_enum_shared();
    match &f {
        bridge::ffi::RustEnum::Empty => println!("The value is string Empty"),
        bridge::ffi::RustEnum::String(s) => println!("The value is string '{s}'"),
        bridge::ffi::RustEnum::Num(n) => println!("The value is {n}"),
        bridge::ffi::RustEnum::Bool(b) => println!("The value is {b}"),
        bridge::ffi::RustEnum::Shared(d) => println!("The value is Shared Data {d:?}"),
    }
}
