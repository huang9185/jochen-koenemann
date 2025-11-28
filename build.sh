echo "Building Stockfish"
cd Stockfish/src
make build
make net
echo "Building Fairy-Stockfish"
cd ../../Fairy-Stockfish/src
make build
make net
# ./stockfish bench

