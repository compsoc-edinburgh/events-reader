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
          rev = "master";
          sha256 = "sha256-hQdEZD84/cnLSzP5Xr9vbOGROQz4BjeVOnvbyhe6rfM=";
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
          ];

          PICO_SDK_PATH = pico-sdk;

          shellHook = ''
            echo "Pico 2 W SDK environment ready."
            echo "PICO_SDK_PATH=$PICO_SDK_PATH"
          '';
        };
      });
}
