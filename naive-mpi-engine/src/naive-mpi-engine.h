#ifndef NAIVE_MPI_ENGINE_H
#define NAIVE_MPI_ENGINE_H

#include "thc.h"      
#include <chrono>
#include <atomic>
#include <vector>     

#include "mpi.h"


class NaiveMPIEngine {
public:
    using Score = float;

    static constexpr Score INF_SCORE = 1000000.0f;
    static constexpr int MAX_DEPTH = 5;
    static constexpr int TIME_LIMIT_SECONDS = 60; // Time limit in seconds

    // Solve function to find the best move
    thc::Move solve(thc::ChessRules& cr, bool is_white_player);

private:
    // Recursive search function with alpha-beta pruning and iterative deepening
    std::pair<Score, thc::Move> solve_naive_mpi_engine(
        thc::ChessRules& cr,
        bool is_white_player,
        int depth,
        int max_depth,
        MPI_Comm mpi_comm
    );

    // Static evaluation function
    Score static_eval(thc::ChessRules& cr);

    // Helper function to score moves for move ordering
    float score_move(const thc::Move& move, thc::ChessRules& cr);

    // **Add the missing function declarations here**

    // Function to evaluate mobility
    int evaluate_mobility(thc::ChessRules& cr, bool is_white, const std::vector<int>& piece_indices);

    // Function to evaluate pawn structure
    int evaluate_pawn_structure(const std::vector<int>& pawn_files, bool is_white);

    // Function to evaluate king safety
    int evaluate_king_safety(thc::ChessRules& cr, int king_index, bool is_white, bool endgame);

    // Function to detect endgame phase
    bool is_endgame(int white_material, int black_material);

    // Function to evaluate king activity in endgame
    int evaluate_king_activity(int own_king_index, int opponent_king_index, bool is_white);

    // Time management variables
    std::chrono::steady_clock::time_point start_time;
    std::atomic<bool> time_limit_reached;
};

#endif 