/*
 *  serial-engine
 *
 *  Usually, chess engines implement the following algorithms to find moves for each of the different phases of the game.
 * 
 *  Opening: Book (Famous openings are extensively studied and search space is not extensive so we can use a database.) 
 *  Midgame: Search
 *  Endgame: Search + Syzygy (When 8 pieces are left, chess is solved. Just find the correct moves from a syzygy database.)
 * 
 *  So far I only implemented part of Search. The other ones are not related to parallelism.
 * 
 *  Searching is done via alpha-beta pruning. We completely search each layer. We search using iterative deepening 
 *  (DFS version of BFS). For up to 20 seconds total, compute game tree up to depth i and compute the node 
 *  (call static_eval). Then propagate back up. If the 20 seconds are not up, we compute the tree to depth i + 1, and
 *  so on.
 * 
 * 
 *  Book (Unimplemented)
 * 
 *  Search: 
 * 
 *  Iterative deepening (Implemented)
 * 
 *             Timestep 1                              Timestep 2                             Timestep 3 (20 seconds up!)
 *              Depth = 1                               Depth = 2                                     Depth = 3                            
 *             
 *            compute node A                         compute node A                                  compute node A                                  
 *                                             /        |       |     \                           /                  
 *                                          compute node B ... compute node E                   compute node B  X cancel 
 * 
 * 
 *                                                     ^
 *                                                  Use results for this one
 * 
 * 
 *  Static evaluation + Alpha-beta pruning (Implemented)
 * 
 *  When we reach the bottom of the search tree, we will put the board into a static evaluation function. We will use 
 *  minimax to determine what position is best for the next choice. Alpha-beta pruning will be used. 
 * 
 *                          max                     compute node A                                  
 *                           ^                 /        |       |     \                                           
 *                          min              compute node B ... compute node E
 *  To do this we use piece square tables or heat maps as well as add bonuses for things like pawn structure, 
 *  king safety, etc. NNUE (unimplemented) is also used to adjust the scoring. 
 * 
 *  Move reordering (Implemented)
 *  If we search branches with "important" moves first, this will greatly help with alpha-beta pruning. 
 *
 * 
 *  Quiescence Search (Unimplemented)
 * 
 *  At the end of each move we should continue searching until captures are no longer possible.
 * 
 *  Transposition Tables (Unimplemented)
 *  
 *  To help speed up search, different transpositions that have already been scored should be stored in a hash map. This prevents
 *  needing to search the same position twice (DP).
 * 
 *  Syzygy (Unimplemented) 
 */


#include "serial-engine.h"
#include <algorithm>
#include <map>
#include <cctype>   
#include <cmath>    
#include <iostream>

#include <utility>
#include <cassert>

void print(){std::cout<<std::endl;}
void print(bool endline) {if(endline)std::cout<<std::endl;}
template<typename T, typename ...TAIL>
void print(const T &t, TAIL... tail)
{
    std::cout<<t<<' ';
    print(tail...);
}

