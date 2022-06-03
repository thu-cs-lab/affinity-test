{ pkgs ? import <nixpkgs> {}
}:

pkgs.mkShell {
  buildInputs = [
    pkgs.openmpi
    pkgs.gnuplot
  ];
}