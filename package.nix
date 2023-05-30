# To run this on nix/nixos, just run `nix-build` in the directory containing this file
{
	lib,
	fastStdenv,
	pkg-config,
	...
}:
# fastStdenv.mkDerivation for optimization/faster running times (8-12%) BUT... nondeterministic builds :(
# Otherwise just use stdenv.mkDerivation
fastStdenv.mkDerivation rec {
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

	enableParallelBuilding = true;

	# any dependencies/build tools needed at compilation/build time
	# gcc is already part of stdenv
	nativeBuildInputs = [pkg-config];

	# any dependencies needed at runtime
	# (why is it not called "runtimeInputs"? and the build time inputs just called "buildInputs"?)
	# because of legacy reasons
	# https://web.archive.org/web/20230529002701/https://nixos.org/manual/nixpkgs/stable/#variables-specifying-dependencies
	buildInputs = [];

	makeFlags = ["PREFIX=$(out)"];
	meta = with lib; {
		mainProgram = "cube";
		license = licenses.mit;
	};
}
