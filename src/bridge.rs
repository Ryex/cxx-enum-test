#[cxx::bridge]
pub mod ffi {

    #[derive(Debug)]
    struct SharedData {
        size: i64,
        tags: Vec<String>,
    }

    ///our test Variant Enum
    pub enum RustEnum {
        Empty,
        Num(i64),
        /// variant with a String
        String(String),
        Bool(bool),
        Shared(SharedData),
        OpaqueRust(Box<RustValue>),
        OpaqueCpp(UniquePtr<CppValue>),
    }

    pub enum EnumWithRef<'a> {
        Str(&'a str),
    }

    unsafe extern "C++" {
        include!("cxxenum-test/include/bridge.h");

        type CppValue;
        pub fn read(self: &CppValue) -> &CxxString;

        pub fn make_shared_data() -> SharedData;
        pub fn make_cpp_value(value: &CxxString) -> UniquePtr<CppValue>;

        pub fn make_enum() -> RustEnum;
        pub fn make_enum_str() -> RustEnum;
        pub fn make_enum_shared() -> RustEnum;
        pub fn make_enum_opaque_rust() -> RustEnum;
        pub fn make_enum_opaque_cpp() -> RustEnum;

        pub fn take_enum(enm: &RustEnum);
        pub fn take_mut_enum(enm: &mut RustEnum);

        pub fn make_ref_enum() -> EnumWithRef<'static>;
        pub fn take_ref_enum(enm: &EnumWithRef);

    }

    extern "Rust" {
        type RustValue;
        fn new_opaque_rust_value(value: String) -> Box<RustValue>;
        fn read(&self) -> &str;
    }
}

#[derive(Default, Clone, Debug)]
pub struct RustValue {
    value: String,
}

fn new_opaque_rust_value(value: String) -> Box<RustValue> {
    RustValue::new(value)
}

impl RustValue {
    pub fn new(value: String) -> Box<Self> {
        Box::new(RustValue { value })
    }
    pub fn read(&self) -> &str {
        &self.value
    }
}
