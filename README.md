# Aries

## Build

You need [frill](https://github.com/huisedenanhai/frill) built first, and configure cmake
with `-DARS_FRILL_EXECUTABLE=<path-to-frill-exe>`

## Run

1. Create a folder and put your assets in there. It will be your project folder.
2. Run `aries_importer` with cwd set to the project folder. It will import all your assets to a subdirectory `.ars/`
3. Run `aries_editor` with cwd set to the project folder.

> Executables in playground may not always runnable. They are there for quick and dirty test of work in progress features, and will be removed as soon as those features are easily reachable from editor or launcher.
