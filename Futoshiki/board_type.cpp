/*
    This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
    See the LICENSE.txt file in the root of this project for complete terms.

    Copyright (c) Erhard Eibl 2024
*/

#include "board_type.h"
#include <iostream>

std::array<std::string, 4> comparesymbols_h = { "!", "<", ">", " " };
std::array<std::string, 4> comparesymbols_v = { "!", "^", "v", " " };

num_grid_type create_empty_num_grid() {
    num_grid_type board = { };
    board.fill(0b00011111);
    return board;
}

comp_grid_type create_empty_comp_grid() {
    comp_grid_type comp_grid = {};
    comp_grid.fill(0b11111111);
    return comp_grid;
}

board_struct create_empty_board() {
    board_struct board = {};
    board.num_grid = create_empty_num_grid();
    board.comp_h_grid = create_empty_comp_grid();
    board.comp_v_grid = create_empty_comp_grid();

    return board;
}

bool board_equals(const board_struct* a, const board_struct* b) {
    bool res = true;

    for (int i = 0; i < 25; i++) {
        res &= a->num_grid[i] == b->num_grid[i];
    }

    for (int i = 0; i < 5; i++) {
        res &= a->comp_h_grid[i] == b->comp_h_grid[i];
        res &= a->comp_v_grid[i] == b->comp_v_grid[i];
    }

    return res;
}

board_struct create_board_from_num_grid(num_grid_type a_num_grid) {

    board_struct a_board = { };

    comp_grid_type comp_h_grid = { };
    comp_h_grid.fill(0b11111111);

    comp_grid_type comp_v_grid = { };
    comp_v_grid.fill(0b11111111);

    int8_t eq_mask = 0b11111100;
    int8_t st_mask = 0b11111101;
    int8_t lt_mask = 0b11111110;
    int8_t uq_mask = 0b11111111;


    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {

            if (col < 4) {
                //fill horizontal comparisons
                if (a_num_grid[row * 5 + col] == a_num_grid[row * 5 + (col + 1)]) comp_h_grid[row] &= ~((~eq_mask) << (2 * col)); //invalid grid
                else if (a_num_grid[row * 5 + col] < a_num_grid[row * 5 + (col + 1)]) comp_h_grid[row] &= ~((~st_mask) << (2 * col));
                else if (a_num_grid[row * 5 + col] > a_num_grid[row * 5 + (col + 1)]) comp_h_grid[row] &= ~((~lt_mask) << (2 * col));
                else                                                                   comp_h_grid[row] &= ~((~uq_mask) << (2 * col)); //something went wrong
            }

            if (row < 4) {
                //fill vertical comparisons
                if (a_num_grid[row * 5 + col] == a_num_grid[(row + 1) * 5 + col]) comp_v_grid[col] &= ~(~eq_mask << (2 * row)); //invalid grid
                else if (a_num_grid[row * 5 + col] < a_num_grid[(row + 1) * 5 + col]) comp_v_grid[col] &= ~(~st_mask << (2 * row));
                else if (a_num_grid[row * 5 + col] > a_num_grid[(row + 1) * 5 + col]) comp_v_grid[col] &= ~(~lt_mask << (2 * row));
                else                                                                   comp_v_grid[col] &= ~(~uq_mask << (2 * row)); //something went wrong
            }

        }
    }

    a_board.num_grid = a_num_grid;
    a_board.comp_h_grid = comp_h_grid;
    a_board.comp_v_grid = comp_v_grid;

    return a_board;
}

board_struct transpose_board(const board_struct* a_board) {
    board_struct new_board = create_empty_board();

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            //former rows are now colums
            new_board.num_grid[row * 5 + col] = a_board->num_grid[col * 5 + row];
        }
    }

    //former horizontal comparisons are now vertical comparisons
    new_board.comp_h_grid = a_board->comp_v_grid;
    new_board.comp_v_grid = a_board->comp_h_grid;

    return new_board;
}

