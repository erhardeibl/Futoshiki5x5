/*
    This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
    See the LICENSE.txt file in the root of this project for complete terms.

    Copyright (c) Erhard Eibl 2024
*/

#ifndef FUTOSHIKI_BOARD_TYPE_H
#define FUTOSHIKI_BOARD_TYPE_H

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <set>

using num_grid_type = std::array<int8_t, 25>;
using comp_grid_type = std::array<int8_t, 5>;

struct board_struct {
    num_grid_type num_grid;
    comp_grid_type comp_h_grid;
    comp_grid_type comp_v_grid;
};

int decode_atom_to_int(int num);

board_struct create_empty_board();
num_grid_type create_empty_num_grid();
comp_grid_type create_empty_comp_grid();

board_struct create_board_from_num_grid(num_grid_type a_num_grid);

board_struct transpose_board(const board_struct* a_board);
board_struct rotate_board_clockwise(const board_struct* a_board);
board_struct invert_board(const board_struct* a_board);

board_struct join_boards(const board_struct* a_board, const board_struct* b_board);

std::string board_struct_to_readable_string(board_struct board);

bool board_equals(const board_struct* a, const board_struct* b);

std::set<uint_fast16_t> hint_board_to_short_code(const board_struct* a_board);

#endif