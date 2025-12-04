# Jochen Koenemann
<small>By Tiger Ding and Elyn Huang</small>

## Building instructions

To build the engine, run `build.sh` which contains the following code.

```bash
echo "Building Jochen Koenemann"
cd Jochen/src
make build
make net
```
Note that the `make net` command will download 2 pretrained NNUEs for Stockfish.

The path to the executable will be `Jochen/src/stockfish`.

The `polyglot.ini` file can be found in `Jochen/`, which contains the following content
```
[Polyglot]
EngineCommand=./src/stockfish
EngineDir=.
Log=false
Ponder=true

[Engine]
Threads=1
```

The command used during testing is
```bash
xboard -fcp "polyglot" -fd "./Jochen/" -tc 1 -mode TwoMachines
```

## Disclosures

[Stockfish](https://github.com/official-stockfish/Stockfish) was used as a base engine with 
adaptations made to support and optimize for the force-capture variant. Jochen uses the
pre-trained NNUEs provided by Stockfish (which were not trained specifically for the force-capture
variant).

[Fairy-Stockfish](https://github.com/fairy-stockfish/Fairy-Stockfish) was used solely as a
benchmark for Jochen to play against. No code or pre-trained NNUE were taken from Fairy-Stockfish.

OpenAI's ChatGPT and Anthropic's Claude were used solely to understand pre-existing code in the 
Stockfish repository, searching mechanics at a general level, and provide suggestions on values of
tunable parameters. 


# Development notes

## MacOS setup instructions
0. Ensure Homebrew is installed.
1. Install `xboard` via `brew install xboard`. Launch the program with `xboard`.
1. Since macOS does not ship with `aplay`, to get sounds, go to "Options" > "Sounds", and change
the command from `aplay -q` to `afplay`.

## Build
Run the script `build.sh` to build both Fairy-Stockfish and Stockfish. Rerun the script if 
changes are made to Stockfish.

## Test with cutechess-cli
Follow the instruction here to setup cutechess-cli: https://github.com/cutechess/cutechess under
the current folder. Then, move `cc_test.sh` to /cutechess/build. Run the script file to play 10
games.


## Fight
To have stockfish fight against itself, run the `vs_stockfish.sh` script. For fighting against
fairy-stockfish configured to the must-capture variant, run `vs_fairy.sh`.

Note that Fairy-Stockfish is using the force-capture trained NNUE, which can be downloaded here
https://drive.google.com/u/0/uc?id=1DxWtldVM0TSniPAKzlhAMGYHuZSqpYH7&export=download and should
be placed in the `Fairy-Stockfish/` directory.

Note the `polyglot.ini` file is in each engine's directory.