// Piece-square tables for evaluation (a.k.a. heat maps)
const int pawn_table[64] = {
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
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

/* Helper function for move scoring. Capturing larger piece is prioritized first.
 */

float SerialEngine::score_move(const thc::Move& move, thc::ChessRules& cr) {
    float score = 0.0f;

    // Check if the move is a capture
    if (move.capture != ' ') {
        // Assign a higher score for capturing higher-value pieces
        switch (tolower(move.capture)) {
            case 'p': score += 1.0f; break;
            case 'n': score += 3.0f; break;
            case 'b': score += 3.0f; break;
            case 'r': score += 5.0f; break;
            case 'q': score += 9.0f; break;
            case 'k': score += 1000.0f; break; // King capture (shouldn't happen)
        }
    }

    // Check for promotions
    if (move.special >= thc::SPECIAL_PROMOTION_QUEEN && move.special <= thc::SPECIAL_PROMOTION_KNIGHT) {
        score += 9.0f; 
    }

    // Positional gain
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

// Add a mobility bonus for the pieces (not sure if this helps).
int SerialEngine::evaluate_mobility(thc::ChessRules& cr, bool is_white, const std::vector<int>& piece_indices) {
    int mobility_score = 0;
    thc::ChessRules cr_copy = cr;
    std::vector<thc::Move> moves;
    cr_copy.GenLegalMoveList(moves);

    for (const auto& move : moves) {
        char piece = cr_copy.squares[move.src];
        if ((is_white && isupper(piece)) || (!is_white && islower(piece))) {
            char lower_piece = tolower(piece);
            switch (lower_piece) {
                case 'n': mobility_score += 4; break;
                case 'b': mobility_score += 4; break;
                case 'r': mobility_score += 2; break;
                case 'q': mobility_score += 1; break;
                default: break;
            }
        }
    }

    return mobility_score;
}

int SerialEngine::evaluate_pawn_structure(const std::vector<int>& pawn_files, bool is_white) {
    int score = 0;

    // Count pawns on each file
    int file_counts[8] = {0};
    for (int file : pawn_files) {
        file_counts[file]++;
    }

    // Evaluate pawn structure
    int pawn_islands = 0;
    bool in_island = false;

    for (int i = 0; i < 8; ++i) {
        if (file_counts[i] > 0) {
            // Check for doubled pawns; we want a penalty for doubled pawns
            if (file_counts[i] > 1) {
                score -= 10 * (file_counts[i] - 1);
            }
            if (!in_island) {
                in_island = true;
                pawn_islands++;
            }
        } else {
            in_island = false;
        }
    }

    // Penalty for more pawn islands
    score -= 5 * (pawn_islands - 1);

    // Evaluate isolated pawns
    for (int i = 0; i < 8; ++i) {
        if (file_counts[i] > 0) {
            bool has_adjacent_pawns = false;
            if (i > 0 && file_counts[i - 1] > 0) has_adjacent_pawns = true;
            if (i < 7 && file_counts[i + 1] > 0) has_adjacent_pawns = true;
            if (!has_adjacent_pawns) {
                score -= 15; // Penalty for isolated pawns, might have to adjust
            }
        }
    }

    return score;
}

int SerialEngine::evaluate_king_safety(thc::ChessRules& cr, int king_index, bool is_white, bool endgame) {
    int safety_score = 0;

    if (king_index == -1) return safety_score; // King not found

    if (endgame) {
        // In the endgame, the king can be more active (try to trap other king but somewhat buggy rn)
        return safety_score; // No penalties in endgame
    }

    // Existing king safety evaluation
    // Evaluate pawn shield, exposure, threats, etc.

    int rank = king_index / 8;
    int file = king_index % 8;

    // Evaluate pawn shield
    int pawn_shield_bonus = 0;
    int direction = is_white ? -1 : 1; // Direction towards opponent

    for (int df = -1; df <= 1; ++df) {
        int shield_rank = rank + direction;
        int shield_file = file + df;
        if (shield_rank >= 0 && shield_rank <= 7 && shield_file >= 0 && shield_file <= 7) {
            int shield_index = shield_rank * 8 + shield_file;
            char shield_piece = cr.squares[shield_index];
            if ((is_white && shield_piece == 'P') || (!is_white && shield_piece == 'p')) {
                pawn_shield_bonus += 10;
            }
        }
    }

    safety_score += pawn_shield_bonus;

    // Penalty for open files or lack of pawn shield
    if (pawn_shield_bonus == 0) {
        safety_score -= 20; // King is exposed
    }

    return safety_score;
}

int SerialEngine::evaluate_king_activity(int own_king_index, int opponent_king_index, bool is_white) {
    int activity_score = 0;

    int rank = own_king_index / 8;
    int file = own_king_index % 8;

    // Centralization bonus
    float center_rank = 3.5f;
    float center_file = 3.5f;
    float distance_to_center = std::abs(rank - center_rank) + std::abs(file - center_file);
    activity_score -= static_cast<int>(distance_to_center * 5); // Encourage centralization

    // Proximity to opponent's king (endgame)
    int opponent_rank = opponent_king_index / 8;
    int opponent_file = opponent_king_index % 8;
    int king_distance = std::abs(rank - opponent_rank) + std::abs(file - opponent_file);
    if (is_white) {
        activity_score -= king_distance * 2; // Encourage approaching opponent's king
    } 
    else {
        activity_score += king_distance * 2;
    }

    // Adjust king safety considerations
    // The king is less likely to be attacked in endgame
    activity_score += 20; // Reduce penalties for exposure

    return activity_score;
}


const int ENDGAME_MATERIAL_THRESHOLD = 2400; // Adjust based on testing

bool SerialEngine::is_endgame(int white_material, int black_material) {
    int total_material = white_material + black_material;
    return total_material <= ENDGAME_MATERIAL_THRESHOLD; // Define a threshold, e.g., 2400 (two rooks)
}

SerialEngine::Score SerialEngine::static_eval(thc::ChessRules& cr) {
    Score total_score = 0.0f;

    // Material counts
    int white_material = 0;
    int black_material = 0;

    // Piece counts for bishop pair evaluation
    int white_bishops = 0;
    int black_bishops = 0;

    // King positions
    int white_king_index = -1;
    int black_king_index = -1;

    // Variables for pawn structure
    std::vector<int> white_pawn_files;
    std::vector<int> black_pawn_files;

    // Piece positions for mobility evaluation
    std::vector<int> white_piece_indices;
    std::vector<int> black_piece_indices;

    // Evaluate material and positional bonuses
    for (int i = 0; i < 64; i++) {
        char piece = cr.squares[i];
        if (piece == ' ')
            continue;

        int index = i;
        int flipped_index = 63 - i; // Flips the board for Black
        int piece_value = 0;
        float positional_bonus = 0.0f;

        bool is_white = isupper(piece);
        char lower_piece = tolower(piece);

        switch (lower_piece) {
            case 'p':
                piece_value = 100;
                positional_bonus = pawn_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_material += piece_value;
                    white_pawn_files.push_back(index % 8);
                } else {
                    black_material += piece_value;
                    black_pawn_files.push_back(index % 8);
                }
                break;
            case 'n':
                piece_value = 320;
                positional_bonus = knight_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_material += piece_value;
                    white_piece_indices.push_back(index);
                } else {
                    black_material += piece_value;
                    black_piece_indices.push_back(index);
                }
                break;
            case 'b':
                piece_value = 330;
                positional_bonus = bishop_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_material += piece_value;
                    white_bishops++;
                    white_piece_indices.push_back(index);
                } else {
                    black_material += piece_value;
                    black_bishops++;
                    black_piece_indices.push_back(index);
                }
                break;
            case 'r':
                piece_value = 500;
                positional_bonus = rook_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_material += piece_value;
                    white_piece_indices.push_back(index);
                } else {
                    black_material += piece_value;
                    black_piece_indices.push_back(index);
                }
                break;
            case 'q':
                piece_value = 900;
                positional_bonus = queen_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_material += piece_value;
                    white_piece_indices.push_back(index);
                } else {
                    black_material += piece_value;
                    black_piece_indices.push_back(index);
                }
                break;
            case 'k':
                piece_value = 20000; // High value for the King
                positional_bonus = king_table[is_white ? index : flipped_index] / 1.0f;
                if (is_white) {
                    white_king_index = index;
                } else {
                    black_king_index = index;
                }
                break;
            default:
                break;
        }

        Score square_score = piece_value + positional_bonus;
        if (is_white) {
            total_score += square_score;
        } else {
            total_score -= square_score;
        }
    }

    // Bishop pair bonus
    if (white_bishops >= 2) total_score += 50;
    if (black_bishops >= 2) total_score -= 50;

    // Mobility evaluation
    total_score += evaluate_mobility(cr, true, white_piece_indices);
    total_score -= evaluate_mobility(cr, false, black_piece_indices);

    // Pawn structure evaluation
    total_score += evaluate_pawn_structure(white_pawn_files, true);
    total_score -= evaluate_pawn_structure(black_pawn_files, false);

    // King safety evaluation

    bool endgame = is_endgame(white_material, black_material);

    total_score += evaluate_king_safety(cr, white_king_index, true, endgame);
    total_score -= evaluate_king_safety(cr, black_king_index, false, endgame);

    // After calculating total material
    
    // Evaluate king activity in endgame
    if (endgame) {
        total_score += evaluate_king_activity(white_king_index, black_king_index, true);
        total_score -= evaluate_king_activity(black_king_index, white_king_index, false);
    }


    return total_score;
}

