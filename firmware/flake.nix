{
  description = "Raspberry Pi Pico 2 W C SDK development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        pico-sdk = pkgs.fetchFromGitHub {
          owner = "raspberrypi";
          repo = "pico-sdk";
          rev = "a1438dff1d38bd9c65dbd693f0e5db4b9ae91779";
          sha256 = "sha256-8ubZW6yQnUTYxQqYI6hi7s3kFVQhe5EaxVvHmo93vgk=";
          fetchSubmodules = true;
        };
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            cmake
            ninja
            git
            python3
            gcc-arm-embedded
            newlib
            minicom
          ];

          PICO_SDK_PATH = pico-sdk;

          shellHook = ''
            echo "Pico 2 W SDK environment ready."
            echo "PICO_SDK_PATH=$PICO_SDK_PATH"
          '';
        };
      });
}
