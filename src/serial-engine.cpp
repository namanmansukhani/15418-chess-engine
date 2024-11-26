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

#include "serial-engine.h"
#include <algorithm>
#include <map>
#include <cctype> // For isupper and islower
#include <cmath>  // For abs

// Piece-square tables and other code remain unchanged

SerialEngine::Score SerialEngine::static_eval(thc::ChessRules& cr) {
    Score total_score = 0.0f;
    int total_material = 0;

    // Variables to store king positions
    int white_king_index = -1;
    int black_king_index = -1;

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
                total_material += 1;
                break;
            case 'N':
                piece_value = 3.0f;
                positional_bonus = knight_table[index] / 100.0f;
                total_material += 3;
                break;
            case 'B':
                piece_value = 3.0f;
                positional_bonus = bishop_table[index] / 100.0f;
                total_material += 3;
                break;
            case 'R':
                piece_value = 5.0f;
                positional_bonus = rook_table[index] / 100.0f;
                total_material += 5;
                break;
            case 'Q':
                piece_value = 9.0f;
                positional_bonus = queen_table[index] / 100.0f;
                total_material += 9;
                break;
            case 'K':
                piece_value = 1000.0f; // Assign a high value to the King
                positional_bonus = king_table[index] / 100.0f;
                white_king_index = i;
                break;
            case 'p':
                piece_value = -1.0f;
                positional_bonus = -pawn_table[flipped_index] / 100.0f;
                total_material += 1;
                break;
            case 'n':
                piece_value = -3.0f;
                positional_bonus = -knight_table[flipped_index] / 100.0f;
                total_material += 3;
                break;
            case 'b':
                piece_value = -3.0f;
                positional_bonus = -bishop_table[flipped_index] / 100.0f;
                total_material += 3;
                break;
            case 'r':
                piece_value = -5.0f;
                positional_bonus = -rook_table[flipped_index] / 100.0f;
                total_material += 5;
                break;
            case 'q':
                piece_value = -9.0f;
                positional_bonus = -queen_table[flipped_index] / 100.0f;
                total_material += 9;
                break;
            case 'k':
                piece_value = -1000.0f; // Assign a high negative value to the King
                positional_bonus = -king_table[flipped_index] / 100.0f;
                black_king_index = i;
                break;
            default:
                break;
        }

        Score square_score = piece_value + positional_bonus;
        total_score += square_score;
    }

    // Determine if it's the endgame
    bool is_endgame = total_material <= 14; // Adjust threshold as needed

    // Add evaluation for enemy king being near the edge
    if (black_king_index != -1) {
        int black_king_rank = black_king_index / 8;
        int black_king_file = black_king_index % 8;

        int distance_to_edge = std::min({black_king_rank, 7 - black_king_rank, black_king_file, 7 - black_king_file});

        // The closer the enemy king is to the edge, the higher the bonus
        float king_edge_bonus = (3 - distance_to_edge) * 0.2f; // Adjust scaling factor as needed
        total_score += king_edge_bonus;
    }

    // Add evaluation for king proximity during endgame
    if (is_endgame && white_king_index != -1 && black_king_index != -1) {
        int white_king_rank = white_king_index / 8;
        int white_king_file = white_king_index % 8;
        int black_king_rank = black_king_index / 8;
        int black_king_file = black_king_index % 8;

        // Calculate Manhattan distance between the kings
        int king_distance = std::abs(white_king_rank - black_king_rank) + std::abs(white_king_file - black_king_file);

        // The closer the kings are, the higher the bonus
        float king_proximity_bonus = (14 - king_distance) * 0.1f; // Adjust scaling factor as needed
        total_score += king_proximity_bonus;
    }

    return total_score;
}


