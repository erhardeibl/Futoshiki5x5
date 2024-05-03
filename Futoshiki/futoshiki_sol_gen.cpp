/*
    This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
    See the LICENSE.txt file in the root of this project for complete terms.

    Copyright (c) Erhard Eibl 2024
*/

#include "futoshiki_sol_gen.h"
#include "board_type.h"
#include <vector>

int board_counter;

std::vector<board_struct> all_boards;

//std::fstream fs;

void printBoard(const num_grid_type& board) {
    for (int i = 0; i < 25; i++) {

        std::cout << std::bitset<8>(board[i]) << ' ';

        if (i % 5 == 4) {
            std::cout << std::endl;
        }

    }
    std::cout << std::endl;
}

bool rec_set_val(num_grid_type& board, int index, int value) {

    int8_t maskedValue = value;
    maskedValue = maskedValue & 0b01111111;

    if ((board[index] & 0b10000000) != 0) return false;       //value already set;
    if ((board[index] & value) == 0) return false;            //value illegal;

    board[index] = value;

    for (int i = index + 1; i < 25; i++) {
        if ((i / 5 == index / 5) || (i % 5 == index % 5)) {
            board[i] = board[i] & ~maskedValue;
            if (board[i] == 0b10000000) return false;
        }
    }

    return true;
}

int fold_num_grid_from(const num_grid_type& board, int index) {
    int res = 0b11111111;
    for (int i = index; i < 25; i++) {
        res = res * board[i];
    }
    return res;
}

bool is_valid(const num_grid_type& board) {

    int checker[10] = { 0b00000000 };

    for (int i = 0; i < 25; i++) {
        checker[i / 5] = checker[i / 5] | board[i];
        checker[i % 5 + 5] = checker[i % 5 + 5] | board[i];
    }

    for (int i = 0; i < 10; i++) {
        if (checker[i] != 0b10011111) return false;
    }

    return true;
}

void generate_numgrids_recursively(num_grid_type a_num_grid, int depth) {

    if (depth >= 25) { //everything filled without contradictions

        board_struct finished_board = create_board_from_num_grid(a_num_grid);
        all_boards.push_back(finished_board);
        board_counter++;

        //DEMO GENERATION
        //std::cout << board_struct_to_readable_string(finished_board);
        
        //fs << toMR_string(a_num_grid);
        return;
    }

    for (int i = 1; i <= 5; i++) {

        num_grid_type recursion_num_grid = a_num_grid;

        if (!rec_set_val(recursion_num_grid, depth, numericValue[i - 1])) {    //illegal move
            continue;
        }

        int folded = fold_num_grid_from(recursion_num_grid, depth + 1);

        if (folded == 0) continue;                              //can't fill a future cell
        else {
            generate_numgrids_recursively(recursion_num_grid, depth + 1);
        }
    }

}

std::vector<board_struct> generate_solutions() {

    num_grid_type empty_num_grid = create_empty_num_grid();

    //fs.open("futoshiki_solutions_library.txt", std::fstream::out | std::fstream::trunc);

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();     //start timer
    generate_numgrids_recursively(empty_num_grid, 0);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();       //end timer

    std::cout << "Board Number: " << board_counter << "\n" << board_struct_to_readable_string(all_boards[board_counter - 1]);

    std::cout.imbue(std::locale(""));
    std::cout << "Runtime = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "us\n" << std::endl;

    return all_boards;

    //fs.close();

}