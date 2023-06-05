# To build this with on nix, just run
# nix build # with flakes
# nix-build --expr "with import <nixpkgs> {}; callPackage ./package.nix {}" # without flakes
{
	lib,
	# override this with fastStdenv for optimization/faster running times (8-12%) BUT... nondeterministic builds :(
	stdenv,
	...
}:
stdenv.mkDerivation rec {
	pname = "retrocube";
	version = "1.0";
	# dont include nix and version control files in the source
	src = lib.cleanSourceWith {
		filter = name: _: let
			n = baseNameOf (toString name);
		in
			!(lib.hasSuffix ".nix" n)
			&& !(lib.hasSuffix ".lock" n);
		src = lib.cleanSource ./.;
	};
	# disable the buildPhase, because make will install the meshes while building,
	# which won't work in the buildPhase, so we will build in the installPhase
	dontBuild = true;
	# build and install retrocube
	installPhase = ''
		make PREFIX=$out
		mkdir $out/bin
		cp cube $out/bin
	'';

	meta = with lib; {
		mainProgram = "cube";
		license = licenses.mit;
	};
}