int debug_node_count = 0;

thc::Move SerialEngine::solve(thc::ChessRules& cr, bool is_white_player) {
    this->time_limit_reached = false;

    int pid;

    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    this->start_time = std::chrono::steady_clock::now();

    thc::Move best_move_so_far;
    bool move_found = false;

    for (int current_depth = 1; current_depth <= MAX_DEPTH; ++current_depth) {
        debug_node_count = 0;
        if (time_limit_reached) {
            break; 
        }

        // thc::Move current_best_move;
        auto [current_score, current_best_move] = solve_serial_engine(
            cr,
            is_white_player,
            0,
            current_depth, 
            MPI_COMM_WORLD
        );

        if (time_limit_reached) {
            break; 
        }

        best_move_so_far = current_best_move;
        move_found = true;

        if (pid != 0) continue;

        // Debug output (record this data as metric for engine performance)
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = current_time - start_time;
        std::cout << "Depth: " <<  current_depth 
        << ", Score: " << (current_score / 100.0f) 
        << ", Time: " << elapsed_seconds.count() << "s" 
        << ", Nodes Evaluated = " << debug_node_count 
        << ", knps: " << (debug_node_count/1000.0) / elapsed_seconds.count() 
        << std::endl;
    }

    if (move_found) {
        return best_move_so_far;
    } else {
        // If no move was found (unlikely), generate a random legal move
        std::vector<thc::Move> legal_moves;
        cr.GenLegalMoveList(legal_moves);
        if (!legal_moves.empty()) {
            return legal_moves[0];
        } else {
            // No legal moves, return a default move
            return thc::Move();
        }
    }
}

