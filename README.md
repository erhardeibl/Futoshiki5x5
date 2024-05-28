# Futoshiki5x5

## Description
This project was developed as part of a university seminar titled "Helping Donald Knuth" in early 2022. It presents an exhaustive proof demonstrating that the minimum number of clues required for any proper 5x5 Futoshiki puzzle is six. The results have been sent to Professor Donald Knuth to verify the related chapter in Volume 4 of "The Art of Computer Programming."

## Proof Overview
The proof involves evaluating all possible 5x5 Futoshiki boards with five clues (2,492,167,506 boards), against a library of all valid solutions (161,280 solutions).

## Optimizations
Given the large search space, the project employs several optimization techniques. These include eliminating obviously unsolvable puzzles, removing symmetric puzzles, minimizing the searched solution space by searching only in the relevant subspace, and implementing numerous bit-level optimizations both for runtime and memory usage. Even after these optimizations, the code requires a fair bit of time and memory - depending on your machine.

## Note to Reviewers
Please note that this code was written several years ago and may not reflect current best practices in C++ or my current coding skills.
