#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "thc.h"
#include "serial-engine.h"


// void print(){std::cout<<std::endl;}
// void print(bool endline) {if(endline)std::cout<<std::endl;}
// template<typename T, typename ...TAIL>
// void print(const T &t, TAIL... tail)
// {
//     std::cout<<t<<' ';
//     print(tail...);
// }

void print_board(thc::ChessRules& cr) {
    std::cout << cr.ToDebugStr() << std::endl;
}

int main(int argc, char* argv[]) {
    bool computer_is_white = false;
    bool computer_is_black = true;

    int mpi_id, mpi_nproc;

    // TODO
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_nproc);

    // print("HELLO", mpi_id, mpi_nproc);

    // Initialize the game
    thc::ChessRules cr;
    cr.Forsyth("startpos");

    SerialEngine engine;

    bool game_over = false;
    thc::TERMINAL terminal;

    while (!game_over) {
        MPI_Barrier(MPI_COMM_WORLD);
        bool white_turn = cr.WhiteToPlay();
        std::string move_name = "Black";
        if (white_turn) move_name = "White";

        if ((white_turn and computer_is_white) or (!white_turn and computer_is_black)) {
            // Computer's turn
            thc::Move best_move = engine.solve(cr, true);
            if (mpi_id == 0) std::cout << "Computer ("<<move_name<<") plays: " << best_move.NaturalOut(&cr) << std::endl;
            cr.PushMove(best_move);

            // std::cout<<"LOL"<<std::endl;
            // break;
        }
        else {

            thc::Move user_move;
            if (mpi_id == 0) {

                print_board(cr);
                std::string user_input;
                std::cout << "Your move ("<< move_name<< "): "<<std::flush;
                std::getline(std::cin, user_input);

                // Parse and apply the move
                // thc::Move user_move;
                bool move_ok = user_move.NaturalIn(&cr, user_input.c_str());

                if (!move_ok) {
                    std::cout<<"INVALID MOVE"<<std::endl;
                    continue;
                }

                std::vector<thc::Move> legal_moves;
                cr.GenLegalMoveList(legal_moves);
                if (std::find(legal_moves.begin(), legal_moves.end(), user_move) == legal_moves.end()) {
                    std::cout << "Illegal move. Try again." << std::endl;
                    continue;
                }

                MPI_Bcast(&user_move, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
            }
            else {
                MPI_Bcast(&user_move, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

                // std::cout<<"RECEIVED "<<user_move.NaturalOut(&cr)<<std::endl;
            }
            
            cr.PushMove(user_move);
        }
        
        // Display the board
        if (mpi_id == 0) print_board(cr);

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

    MPI_Finalize();

    return 0;
}
