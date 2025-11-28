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

## Disclosures

[Stockfish](https://github.com/official-stockfish/Stockfish) was used as a base engine with 
adaptations made to support and optimize for the force-capture variant. Jochen uses the
pre-trained NNUE provided by Stockfish (which was not trained specifically for the force-capture
variant).

[Fairy-Stockfish](https://github.com/fairy-stockfish/Fairy-Stockfish) was used solely as a
benchmark for Jochen to play against. No code or pre-trained NNUE were taken from Fairy-Stockfish.

OpenAI's ChatGPT and Anthropic's Claude were used solely to understand pre-existing code in the 
Stockfish repository, searching mechanics at a general level, and provide suggestions on values of
tunable parameters. 


