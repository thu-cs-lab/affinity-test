with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "affinity-test";
  version = "1.0";

  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    openmpi
  ];
}

