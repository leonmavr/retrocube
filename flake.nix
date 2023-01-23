# If you run with Nix and you have "flakes" enabled,
# you can just run "nix run" and it will run the default executable here ("cube").
# Also see the note(s) at the top of default.nix.
{
  inputs = {
    # nixpkgs.url = github:NixOS/nixpkgs/nixos-21.11;
    nixpkgs.url = "nixpkgs";
    flake-utils.url = github:numtide/flake-utils;
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      rec {
        packages.default = (import ./default.nix)
          { pkgs = nixpkgs.legacyPackages.${system}; };

        apps = rec {
          default = retrocube;
          retrocube = {
            type = "app";
            program = "${self.packages.${system}.default}/bin/cube";
          };
        };
      }
    );
}
