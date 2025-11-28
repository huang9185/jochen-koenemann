# jochen-koenemann

## MacOS setup instructions
0. Ensure Homebrew is installed.
1. Install `xboard` via `brew install xboard`. Launch the program with `xboard`.
1. Since macOS does not ship with `aplay`, to get sounds, go to "Options" > "Sounds", and change
the command from `aplay -q` to `afplay`.

## Build
Run the script `build.sh` to build both Fairy-Stockfish and Stockfish. Rerun the script if 
changes are made to Stockfish.

## Fight
To have stockfish fight against itself, run the `vs_stockfish.sh` script. For fighting against
fairy-stockfish configured to the must-capture variant, run `vs_fairy.sh`.

Note the `polyglot.ini` file is in each engine's directory.
