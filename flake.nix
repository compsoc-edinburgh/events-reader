{
  description = "Rust + rp-hal dev shell for Pico 2 W (RP2350)";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    rust-overlay.url = "github:oxalica/rust-overlay";
  };

  outputs = { self, nixpkgs, flake-utils, rust-overlay }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ (import rust-overlay) ];
        pkgs = import nixpkgs { inherit system overlays; };

        rust = pkgs.rust-bin.stable.latest.default.override {
          extensions = [ "rust-src" "llvm-tools-preview" ];
          targets = [ "thumbv6m-none-eabi" "thumbv8m.main-none-eabihf" "riscv32imac-unknown-none-elf" ];
        };
      in {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            rust
            flip-link
            probe-rs-tools
            elf2uf2-rs
            pkg-config
            picotool
          ];

          shellHook = ''
            echo "rp-hal environment ready"
          '';
        };
      });
}
