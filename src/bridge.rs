use cxx::UniquePtr;

#[repr(C)]
#[derive(Debug)]
pub enum RustEnum<'a> {
    Empty,
    Num(i64),
    String(String),
    Bool(bool),
    Shared(ffi::SharedData),
    SharedRef(&'a ffi::SharedData),
    Opaque(Box<RustValue>),
    OpaqueRef(&'a RustValue),
    Tuple(i32, i32),
    Struct { val: i32, str: String },
    Unit1,
    Unit2,
}

unsafe impl<'a> ::cxx::ExternType for RustEnum<'a> {
    type Id = ::cxx::type_id!("RustEnum"); // see src/bridge.cc
    type Kind = ::cxx::kind::Trivial;
}

#[cxx::bridge]
pub mod ffi {

    #[derive(Debug)]
    struct SharedData {
        size: i64,
        tags: Vec<String>,
    }

    unsafe extern "C++" {
        include!("cxxenum-test/include/bridge.h");

        type RustEnum<'a> = crate::bridge::RustEnum<'a>;

        pub fn make_enum<'a>() -> RustEnum<'a>;
        pub fn make_enum_str<'a>() -> RustEnum<'a>;
        pub fn make_enum_shared<'a>() -> RustEnum<'a>;
        pub fn make_enum_shared_ref<'a>() -> RustEnum<'a>;
        pub fn make_enum_opaque<'a>() -> RustEnum<'a>;
        pub fn take_enum(enm: &RustEnum);
        pub fn take_mut_enum(enm: &mut RustEnum);

    }

    extern "Rust" {
        type RustValue;
        fn read(&self) -> &String;

        fn new_rust_value() -> Box<RustValue>;
    }
}

fn new_rust_value() -> Box<RustValue> {
    Box::new(RustValue {
        value: "A Hidden Rust String".into(),
    })
}

#[derive(Default, Debug, Clone)]
pub struct RustValue {
    value: String,
}

impl RustValue {
    pub fn read(&self) -> &String {
        &self.value
    }
    pub fn new(val: &str) -> Self {
        RustValue {
            value: val.to_string(),
        }
    }
}
