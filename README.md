# Parallel Chess Engine

In this project, we implemented six terminal-based chess engines using either the simple minimax algorithm or alpha-beta pruning, with serial, OpenMP, and MPI versions for each method. The minimax engines achieved a 50x speedup on 128 cores and a playing strength of just under 2000 ELO at a depth 5 cutoff. Alpha-beta pruning, which significantly optimizes search efficiency, delivered a 300x speedup over the naive minimax serial engine. However, due to the sequential nature of alpha-beta pruning, its parallel versions had less pronounced speedups relative to their serial counterpart. Despite this, all parallel alpha-beta engines achieved over 2200+ ELO at a depth 7 cutoff, demonstrating strong performance and computational efficiency.

Project Repository: https://github.com/namanmansukhani/15418-chess-engine

Project Website: https://vincent2135.wixsite.com/chess


# Instructions

Note the name of the directories in this repo:


serial-engine, omp-engine, mpi-engine use alpha beta pruning, and the naive version uses simple minimax

# Build

cd into each engine's src folder and type make. Then type ./chess-engine. For mpi engines, you need to use mpirun and for mpi and openmp engines you can to specify how many threads to use.

If make does not work, try to change to complier from g++-14 in the Makefile to g++.