std::pair<SerialEngine::Score, thc::Move>
SerialEngine::solve_serial_engine(
    thc::ChessRules& cr,
    bool is_white_player,
    int depth,
    int max_depth,
    MPI_Comm comm
) {
    int pid, nproc;

    MPI_Comm_rank(comm, &pid);
    MPI_Comm_size(comm, &nproc);

    {
        thc::Move null_move;

        thc::DRAWTYPE draw_reason;
        if (cr.IsDraw(false, draw_reason)) {
            return {0.0f, null_move};
        }

        // Check for checkmate or stalemate
        thc::TERMINAL terminal;
        if (cr.Evaluate(terminal)) {
            if (terminal == thc::TERMINAL_WCHECKMATE) {
                debug_node_count++;
                return {-INF_SCORE + depth, null_move}; // White is checkmated
            } else if (terminal == thc::TERMINAL_BCHECKMATE) {
                debug_node_count++;
                return {INF_SCORE - depth, null_move}; // Black is checkmated
            } else if (terminal == thc::TERMINAL_WSTALEMATE || terminal == thc::TERMINAL_BSTALEMATE) {
                debug_node_count++;
                return {0.0f, null_move}; // Stalemate is a draw
            }
        }
        if (depth == max_depth) {
            debug_node_count++;
            return {static_eval(cr), null_move};
        }
    }

    std::vector<thc::Move> legal_moves;
    cr.GenLegalMoveList(legal_moves);

    std::pair<SerialEngine::Score, thc::Move> ans_pair;

    if (nproc <= legal_moves.size()) {
        MPI_Comm my_comm;
        MPI_Comm_split(comm, pid, pid, &my_comm);
        // only contains me in the subset
        bool found = false;

        for (int i=pid;i<legal_moves.size();i+=nproc) {
            thc::ChessRules cr_copy = cr;
            cr_copy.PushMove(legal_moves[i]);

            auto curr_ans = solve_serial_engine(cr_copy, !is_white_player, depth+1, max_depth, my_comm);
            if (!found) {
                ans_pair = curr_ans;
                found = true;
                ans_pair.second = legal_moves[i];
            }
            else {
                if (is_white_player and ans_pair.first > curr_ans.first) {
                    ans_pair.first = curr_ans.first;
                    ans_pair.second = legal_moves[i];
                }
                else if(!is_white_player and ans_pair.first < curr_ans.first) {
                    ans_pair.first = curr_ans.first;
                    ans_pair.second = legal_moves[i];
                }
            }
        }
        
        MPI_Comm_free(&my_comm);
    }
    else {
        MPI_Comm my_comm;
        int my_move_ind = pid % legal_moves.size();
        MPI_Comm_split(comm, my_move_ind, pid, &my_comm);

        thc::ChessRules cr_copy = cr;
        cr_copy.PushMove(legal_moves[my_move_ind]);

        ans_pair = solve_serial_engine(cr_copy, !is_white_player, depth+1, max_depth, my_comm);
        ans_pair.second = legal_moves[my_move_ind];

        MPI_Comm_free(&my_comm);
    }

    std::pair<SerialEngine::Score, thc::Move> best_ans;

    if (is_white_player) {
        MPI_Allreduce(&ans_pair, &best_ans, 1, MPI_FLOAT_INT, MPI_MINLOC, comm);
    }
    else {
        MPI_Allreduce(&ans_pair, &best_ans, 1, MPI_FLOAT_INT, MPI_MAXLOC, comm);
    }

    return best_ans;
}
