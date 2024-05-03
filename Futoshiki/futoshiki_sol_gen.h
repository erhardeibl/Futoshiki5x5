/*
    This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
    See the LICENSE.txt file in the root of this project for complete terms.

    Copyright (c) Erhard Eibl 2024
*/

#ifndef FUTOSHIKI_SOL_GEN_H
#define FUTOSHIKI_SOL_GEN_H

#include <iostream>
#include <bitset>
#include <array>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>

#include "board_type.h"

const int one = 0b10000001;
const int two = 0b10000010;
const int three = 0b10000100;
const int four = 0b10001000;
const int five = 0b10010000;

const int numericValue[] = { one, two, three, four, five };

std::vector<board_struct> generate_solutions();
board_struct create_empty_board();
void test_knuth_board();

void demo_with_one_strong();

#endif