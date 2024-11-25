#include "thc.h"


class SerialEngine {
using Score = float;
public:
    float INF_SCORE = 1000000f;
    int MAX_DEPTH = 3;
    thc::Move solve(thc::ChessRules& cr, bool is_white_player);
private:
    Score solve_serial_engine(thc::ChessRules cr, bool is_white_player, thc::Move& best_move, int depth, Score alpha_score, Score beta_score);
    Score static_eval(thc::ChessRules& cr);
};