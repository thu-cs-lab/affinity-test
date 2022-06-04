{ pkgs ? import <nixpkgs> {}
}:

pkgs.mkShell {
  buildInputs = [
    pkgs.cmake
    pkgs.openmpi
    pkgs.gnuplot
  ];
}
