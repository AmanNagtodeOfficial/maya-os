{
  pkgs, ... }:
{
  packages = [
    pkgs.gcc
    pkgs.gdb
    pkgs.qemu
    pkgs.make
    pkgs.nasm
    pkgs.grub
    pkgs.xorriso
    pkgs.binutils
  ];

  idx.extensions = [
    "ms-vscode.cpptools"
  ];

  idx.workspace = {
    onStart = {
      debug = "tools/debug.sh"; # Assuming you have a debug script
    };
  };
}
