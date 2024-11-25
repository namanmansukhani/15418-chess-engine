#include <functional>
#include <utility>
#include "serial-engine.h"

SerialEngine::Score SerialEngine::static_eval(thc::ChessRules& cr) {

}

SerialEngine::Score SerialEngine::solve_serial_engine(thc::ChessRules cr, bool is_white_player, thc::Move& best_move, int depth, Score alpha_score, Score beta_score) {
    thc::DRAWTYPE draw_reason;

    if (cr.IsDraw(false, draw_reason)) {
        return 0;
    }

    std::vector<thc::Move> legal_moves;
    std::vector<bool> check;
    std::vector<bool> mate;
    std::vector<bool> stalemate;

    if (depth == MAX_DEPTH) {
        return static_eval(cr);
    }

    cr.GenLegalMoveList(legal_moves, check, mate, stalemate);

    int best_move_id = -1;
    Score best_score = is_white_player ? -INF_SCORE : INF_SCORE;
    
    int num_moves = legal_moves.size();

    for (int i=0;i<num_moves;i++) {
        if (mate[i]) {
            if (is_white_player) return INF_SCORE;
            else return -INF_SCORE;
        }

        // push the move

        Score current_score = solve_serial_engine(cr, !is_white_player, best_move, depth + 1, alpha_score, beta_score);

        if (is_white_player) {
            if (current_score < best_score) {
                best_move_id = i;
                best_score = current_score;

                alpha = 
            }

            if (beta < alpha) {
                // pop the move
                
                break;
            }

            // pop the move
        }
        else {
            // symmetric to white case
        }
    }

    if (depth == 0) {
       best_move = legal_moves[best_move_id];
    }
};

thc::Move SerialEngine::solve(thc::ChessRules& cr, bool is_white_player) {
    thc::Move best_move;
    auto score = solve_serial_engine(cr, is_white_player, best_move, 0, INF_SCORE, INF_SCORE);

    return best_move;
}