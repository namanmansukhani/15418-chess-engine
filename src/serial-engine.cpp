#include "serial-engine.h"
#include <algorithm> 
#include <map>

#include "serial-engine.h"
#include <algorithm>

// Piece-square tables for evaluation
const int pawn_table[64] = {
    // Pawn
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

const int knight_table[64] = {
    // Knight
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishop_table[64] = {
    // Bishop
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int rook_table[64] = {
    // Rook
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

const int queen_table[64] = {
    // Queen
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int king_table[64] = {
    // King
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

SerialEngine::Score SerialEngine::static_eval(thc::ChessRules& cr) {
    Score total_score = 0.0f;

    for (int i = 0; i < 64; i++) {
        char piece = cr.squares[i];
        Score piece_value = 0.0f;
        float positional_bonus = 0.0f;

        // The indices in the piece-square tables correspond to the board from White's perspective.
        // For Black pieces, we need to flip the index.
        int index = i;
        int flipped_index = 63 - i; // Flips the board for Black

        switch (piece) {
            case 'P':
                piece_value = 1.0f;
                positional_bonus = pawn_table[index] / 100.0f;
                break;
            case 'N':
                piece_value = 3.0f;
                positional_bonus = knight_table[index] / 100.0f;
                break;
            case 'B':
                piece_value = 3.0f;
                positional_bonus = bishop_table[index] / 100.0f;
                break;
            case 'R':
                piece_value = 5.0f;
                positional_bonus = rook_table[index] / 100.0f;
                break;
            case 'Q':
                piece_value = 9.0f;
                positional_bonus = queen_table[index] / 100.0f;
                break;
            case 'K':
                piece_value = 1000.0f; // Assign a high value to the King
                positional_bonus = king_table[index] / 100.0f;
                break;
            case 'p':
                piece_value = -1.0f;
                positional_bonus = -pawn_table[flipped_index] / 100.0f;
                break;
            case 'n':
                piece_value = -3.0f;
                positional_bonus = -knight_table[flipped_index] / 100.0f;
                break;
            case 'b':
                piece_value = -3.0f;
                positional_bonus = -bishop_table[flipped_index] / 100.0f;
                break;
            case 'r':
                piece_value = -5.0f;
                positional_bonus = -rook_table[flipped_index] / 100.0f;
                break;
            case 'q':
                piece_value = -9.0f;
                positional_bonus = -queen_table[flipped_index] / 100.0f;
                break;
            case 'k':
                piece_value = -1000.0f; // Assign a high negative value to the King
                positional_bonus = -king_table[flipped_index] / 100.0f;
                break;
            default:
                break;
        }

        Score square_score = piece_value + positional_bonus;
        total_score += square_score;
    }

    return total_score;
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