board_struct rotate_board_clockwise(const board_struct* a_board) {
    board_struct new_board = create_empty_board();

    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            //set the former bottom row as the leftmost column and so on
            new_board.num_grid[row * 5 + col] = a_board->num_grid[(4 - col) * 5 + row];
        }

        //same as num_grid
        new_board.comp_v_grid[row] = a_board->comp_h_grid[4 - row];

        //former vertival comparisons are now horizontal comparisons but inverted in order and direction. So reverse the encoding
        int8_t temp = a_board->comp_v_grid[row];

        //divide and conquer reverse integer
        temp = (temp & 0xF0) >> 4 | (temp & 0x0F) << 4;
        temp = (temp & 0xCC) >> 2 | (temp & 0x33) << 2;
        temp = (temp & 0xAA) >> 1 | (temp & 0x55) << 1;

        new_board.comp_h_grid[row] = temp;
    }

    return new_board;
}

board_struct invert_board(const board_struct* a_board) {
    board_struct new_board = *a_board;
    for (int i = 0; i < 25; i++) {
        int8_t temp = new_board.num_grid[i];
        new_board.num_grid[i] >>= 5;
        for (int j = 0; j < 5; j++) {
            new_board.num_grid[i] <<= 1;
            new_board.num_grid[i] += (temp & 1);
            temp >>= 1;
        }
    }
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            if (((new_board.comp_h_grid[i] >> j * 2) ^ (new_board.comp_h_grid[i] >> j * 2 + 1)) & 1) {
                new_board.comp_h_grid[i] ^= (1 << (j * 2));
                new_board.comp_h_grid[i] ^= (1 << (j * 2 + 1));
            }
            if (((new_board.comp_v_grid[i] >> j * 2) ^ (new_board.comp_v_grid[i] >> j * 2 + 1)) & 1) {
                new_board.comp_v_grid[i] ^= (1 << (j * 2));
                new_board.comp_v_grid[i] ^= (1 << (j * 2 + 1));
            }
        }
    }
    return new_board;
}

board_struct join_boards(const board_struct* a_board, const board_struct* b_board) {

    board_struct return_board = create_empty_board();

    for (int i = 0; i < 25; i++) {
        //MSB OR, rest AND
        return_board.num_grid[i] = (0b01111111 & a_board->num_grid[i] & b_board->num_grid[i]) | (0b10000000 & (a_board->num_grid[i] | b_board->num_grid[i]));
    }

    for (int i = 0; i < 5; i++) {
        return_board.comp_h_grid[i] &= (a_board->comp_h_grid[i] & b_board->comp_h_grid[i]);
    }

    for (int i = 0; i < 5; i++) {
        return_board.comp_v_grid[i] &= (a_board->comp_v_grid[i] & b_board->comp_v_grid[i]);
    }

    return return_board;
}

int decode_atom_to_int(int num) {

    if (num > 0) return 0;

    int counter = 0;
    for (int temp = num & 0b01111111; temp > 0; temp>>=1) counter++;
    
    return counter ? counter : -1;
}

std::string decode_atom_to_string(int num) {

    int int_res = decode_atom_to_int(num);

    if (int_res < 0) return "X";
    return int_res ? std::to_string(int_res) : "O";

}
std::string board_struct_to_readable_string(board_struct board) {

    std::string out = "";

    num_grid_type num_grid = board.num_grid;
    comp_grid_type comp_h = board.comp_h_grid;
    comp_grid_type comp_v = board.comp_v_grid;

    for (int row = 0; row < 5; row++) {
        std::string num_line = "";
        std::string comp_line = "";
        for (int col = 0; col < 5; col++) {
            int index = row * 5 + col;
            std::string content = decode_atom_to_string(num_grid[index]);
            num_line += content + " " + comparesymbols_h[(comp_h[row] >> col * 2) & 0b00000011] + " ";

            if (row < 4) comp_line += comparesymbols_v[(comp_v[col] >> row * 2) & 0b00000011] + "   ";
        }
        num_line.erase(num_line.length() - 3);
        if (row < 4) comp_line.erase(comp_line.length() - 3);
        out += num_line + "\n" + comp_line + "\n";
    }

    return out;

}