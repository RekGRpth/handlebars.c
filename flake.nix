{
  description = "handlebars.c";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-23.11";
    systems.url = "github:nix-systems/default";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
    mustache_spec = {
      url = "github:jbboehr/mustache-spec";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    handlebars_spec = {
      url = "github:jbboehr/handlebars-spec";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    gitignore = {
      url = "github:hercules-ci/gitignore.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    pre-commit-hooks = {
      url = "github:cachix/pre-commit-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    nix-github-actions = {
      url = "github:nix-community/nix-github-actions";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    mustache_spec,
    handlebars_spec,
    gitignore,
    pre-commit-hooks,
    nix-github-actions,
    ...
  }:
    flake-utils.lib.eachDefaultSystem
    (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};
        lib = pkgs.lib;

        src = pkgs.lib.cleanSourceWith {
          filter = path: type: (builtins.all (x: x != baseNameOf path)
            [".idea" ".git" "nix" "ci.nix" ".travis.sh" ".travis.yml" ".github" "flake.nix" "flake.lock"]);
          src = gitignore.lib.gitignoreSource ./.;
        };

        pre-commit-check = pre-commit-hooks.lib.${system}.run {
          inherit src;
          hooks = {
            alejandra.enable = true;
            alejandra.excludes = ["\/vendor\/"];
            #clang-format.enable = true;
            #clang-format.types_or = ["c" "c++"];
            #clang-format.files = "\\.(c|h)$";
            markdownlint.enable = true;
            markdownlint.excludes = ["LICENSE.*\.md" "CHANGELOG\.md" "vendor/.*"];
            markdownlint.settings.configuration = {
              MD013 = {
                line_length = 1488;
                # this doesn't seem to work
                table = false;
              };
            };
            #shellcheck.enable = true;
          };
        };

        makeDevShell = package:
          (pkgs.mkShell.override {
            stdenv = package.stdenv;
          }) {
            inputsFrom = [package];
            buildInputs = with pkgs; [
              actionlint
              autoconf-archive
              clang-tools
              editorconfig-checker
              lcov
              gdb
              valgrind
            ];
            shellHook = ''
              ${pre-commit-check.shellHook}
            '';
          };

        makePackage = {...} @ args:
          pkgs.callPackage ./nix/derivation.nix ({
              handlebars_spec = handlebars_spec.packages.${system}.handlebars-spec;
              mustache_spec = mustache_spec.packages.${system}.mustache-spec;
              inherit (gitignore.lib) gitignoreSource;
            }
            // args);

        makeCheck = package: package;

        # @see https://github.com/NixOS/nixpkgs/pull/110787
        # commented out ones are broken :(
        buildConfs = [
          {
            attr = "handlebars-c";
          }
          {
            attr = "handlebars-c-cmake";
            cmakeSupport = true;
          }
          #{
          #  attr = "handlebars-c-norc";
          #  noRefcountingSupport = true;
          #}
          #{
          #  attr = "handlebars-c-debug";
          #  debugSupport = true;
          #  hardeningSupport = false;
          #}
          #{
          #  attr = "handlebars-c-lto";
          #  ltoSupport = true;
          #  sharedSupport = false;
          #}
          {
            attr = "handlebars-c-minimal";
            debugSupport = false;
            hardeningSupport = false;
            jsonSupport = false;
            lmdbSupport = false;
            pthreadSupport = false;
            yamlSupport = false;
            valgrindSupport = false;
          }
          {
            attr = "handlebars-c-static";
            sharedSupport = false;
          }
          #{
          #  attr = "handlebars-c-shared";
          #  staticSupport = false;
          #}
          #{
          #  attr = "handlebars-c-valgrind";
          #  valgrindSupport = true;
          #}
        ];

        buildFn = {attr, ...} @ args:
          lib.nameValuePair
          attr
          (
            makePackage ({} // args)
            #makePackage ({} // (builtins.removeAttrs args ["attr"]))
          );

        packages' = builtins.listToAttrs (builtins.map buildFn buildConfs);
        packages =
          packages'
          // {
            default = packages'.handlebars-c;
          };
      in {
        inherit packages;

        devShells = builtins.mapAttrs (name: package: makeDevShell package) packages;

        checks =
          {inherit pre-commit-check;}
          // (builtins.mapAttrs (name: package: makeCheck package) packages);

        apps.default = {
          type = "app";
          program = "${packages.default.bin}/bin/handlebarsc";
        };

        formatter = pkgs.alejandra;
      }
    )
    // {
      # prolly gonna break at some point
      githubActions.matrix.include = let
        cleanFn = v: v // {name = builtins.replaceStrings ["githubActions." "checks." "x86_64-linux."] ["" "" ""] v.attr;};
      in
        builtins.map cleanFn
        (nix-github-actions.lib.mkGithubMatrix {
          attrPrefix = "checks";
          checks = nixpkgs.lib.getAttrs ["x86_64-linux"] self.checks;
        })
        .matrix
        .include;
    };
}
