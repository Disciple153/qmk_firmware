{
  description = "Dev shell for building and flashing the Tzarc Djinn (rev2) QMK firmware";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = pkgs.mkShell {
          name = "djinn-qmk";

          packages = with pkgs; [
            qmk               # QMK CLI: `qmk compile`, `qmk flash`, `qmk lint`, `qmk console`, ...
            gcc-arm-embedded  # arm-none-eabi-gcc/g++/objcopy/size -- Djinn rev2 is an STM32G474 (Cortex-M4)
            dfu-util          # flashes the stm32-dfu bootloader Djinn uses, over USB
            git               # qmk_firmware and its submodules
            clang-tools       # clang-format, used by `qmk format-c`
            diffutils
          ];

          shellHook = ''
            echo ""
            echo "Djinn (rev2) QMK dev shell"
            echo "--------------------------"
            if [ ! -f "Makefile" ] || [ ! -d "keyboards/tzarc/djinn" ]; then
              echo "This doesn't look like a qmk_firmware checkout."
              echo "Clone one (with submodules!) and cd into it, then re-enter this shell:"
              echo ""
              echo "  git clone --recurse-submodules https://github.com/qmk/qmk_firmware"
              echo "  cd qmk_firmware"
              echo ""
            else
              echo "Build:      qmk compile -kb tzarc/djinn/rev2 -km default"
              echo "Compile DB: qmk compile -kb tzarc/djinn/rev2 -km default --compiledb"
              echo "Flash:      qmk flash   -kb tzarc/djinn/rev2 -km default"
              echo ""
              echo "(Put the half you're flashing into its bootloader first: press RESET,"
              echo " or hold the top-left key while plugging it in.)"
            fi
            echo ""
          '';
        };
      });
}
