{
  description = "Nujel";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      # Target system(s) — adjust if you need macOS or other
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
      {
        devShells = forAllSystems (system:
          let
            pkgs = nixpkgs.legacyPackages.${system};
          in
            {
              default = pkgs.mkShell {
                strictDeps = true;
                nativeBuildInputs = with pkgs; [
                  gcc
                  clang
                  mold
                  dash
                  gnumake
                  rlwrap
                  tokei
                ];
              };
            });
      };
}
