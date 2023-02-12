# To run this on nix/nixos, just run `nix-build` in the directory containing this file
# and then run any executable in the result/bin directory,
# e.g. `./result/bin/cube`
# Or, if there is a corresponding flake.nix file and you have "flakes" enabled,
# you can just run "nix run" and it will run the default executable ("cube").

{ pkgs ? import <nixpkgs> {}}:
with pkgs;
# fastStdenv.mkDerivation for optimization/faster running times (8-12%) BUT... nondeterministic builds :(
# Otherwise just use stdenv.mkDerivation
fastStdenv.mkDerivation {
  name = "retrocube";
  src = ./.;
  
  enableParallelBuilding = true;

  # any dependencies/build tools needed at compilation/build time
  nativeBuildInputs = [ pkg-config gcc ];

  # any dependencies needed at runtime
  # (why is it not called "runtimeInputs"? and the build time inputs just called "buildInputs"?)
  buildInputs = [ ];

  # the bash shell steps to build it
  buildPhase = ''
    make all
  '';

  # for a generic copy to the nix store of all compiled executables:
  # cp $(find * -maxdepth 1 -executable -type f) $out/bin/
  installPhase = ''
    mkdir -p $out/bin
    cp $(find * -maxdepth 1 -executable -type f) $out/bin/
  '';
}
