/*
    This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
    See the LICENSE.txt file in the root of this project for complete terms.

    Copyright (c) Erhard Eibl 2024
*/

#ifndef FUTOSHIKI_PUZZLE_GEN_H
#define FUTOSHIKI_PUZZLE_GEN_H

#include <cstdint>
#include <iostream>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <iomanip>
#include <bitset>

int32_t choose(const int n, const int k);
int32_t power(const int base, int exponent);

void generate_hint_boards();
void test();

std::vector<uint64_t> generate_hamming_weight_bitsets(uint8_t size, int bits_set);

void run();

void test_equals();

#endif