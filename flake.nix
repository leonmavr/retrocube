{
	inputs = {
		nixpkgs.url = github:nixos/nixpkgs/release-23.05;
		flake-utils.url = github:numtide/flake-utils;
	};
	outputs = {
		self,
		nixpkgs,
		flake-utils,
	}:
		flake-utils.lib.eachDefaultSystem (system: let
			pkgs = nixpkgs.legacyPackages.${system};
		in {
			# usable with nix when flake are enabled with
			# `nix build` # to build the default package
			# `nix run` # to run the default executable (cube) in the default package
			packages = rec {
				retrocube = pkgs.callPackage ./package.nix {};
				default = retrocube;
			};
		});
}
