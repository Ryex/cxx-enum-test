use cxx::UniquePtr;

#[repr(C)]
pub enum RustEnum {
    Empty,
    Num(i64),
    String(std::string::String),
    Bool(bool),
}

unsafe impl ::cxx::ExternType for RustEnum {
    type Id = ::cxx::type_id!("RustEnum"); // see src/bridge.cc
    type Kind = ::cxx::kind::Trivial;
}


#[cxx::bridge]
pub mod ffi {
    
    struct SharedData {
        size: i64,
        tags: Vec<String>,
    }

    unsafe extern "C++" {
        include!("cxx_enum_test/include/bridge.h");

        type RustEnum = crate::bridge::RustEnum;

        pub fn make_enum() -> RustEnum;
        pub fn make_enum_str() -> RustEnum;
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


