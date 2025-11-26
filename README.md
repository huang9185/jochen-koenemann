# jochen-koenemann

## MacOS setup instructions
0. Ensure Homebrew is installed.
1. Install `xboard` via `brew install xboard`. Launch the program with `xboard`.
1. Since macOS does not ship with `aplay`, to get sounds, go to "Options" > "Sounds", and change
the command from `aplay -q` to `afplay`.
1. To launch a game with a 1 minute time limit and two specified chess program, run
```bash
xboard -fcp "first chess program path" -scp "second chess program path" -tc 1 -mode "TwoMachines"
```
Note that we must include `fUCI` or `sUCI` if the first or second engine is a UCI engine (which
is the case for stockfish.

For instance, running stockfish against fairymax would be
```bash
xboard -fcp "path/to/stockfish" -fUCI -scp "fairymax" -tc 1 -mode "TwoMachines"
```
