/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2025 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "evaluate.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>

#include "bitboard.h"
#include "movegen.h"
#include "nnue/network.h"
#include "nnue/nnue_misc.h"
#include "position.h"
#include "types.h"
#include "uci.h"
#include "nnue/nnue_accumulator.h"

namespace Stockfish {

int capture_pressure(const Position& pos) {
    int white_pressure = 0;
    int black_pressure = 0;
    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Piece pc = pos.piece_on(s);
        if (pc == NO_PIECE)
        {
            continue;
        }
        Color piece_color = color_of(pc);
        if (pos.attackers_to(s, ~piece_color))
        {
            int val;
            switch (type_of(pc))
            {
            case PAWN :
                val = 100;
                break;
            case KNIGHT :
                val = 320;
                break;
            case BISHOP :
                val = 330;
                break;
            case ROOK :
                val = 500;
                break;
            case QUEEN :
                val = 900;
                break;
            default :
                val = 0;
                break;
            }
            if (piece_color == WHITE)
                white_pressure += val;
            else
                black_pressure += val;
        }
    }
    return (pos.side_to_move() == WHITE ? 1 : -1) * black_pressure - white_pressure;
}

// Adjust for opponent's capturing options
int forced_capture_penalty(const Position& pos) {
    bool has_capture   = false;
    bool has_quiet     = false;
    int  capture_count = 0;
    for (const auto& move : MoveList<LEGAL>(pos))
    {
        if (pos.capture_stage(move))
        {
            has_capture = true;
            capture_count++;
        }
        else
        {
            has_quiet = true;
        }
    }
    if (has_capture && !has_quiet)
    {
        // tunable
        int penalty = 15;

        // all tunable
        switch (capture_count)
        {
        case 1 :
            penalty += 25;
            break;
        case 2 :
            penalty += 15;
            break;
        case 3 :
            penalty += 8;
            break;
        default :
            break;
        }
        return -penalty;
    }
    return 0;
}

int hanging_penalty(const Position& pos) {
    int   penalty = 0;
    Color me      = pos.side_to_move();

    for (Square s = SQ_A1; s <= SQ_H8; ++s)
    {
        Piece pc = pos.piece_on(s);
        if (pc != NO_PIECE && color_of(pc) == me)
        {
            Bitboard attacker = pos.attackers_to(s, ~me);
            Bitboard defender = pos.attackers_to(s, me);
            if (attacker && popcount(attacker) > popcount(defender))
            {
                int pieceValue = type_of(pc) == PAWN   ? 100
                               : type_of(pc) == KNIGHT ? 320
                               : type_of(pc) == BISHOP ? 330
                               : type_of(pc) == ROOK   ? 500
                               : type_of(pc) == QUEEN  ? 900
                                                       : 0;
                // tunable
                penalty += pieceValue * 1 / 3;
            }
        }
    }
    return -penalty;
}

int forced_exchange_eval(const Position& pos) {

    if (!pos.has_capture_moves())
    {
        return 0;
    }
    int   eval            = 0;
    Color me              = pos.side_to_move();
    int   best_safe_cap   = -0x3f3f3f3f;
    int   best_unsafe_cap = -0x3f3f3f3f;
    for (const auto& move : MoveList<CAPTURES>(pos))
    {
        if (!pos.legal(move))
        {
            continue;
        }

        Piece captured    = pos.piece_on(move.to_sq());
        int   capture_val = type_of(captured) == PAWN   ? 100
                          : type_of(captured) == KNIGHT ? 320
                          : type_of(captured) == BISHOP ? 330
                          : type_of(captured) == ROOK   ? 500
                          : type_of(captured) == QUEEN  ? 900
                                                        : 0;
        Piece mover       = pos.moved_piece(move);
        int   move_val    = type_of(mover) == PAWN   ? 100
                          : type_of(mover) == KNIGHT ? 320
                          : type_of(mover) == BISHOP ? 330
                          : type_of(mover) == ROOK   ? 500
                          : type_of(mover) == QUEEN  ? 900
                                                     : 0;

        int  imm_val   = capture_val - move_val;
        bool can_recap = pos.attackers_to(move.to_sq(), ~me);
        if (can_recap)
        {
            best_unsafe_cap = std::max(imm_val, best_unsafe_cap);
        }
        else
        {
            best_safe_cap = std::max(capture_val, best_safe_cap);
        }
    }

    if (best_safe_cap > -0x3f3f3f3f)
    {
        // tunable
        eval += std::min(best_safe_cap / 10, 50);
    }
    else if (best_unsafe_cap < 0)
    {
        eval += best_unsafe_cap / 2;
    }
    else if (best_unsafe_cap == 0)
    {
        eval -= 10;
    }
    return eval;
}

