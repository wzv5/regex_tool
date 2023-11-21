fn main() {
    let _ = cxx_build::bridge("src/cppbridge.rs");
    println!("cargo:rerun-if-changed=src/cppbridge.rs");
}
