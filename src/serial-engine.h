#ifndef SERIAL_ENGINE_H
#define SERIAL_ENGINE_H

#include "thc.h" // Include the THC library header

class SerialEngine {
public:
    using Score = float;

    static constexpr Score INF_SCORE = 1000000.0f;
    static constexpr int MAX_DEPTH = 3;

    thc::Move solve(thc::ChessRules& cr, bool is_white_player);

private:
    Score solve_serial_engine(thc::ChessRules cr, bool is_white_player, thc::Move& best_move, int depth, Score alpha_score, Score beta_score);
    Score static_eval(thc::ChessRules& cr);
};

#endif // SERIAL_ENGINE_H
