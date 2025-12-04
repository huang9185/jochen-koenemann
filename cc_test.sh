./cutechess-cli \
 -engine name=Jochen cmd=polyglot dir="../../Jochen/" proto=xboard \
 -engine name=FairySF cmd=polyglot dir="../../testing/Fairy-Stockfish/" proto=xboard \
 -each tc=60+0 \
 -games 10 \
#  -sprt can be added for hypothesis testing
