use cxx::UniquePtr;

#[repr(C)]
#[derive(Debug)]
pub enum RustEnum {
    Empty,
    Num(i64),
    String(String),
    Bool(bool),
    Shared(ffi::SharedData),
    Tuple(i32, i32),
    Struct{
        val: i32,
        str: String
    },
    Unit1,
    Unit2,
}

unsafe impl ::cxx::ExternType for RustEnum {
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

        type RustEnum = crate::bridge::RustEnum;

        pub fn make_enum() -> RustEnum;
        pub fn make_enum_str() -> RustEnum;
        pub fn make_enum_shared() -> RustEnum;
        pub fn take_enum(enm: &RustEnum);
        pub fn take_mut_enum(enm: &mut RustEnum);

    }
}

// #[derive(Default, Clone)]
pub struct RustValue {
    value: String,
}

impl RustValue {
    pub fn read(&self) -> &String {
        &self.value
    }
}


