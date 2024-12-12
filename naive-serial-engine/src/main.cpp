#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "thc.h"
#include "serial-engine.h"

void print_board(thc::ChessRules& cr) {
    std::cout << cr.ToDebugStr() << std::endl;
}

int main(int argc, char* argv[]) {
    bool computer_is_white = false;
    bool computer_is_black = false;

    // Parse command-line arguments
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--white") {
            computer_is_white = true;
        } else if (arg == "--black") {
            computer_is_black = true;
        } else {
            std::cout << "Usage: " << argv[0] << " [--white | --black]" << std::endl;
            return 1;
        }
    } else {
        // Default to computer playing black
        computer_is_black = true;
    }

    // Initialize the game
    thc::ChessRules cr;
    cr.Forsyth("startpos");

    SerialEngine engine;

    bool game_over = false;
    thc::TERMINAL terminal;

    while (!game_over) {
        if (cr.WhiteToPlay()) {
            if (computer_is_white) {
                // Computer's turn
                thc::Move best_move = engine.solve(cr, true);
                std::cout << "Computer (White) plays: " << best_move.NaturalOut(&cr) << std::endl;
                cr.PushMove(best_move);
            } else {
                // Human's turn
                print_board(cr);
                std::string user_input;
                std::cout << "Your move (White): ";
                std::getline(std::cin, user_input);

                // Parse and apply the move
                thc::Move user_move;
                bool move_ok = user_move.NaturalIn(&cr, user_input.c_str());
                if (!move_ok) {
                    std::cout << "Invalid move. Try again." << std::endl;
                    continue;
                }
                std::vector<thc::Move> legal_moves;
                cr.GenLegalMoveList(legal_moves);
                if (std::find(legal_moves.begin(), legal_moves.end(), user_move) == legal_moves.end()) {
                    std::cout << "Illegal move. Try again." << std::endl;
                    continue;
                }
                cr.PushMove(user_move);
            }
        } else {
            if (computer_is_black) {
                // Computer's turn
                thc::Move best_move = engine.solve(cr, false);
                std::cout << "Computer (Black) plays: " << best_move.NaturalOut(&cr) << std::endl;
                cr.PushMove(best_move);
            } else {
                // Human's turn
                print_board(cr);
                std::string user_input;
                std::cout << "Your move (Black): ";
                std::getline(std::cin, user_input);

                // Parse and apply the move
                thc::Move user_move;
                bool move_ok = user_move.NaturalIn(&cr, user_input.c_str());
                if (!move_ok) {
                    std::cout << "Invalid move. Try again." << std::endl;
                    continue;
                }
                std::vector<thc::Move> legal_moves;
                cr.GenLegalMoveList(legal_moves);
                if (std::find(legal_moves.begin(), legal_moves.end(), user_move) == legal_moves.end()) {
                    std::cout << "Illegal move. Try again." << std::endl;
                    continue;
                }
                cr.PushMove(user_move);
            }
        }

        // Display the board
        print_board(cr);

        // Check for game termination
        if (cr.Evaluate(terminal)) {
            if (terminal == thc::TERMINAL_WCHECKMATE) {
                std::cout << "White is checkmated. Black wins!" << std::endl;
                game_over = true;
            } else if (terminal == thc::TERMINAL_BCHECKMATE) {
                std::cout << "Black is checkmated. White wins!" << std::endl;
                game_over = true;
            } else if (terminal == thc::TERMINAL_WSTALEMATE || terminal == thc::TERMINAL_BSTALEMATE) {
                std::cout << "Stalemate. It's a draw!" << std::endl;
                game_over = true;
            }
        }

        // Check for draw
        thc::DRAWTYPE draw_type;
        if (cr.IsDraw(false, draw_type)) {
            std::cout << "Draw due to ";
            switch (draw_type) {
                case thc::DRAWTYPE_50MOVE:
                    std::cout << "50-move rule." << std::endl;
                    break;
                case thc::DRAWTYPE_INSUFFICIENT:
                case thc::DRAWTYPE_INSUFFICIENT_AUTO:
                    std::cout << "insufficient material." << std::endl;
                    break;
                case thc::DRAWTYPE_REPITITION:
                    std::cout << "threefold repetition." << std::endl;
                    break;
                default:
                    std::cout << "unknown reason." << std::endl;
                    break;
            }
            game_over = true;
        }
    }

    return 0;
}
