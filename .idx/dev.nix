{
  pkgs, ... }:
{
  packages = [
    pkgs.gcc
    pkgs.gdb
    pkgs.qemu
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