// Returns a static, purely materialistic evaluation of the position from
// the point of view of the side to move. It can be divided by PawnValue to get
// an approximation of the material advantage on the board in terms of pawns.
int Eval::simple_eval(const Position& pos) {
    Color c             = pos.side_to_move();
    int   material_eval = PawnValue * (pos.count<PAWN>(c) - pos.count<PAWN>(~c))
                      + (pos.non_pawn_material(c) - pos.non_pawn_material(~c));

    // return material_eval + hanging_penalty(pos);
        // + forced_capture_penalty(pos);
    return material_eval;
}

bool Eval::use_smallnet(const Position& pos) {
    // reduced cuz want to use big network more often
    // tunable
    return std::abs(simple_eval(pos)) > 800;
}

// Evaluate is the evaluator for the outer world. It returns a static evaluation
// of the position from the point of view of the side to move.
Value Eval::evaluate(const Eval::NNUE::Networks&    networks,
                     const Position&                pos,
                     Eval::NNUE::AccumulatorStack&  accumulators,
                     Eval::NNUE::AccumulatorCaches& caches,
                     int                            optimism) {

    assert(!pos.checkers());

    bool smallNet           = use_smallnet(pos);
    auto [psqt, positional] = smallNet ? networks.small.evaluate(pos, accumulators, caches.small)
                                       : networks.big.evaluate(pos, accumulators, caches.big);

    Value nnue = (125 * psqt + 131 * positional) / 128;

    // Re-evaluate the position when higher eval accuracy is worth the time spent
    if (smallNet && (std::abs(nnue) < 277))
    {
        std::tie(psqt, positional) = networks.big.evaluate(pos, accumulators, caches.big);
        nnue                       = (125 * psqt + 131 * positional) / 128;
        smallNet                   = false;
    }

    // Blend optimism and eval with nnue complexity
    int nnueComplexity = std::abs(psqt - positional);
    optimism += optimism * nnueComplexity / 476;
    nnue -= nnue * nnueComplexity / 18236;

    int material = 534 * pos.count<PAWN>() + pos.non_pawn_material();
    int v        = (nnue * (77871 + material) + optimism * (7191 + material)) / 77871;

    // tunable
    v += capture_pressure(pos) / 10;
    v += forced_capture_penalty(pos);
    // apparently makes the engine much worse so disabled
    // v += hanging_penalty(pos) / 3;
    // v += forced_exchange_eval(pos)/3;

    // Damp down the evaluation linearly when shuffling
    // tunable, reduced since non progressive moves are less common
    v -= v * pos.rule50_count() / 199;

    // Guarantee evaluation does not hit the tablebase range
    v = std::clamp(v, VALUE_TB_LOSS_IN_MAX_PLY + 1, VALUE_TB_WIN_IN_MAX_PLY - 1);

    return v;
}

// Like evaluate(), but instead of returning a value, it returns
// a string (suitable for outputting to stdout) that contains the detailed
// descriptions and values of each evaluation term. Useful for debugging.
// Trace scores are from white's point of view
std::string Eval::trace(Position& pos, const Eval::NNUE::Networks& networks) {

    if (pos.checkers())
        return "Final evaluation: none (in check)";

    auto accumulators = std::make_unique<Eval::NNUE::AccumulatorStack>();
    auto caches       = std::make_unique<Eval::NNUE::AccumulatorCaches>(networks);

    std::stringstream ss;
    ss << std::showpoint << std::noshowpos << std::fixed << std::setprecision(2);
    ss << '\n' << NNUE::trace(pos, networks, *caches) << '\n';

    ss << std::showpoint << std::showpos << std::fixed << std::setprecision(2) << std::setw(15);

    auto [psqt, positional] = networks.big.evaluate(pos, *accumulators, caches->big);
    Value v                 = psqt + positional;
    v                       = pos.side_to_move() == WHITE ? v : -v;
    ss << "NNUE evaluation        " << 0.01 * UCIEngine::to_cp(v, pos) << " (white side)\n";

    v = evaluate(networks, pos, *accumulators, *caches, VALUE_ZERO);
    v = pos.side_to_move() == WHITE ? v : -v;
    ss << "Final evaluation       " << 0.01 * UCIEngine::to_cp(v, pos) << " (white side)";
    ss << " [with scaled NNUE, ...]";
    ss << "\n";

    return ss.str();
}

}  // namespace Stockfish
