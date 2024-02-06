{pkgs ? import <nixpkgs> {}}:

pkgs.mkShell {
	packages = [ pkgs.cglm pkgs.glfw];
}