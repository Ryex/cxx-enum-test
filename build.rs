use cxx_build::CFG;

fn main() {
    CFG.doxygen = true;

    cxx_build::bridge("src/bridge.rs")// returns a cc::Build
        .file("src/cxxvariant.cc")
        .file("src/bridge.cc")
        .cpp(true)
        .std("c++17")
        .compile("cxx_enum_test");

    println!("cargo:rerun-if-changed=src/bridge.rs");
    println!("cargo:rerun-if-changed=src/cxxvariant.cc");
    println!("cargo:rerun-if-changed=src/bridge.cc");
    println!("cargo:rerun-if-changed=include/cxxvariant.h");
    println!("cargo:rerun-if-changed=include/bridge.h");
}
