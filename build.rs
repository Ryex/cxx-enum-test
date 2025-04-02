use cxx_build::CFG;

fn main() {
    // CFG.change_detection = true;
    CFG.doxygen = true;

    cxx_build::bridge("src/bridge.rs") // returns a cc::Build
        .include("include/")
        .file("src/bridge.cc")
        .cpp(true)
        .std("c++17")
        .flag_if_supported("/Zc:__cplusplus")
        .compile("cxx_enum_test");

    println!("cargo:rerun-if-changed=src/bridge.rs");
    println!("cargo:rerun-if-changed=src/bridge.cc");
    println!("cargo:rerun-if-changed=include/bridge.h");
}
