# If you run with Nix and you have "flakes" enabled,
# you can just run "nix run" and it will run the default executable here ("cube").
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
			packages = rec {
				retrocube = pkgs.callPackage ./package.nix {};
				default = retrocube;
			};
		});
}
