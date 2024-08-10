# GBA Memory Viewer
This is a small homebrew to view the GBA's Memory. By default, it resides in OVRAM.

## Building
Docker allows easily building without having to install the dependencies.
- To build for the GBA, while in the root of the project, run: `docker run --rm -it -v ${PWD}:/home/builder/pokemon_gen3_to_genx lorenzooone/pokemon_gen3_to_genx:gba_builder`