float score_move(const thc::Move& move, thc::ChessRules& cr) {
    float score = 0.0f;

    // Check if the move is a capture
    if (move.capture != ' ') {
        // Assign a higher score for capturing higher-value pieces
        switch (move.capture) {
            case 'p': score += 1.0f; break;
            case 'n': score += 3.0f; break;
            case 'b': score += 3.0f; break;
            case 'r': score += 5.0f; break;
            case 'q': score += 9.0f; break;
            case 'k': score += 1000.0f; break; // Very high for king (illegal in chess but handled for consistency)
        }
    }

    // Check if the move results in a pawn promotion
    if (move.special == thc::SPECIAL_PROMOTION_QUEEN) {
        score += 9.0f; // Promoting to a queen
    } else if (move.special == thc::SPECIAL_PROMOTION_ROOK) {
        score += 5.0f; // Promoting to a rook
    } else if (move.special == thc::SPECIAL_PROMOTION_BISHOP) {
        score += 3.0f; // Promoting to a bishop
    } else if (move.special == thc::SPECIAL_PROMOTION_KNIGHT) {
        score += 3.0f; // Promoting to a knight
    }

    // Add positional bonus based on piece-square tables
    int from_index = static_cast<int>(move.src);
    int to_index = static_cast<int>(move.dst);
    char piece = cr.squares[from_index];

    if (isupper(piece)) { // White pieces
        switch (piece) {
            case 'P': score += (pawn_table[to_index] - pawn_table[from_index]) / 100.0f; break;
            case 'N': score += (knight_table[to_index] - knight_table[from_index]) / 100.0f; break;
            case 'B': score += (bishop_table[to_index] - bishop_table[from_index]) / 100.0f; break;
            case 'R': score += (rook_table[to_index] - rook_table[from_index]) / 100.0f; break;
            case 'Q': score += (queen_table[to_index] - queen_table[from_index]) / 100.0f; break;
            case 'K': score += (king_table[to_index] - king_table[from_index]) / 100.0f; break;
        }
    } else { // Black pieces
        int flipped_from_index = 63 - from_index;
        int flipped_to_index = 63 - to_index;

        switch (piece) {
            case 'p': score += (pawn_table[flipped_to_index] - pawn_table[flipped_from_index]) / 100.0f; break;
            case 'n': score += (knight_table[flipped_to_index] - knight_table[flipped_from_index]) / 100.0f; break;
            case 'b': score += (bishop_table[flipped_to_index] - bishop_table[flipped_from_index]) / 100.0f; break;
            case 'r': score += (rook_table[flipped_to_index] - rook_table[flipped_from_index]) / 100.0f; break;
            case 'q': score += (queen_table[flipped_to_index] - queen_table[flipped_from_index]) / 100.0f; break;
            case 'k': score += (king_table[flipped_to_index] - king_table[flipped_from_index]) / 100.0f; break;
        }
    }

    return score;
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

    // Assign scores to moves
    std::vector<std::pair<float, thc::Move>> scored_moves;
    for (const auto& move : legal_moves) {
        float score = score_move(move, cr);
        scored_moves.emplace_back(score, move);
    }

    // Sort moves by descending score using a custom comparator
    std::sort(scored_moves.begin(), scored_moves.end(), [](const std::pair<float, thc::Move>& a, const std::pair<float, thc::Move>& b) {
        return a.first > b.first;
    });

    int best_move_id = -1;
    Score best_score = is_white_player ? -INF_SCORE : INF_SCORE;

    for (size_t i = 0; i < scored_moves.size(); i++) {
        auto& move = scored_moves[i].second; // Ensure 'move' is non-const

        // Push the move
        cr.PushMove(move);

        // Recurse
        thc::Move temp_best_move;
        Score current_score = solve_serial_engine(cr, !is_white_player, temp_best_move, depth + 1, alpha_score, beta_score);

        // Pop the move
        cr.PopMove(move);

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
        best_move = scored_moves[best_move_id].second;
    }

    return best_score;
}


thc::Move SerialEngine::solve(thc::ChessRules& cr, bool is_white_player) {
    thc::Move best_move;
    auto score = solve_serial_engine(cr, is_white_player, best_move, 0, -INF_SCORE, INF_SCORE);
    return best_move;
}
