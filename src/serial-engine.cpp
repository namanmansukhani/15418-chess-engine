#include "serial-engine.h"
#include <algorithm> // For std::max and std::min

SerialEngine::Score SerialEngine::static_eval(thc::ChessRules& cr) {
    Score white_score = 0.0f;
    Score black_score = 0.0f;

    for (int i = 0; i < 64; i++) {
        char piece = cr.squares[i];
        switch (piece) {
            case 'P':
                white_score += 1.0f;
                break;
            case 'N':
                white_score += 3.0f;
                break;
            case 'B':
                white_score += 3.0f;
                break;
            case 'R':
                white_score += 5.0f;
                break;
            case 'Q':
                white_score += 9.0f;
                break;
            case 'p':
                black_score += 1.0f;
                break;
            case 'n':
                black_score += 3.0f;
                break;
            case 'b':
                black_score += 3.0f;
                break;
            case 'r':
                black_score += 5.0f;
                break;
            case 'q':
                black_score += 9.0f;
                break;
            default:
                break;
        }
    }

    return white_score - black_score;
}

SerialEngine::Score SerialEngine::solve_serial_engine(thc::ChessRules cr, bool is_white_player, thc::Move& best_move, int depth, Score alpha_score, Score beta_score) {
    if (depth == MAX_DEPTH) {
        return static_eval(cr);
    }

    thc::DRAWTYPE draw_reason;
    if (cr.IsDraw(false, draw_reason)) {
        return 0.0f;
    }

    // Check for checkmate or stalemate
    thc::TERMINAL terminal;
    if (cr.Evaluate(terminal)) {
        if (terminal == thc::TERMINAL_WCHECKMATE) {
            return -INF_SCORE; // White is checkmated
        } else if (terminal == thc::TERMINAL_BCHECKMATE) {
            return INF_SCORE; // Black is checkmated
        } else if (terminal == thc::TERMINAL_WSTALEMATE || terminal == thc::TERMINAL_BSTALEMATE) {
            return 0.0f; // Stalemate is a draw
        }
    }

    std::vector<thc::Move> legal_moves;
    cr.GenLegalMoveList(legal_moves);

    if (legal_moves.empty()) {
        // No legal moves: checkmate or stalemate
        return 0.0f;
    }

    int best_move_id = -1;
    Score best_score = is_white_player ? -INF_SCORE : INF_SCORE;

    for (size_t i = 0; i < legal_moves.size(); i++) {
        // Push the move
        cr.PushMove(legal_moves[i]);

        // Recurse
        thc::Move temp_best_move;
        Score current_score = solve_serial_engine(cr, !is_white_player, temp_best_move, depth + 1, alpha_score, beta_score);

        // Pop the move
        cr.PopMove(legal_moves[i]);

        if (is_white_player) {
            if (current_score > best_score) {
                best_move_id = i;
                best_score = current_score;
                alpha_score = std::max(alpha_score, best_score);
            }
            if (beta_score <= alpha_score) {
                break; // Beta cutoff
            }
        } else {
            if (current_score < best_score) {
                best_move_id = i;
                best_score = current_score;
                beta_score = std::min(beta_score, best_score);
            }
            if (beta_score <= alpha_score) {
                break; // Alpha cutoff
            }
        }
    }

    if (depth == 0 && best_move_id != -1) {
        best_move = legal_moves[best_move_id];
    }

    return best_score;
}

thc::Move SerialEngine::solve(thc::ChessRules& cr, bool is_white_player) {
    thc::Move best_move;
    auto score = solve_serial_engine(cr, is_white_player, best_move, 0, -INF_SCORE, INF_SCORE);
    return best_move;
}
