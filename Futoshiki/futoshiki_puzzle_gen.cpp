/*
	This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
	See the LICENSE.txt file in the root of this project for complete terms.

	Copyright (c) Erhard Eibl 2024
*/

#include "futoshiki_puzzle_gen.h"
#include "futoshiki_sol_gen.h"

#include <windows.h>
#include <chrono>
#include <thread>
#include <set>

int chunk_size = 1e6;
int chunk_index = 0;

std::fstream fs;

board_struct knuth_board = create_empty_board();

std::set<std::string> class_info;

/*
struct set_hash {
	size_t operator()(const std::set<uint_fast16_t>& v) const {
		std::hash<uint_fast16_t> hasher;
		size_t seed = 0;
		for (uint_fast16_t i : v) {
			seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};

using uor_set_set = std::set<std::set<uint_fast16_t>, set_hash>;
*/

using uor_set_set = std::set<std::set<uint_fast16_t>>;

std::array<std::array<board_struct, 5>, 25> atomic_strong_clue_boards;
std::array<std::array<board_struct, 2>, 40> atomic_weak_clue_boards;

std::vector<std::set<uint_fast16_t>> few_solutions = {};

std::vector<board_struct> all_solutions;

std::chrono::steady_clock::time_point begin;
std::chrono::steady_clock::time_point last_update;

int counter = 0;
//int total = 2492167506	//there are exactly 2492167506 boards with 5 hints - including contradictions
int verbose = 100000;

void print_bits(board_struct board) {
	std::string out = "";
	for (int i = 0; i < 25; i++) {
		std::string rep = std::bitset<8>(board.num_grid[i]).to_string();
		out += rep + " ";
		if (i % 5 == 4) out += "\n";
	}
	for (int i = 0; i < 5; i++) {
		std::string rep = std::bitset<8>(board.comp_h_grid[i]).to_string();
		out += rep + " ";
		if (i % 5 == 4) out += "\n";
	}
	for (int i = 0; i < 5; i++) {
		std::string rep = std::bitset<8>(board.comp_v_grid[i]).to_string();
		out += rep + " ";
		if (i % 5 == 4) out += "\n";
	}
	std::cout << out + "\n\n";
}

void init_atomic_strong_clue_boards() {
	for (int position = 0; position < 25; position++) {
		for (int clue = 0; clue < 5; clue++) {
			board_struct atomic_board = create_empty_board();
			for (int help = 0; help < 25; help++) {
				if (help == position) atomic_board.num_grid[help] = numericValue[clue];
				else if (help / 5 == position / 5 || help % 5 == position % 5) atomic_board.num_grid[help] &= ~(numericValue[clue] & 0b01111111);
			}
			atomic_strong_clue_boards[position][clue] = atomic_board;

			//OPTIONAL DEMO ATOMIC BOARDS
			//std::cout << board_struct_to_readable_string(atomic_board) + "\n";
			//print_bits(atomic_board);
		}
	}
}

void init_atomic_weak_clue_boards() {
	for (int position = 0; position < 40; position++) {
		for (int clue = 1; clue <= 2; clue++) {
			board_struct atomic_board = create_empty_board();
			if (position < 20) atomic_board.comp_h_grid[(position % 20) / 4] &= ((clue << (position % 4 * 2)) | ~((0b00000011) << (position % 4 * 2)));
			else               atomic_board.comp_v_grid[(position % 20) / 4] &= ((clue << (position % 4 * 2)) | ~((0b00000011) << (position % 4 * 2)));
			atomic_weak_clue_boards[position][clue - 1] = atomic_board;
		}
	}
}

int32_t choose(const int n, const int k) {
	int32_t numerator = 1;
	int32_t denominator = 1;

	for (int i = 1; i <= k; i++) {
		numerator *= (n - (k - i));
		denominator *= i;
	}

	return numerator / denominator;
}

int32_t power(const int base, int exponent) {
	int32_t res = 1;
	for (; exponent > 0; exponent--) res *= base;
	return res;
}

uint8_t int_div(uint8_t a, uint8_t b) {
	int8_t res = 0;
	for (; a >= b; a -= b) res++;
	return res;
}

uint8_t int_mod(uint8_t a, uint8_t b) {
	for (; a >= b; a -= b);
	return a;
}



void calculate_test() {

	int64_t total = 0;

	for (int strong_clues = 0; strong_clues < 6; strong_clues++) {
		int weak_clues = 5 - strong_clues;

		int32_t temp = 1;

		temp *= choose(25, strong_clues) * power(5, strong_clues);
		temp *= choose(40, weak_clues) * power(2, weak_clues);

		total += temp;
	}

	std::cout << total;

}

void init() {

	all_solutions = generate_solutions();

	init_atomic_weak_clue_boards();
	init_atomic_strong_clue_boards();

	knuth_board = join_boards(&knuth_board, &atomic_strong_clue_boards[0][0]);
	knuth_board = join_boards(&knuth_board, &atomic_weak_clue_boards[8][0]);
	knuth_board = join_boards(&knuth_board, &atomic_weak_clue_boards[9][0]);
	knuth_board = join_boards(&knuth_board, &atomic_weak_clue_boards[24][0]);
	knuth_board = join_boards(&knuth_board, &atomic_weak_clue_boards[29][1]);
	
}


std::array<std::set<uint_fast16_t>, 4> generated_code_transformations(const board_struct* board){
	std::set<uint_fast16_t> org, trp, inv, t_i;

	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 5; col++) {
			int num = decode_atom_to_int(board->num_grid[5 * row + col]);
			if (num > 0) {
				org.insert((5 * row + col) * 5 + (num - 1));
				inv.insert((5 * row + col) * 5 + (5 - num));
				trp.insert((5 * col + row) * 5 + (num - 1));
				t_i.insert((5 * col + row) * 5 + (5 - num));
			}
		}
	}

	for (int z = 0; z < 5; z++) {
		int8_t temp = board->comp_h_grid[z];
		for (int p = 0; p < 4; p++) {
			int8_t comp = temp & 0b00000011;

			if (comp == 0b00000001) {
				org.insert(5 * (25 + z * 4 + p));
				inv.insert(5 * (25 + z * 4 + p) + 1);
				trp.insert(5 * (45 + z * 4 + p));
				t_i.insert(5 * (45 + z * 4 + p) + 1);
			}

			else if (comp == 0b00000010) {
				org.insert(5 * (25 + z * 4 + p) + 1);
				inv.insert(5 * (25 + z * 4 + p));
				trp.insert(5 * (45 + z * 4 + p) + 1);
				t_i.insert(5 * (45 + z * 4 + p));
			}
			temp >>= 2;
		}
	}

	for (int z = 0; z < 5; z++) {
		int8_t temp = board->comp_v_grid[z];
		for (int p = 0; p < 4; p++) {
			int8_t comp = temp & 0b00000011;

			if (comp == 0b00000001) {
				org.insert(5 * (45 + z * 4 + p));
				inv.insert(5 * (45 + z * 4 + p) + 1);
				trp.insert(5 * (25 + z * 4 + p));
				t_i.insert(5 * (25 + z * 4 + p) + 1);
			}

			else if (comp == 0b00000010) {
				org.insert(5 * (45 + z * 4 + p) + 1);
				inv.insert(5 * (45 + z * 4 + p));
				trp.insert(5 * (25 + z * 4 + p) + 1);
				t_i.insert(5 * (25 + z * 4 + p));
			}
			temp >>= 2;
		}
	}

	std::array<std::set<uint_fast16_t>, 4> res;
	res[0] = org;
	res[1] = inv;
	res[2] = trp;
	res[3] = t_i;

	return res;
}


std::array<std::set<uint_fast16_t>, 4> generate_code_rotations(const board_struct* board) {
	std::set<uint_fast16_t> rot0, rot1, rot2, rot3;

	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 5; col++) {

			int num = decode_atom_to_int(board->num_grid[5 * row + col]);

			if (num > 0) {
				int t_row = row, t_col = col;
				int t_pos = t_row * 5 + t_col;

				rot0.insert((5 * t_pos) + (num - 1));

				int temp = t_col;
				t_col = 4 - t_row;
				t_row = temp;
				t_pos = t_row * 5 + t_col;

				rot1.insert((5 * t_pos) + (num - 1));

				temp = t_col;
				t_col = 4 - t_row;
				t_row = temp;
				t_pos = t_row * 5 + t_col;

				rot2.insert((5 * t_pos) + (num - 1));

				temp = t_col;
				t_col = 4 - t_row;
				t_row = temp;
				t_pos = t_row * 5 + t_col;

				rot3.insert((5 * t_pos) + (num - 1));
			}
		}
	}

	for (int z = 0; z < 5; z++) {
		int8_t temp = board->comp_h_grid[z];
		for (int p = 0; p < 4; p++) {
			int8_t comp = temp & 0b00000011;

			int pos0 = 25 + z * 4 + p;
			int pos1 = 45 + (4 - z) * 4 + p;
			int pos2 = 25 + (4 - z) * 4 + (3 - p);
			int pos3 = 45 + z * 4 + (3 - p);

			if (comp == 0b00000001) {

				rot0.insert(5 * pos0);
				rot1.insert(5 * pos1);
				rot2.insert(5 * pos2 + 1);
				rot3.insert(5 * pos3 + 1);
			}

			else if (comp == 0b00000010) {

				rot0.insert(5 * pos0 + 1);
				rot1.insert(5 * pos1 + 1);
				rot2.insert(5 * pos2);
				rot3.insert(5 * pos3);
			}

			temp >>= 2;
		}
	}

	for (int z = 0; z < 5; z++) {
		int8_t temp = board->comp_v_grid[z];
		for (int p = 0; p < 4; p++) {
			int8_t comp = temp & 0b00000011;

			int pos0 = 45 + z * 4 + p;				//offset incorrect
			int pos1 = 25 + z * 4 + (3 - p);
			int pos2 = 45 + (4 - z) * 4 + (3 - p);
			int pos3 = 25 + (4 - z) * 4 + p;

			if (comp == 0b00000001) {
				rot0.insert(5 * pos0);
				rot1.insert(5 * pos1 + 1);
				rot2.insert(5 * pos2 + 1);
				rot3.insert(5 * pos3);
			}

			else if (comp == 0b00000010) {
				rot0.insert(5 * pos0 + 1);
				rot1.insert(5 * pos1);
				rot2.insert(5 * pos2);
				rot3.insert(5 * pos3 + 1);
			}

			temp >>= 2;
		}
	}

	std::array<std::set<uint_fast16_t>, 4> rotations;

	rotations[0] = rot0;
	rotations[1] = rot1;
	rotations[2] = rot2;
	rotations[3] = rot3;

	return rotations;
}

board_struct short_code_to_hint_board(std::set<uint_fast16_t> code) {

	board_struct board = create_empty_board();

	for (auto content : code) {
		//int pos = int_div(content, 5);
		//int val = int_mod(content, 5);

		int pos = content / 5;
		int val = content % 5;

		if (pos < 25) board = join_boards(&board, &atomic_strong_clue_boards[pos][val]);
		else board = join_boards(&board, &atomic_weak_clue_boards[pos - 25][val]);
	}

	return board;
}

//creates dense representation of hint boards
std::set<uint_fast16_t> hint_board_to_short_code(const board_struct* a_board) {
	std::set<uint_fast16_t> code;

	int counter = 0;

	for (int i = 0; i < 25; i++) {
		int8_t temp = decode_atom_to_int(a_board->num_grid[i]);
		if (temp > 0) {
			code.insert((5 * counter) + (temp - 1));
		}
		counter++;
	}

	for (int i = 0; i < 5; i++) {
		int8_t temp = a_board->comp_h_grid[i];
		for (int j = 0; j < 4; j++) {
			int8_t comp = temp & 0b00000011;
			if (comp == 0b00000001) code.insert(5 * counter);
			else if (comp == 0b00000010) code.insert(5 * counter + 1);
			temp >>= 2;
			counter++;
		}
	}

	for (int i = 0; i < 5; i++) {
		int8_t temp = a_board->comp_v_grid[i];
		for (int j = 0; j < 4; j++) {
			int8_t comp = temp & 0b00000011;
			if (comp == 0b00000001) code.insert(5 * counter);
			else if (comp == 0b00000010) code.insert(5 * counter + 1);
			temp >>= 2;
			counter++;
		}
	}

	return code;
}


bool has_obvious_contradictions(const board_struct* board) {

	for (int i = 0; i < 25; i++) {
		if ((board->num_grid[i] & 0b01111111) == 0b00000000) return true;
	}
	return false;
}

bool solves(const board_struct* puzzle, const board_struct* solution) {

	for (int i = 0; i < 25; i++) if (!(puzzle->num_grid[i] & solution->num_grid[i])) return false;
	
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			if (!((0b00000011 << (j * 2)) & puzzle->comp_h_grid[i] & solution->comp_h_grid[i])) return false;
			if (!((0b00000011 << (j * 2)) & puzzle->comp_v_grid[i] & solution->comp_v_grid[i])) return false;
		}
	}

	return true;
}

int count_class_size(const board_struct* board) {
	uor_set_set set;

	board_struct r0 = rotate_board_clockwise(board);
	board_struct r1 = rotate_board_clockwise(&r0);
	board_struct r2 = rotate_board_clockwise(&r1);

	for (auto f : generated_code_transformations(board)) set.insert(f);
	for (auto f : generated_code_transformations(&r0)) set.insert(f);
	for (auto f : generated_code_transformations(&r1)) set.insert(f);
	for (auto f : generated_code_transformations(&r2)) set.insert(f);

	return set.size();

}

bool has_fewer_than_n_solutions(const board_struct* board, std::vector<board_struct>* possible_solutions, int n) {
	int sol_count = 0;
  
	for (int i = 0; i < possible_solutions->size(); i++) {
		if (solves(board, &(*possible_solutions)[i])) {
			sol_count++;
			if (sol_count >= n) return false;
		}
	}

	if (sol_count > 0) {
		int class_size = count_class_size(board);
		std::string message = 
			"====\n\nBoard with " + std::to_string(sol_count) + " solutions. Class size is " 
			+ std::to_string(class_size) + ":\n" + board_struct_to_readable_string(*board) + "====\n\n";

		fs << message;
		std::cout << message;
		few_solutions.push_back(hint_board_to_short_code(board));

		return true;
	}
	
	return false;
}

bool mark_all_symmetries_if_unkown(const board_struct* board, uor_set_set* all_checked) {
	for (std::set<uint_fast16_t> f : generate_code_rotations(board)) if (all_checked->contains(f)) return true;
	for (std::set<uint_fast16_t> f : generated_code_transformations(board)) all_checked->insert(f);
	return false;
}

bool check_solutions_and_mark_all_symmetries_if_unkown(const board_struct* board, uor_set_set* all_checked, std::vector<board_struct>* possible_solutions) {
	for (std::set<uint_fast16_t> f : generate_code_rotations(board)) if (all_checked->contains(f)) return true;
	for (std::set<uint_fast16_t> f : generated_code_transformations(board)) all_checked->insert(f);

	if (has_fewer_than_n_solutions(board, possible_solutions, 5)) {

		few_solutions.push_back(hint_board_to_short_code(board));
	}

	return false;
}

void recursively_add_clue(board_struct board, int pos, int clues_remaining, int strong_remaining, uor_set_set* all_checked, std::vector<board_struct>* possible_solutions) {

	if (possible_solutions->empty()) {
		mark_all_symmetries_if_unkown(&board, all_checked);
		return;
	}

	if (!clues_remaining) {
		counter++;

		//DEMO PUZZLE GEN
		//std::cout << board_struct_to_readable_string(board) + "\n";

		//OPTIONAL DEMO INTERNAL REPRESENTATION
		//print_bits(board);


		/*
		std::array<std::vector<uint_fast16_t>, 4> rotations = generate_code_rotations(&board);
		for (int i = 0; i < 4; i++) {
			std::cout << board_struct_to_readable_string(short_code_to_hint_board(rotations[i])) << "\n";
		}
		*/

		
		if ((counter % verbose) == 0) {
			std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

			std::string boardstr = board_struct_to_readable_string(board);
			std::cout.imbue(std::locale(""));
			std::cout << "Time passed since beginning: \t\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(now - begin).count() / 1000 << "ms\n";
			std::cout << "Time passed since last update: \t\t\t" << std::chrono::duration_cast<std::chrono::microseconds>(now - last_update).count() / 1000 << "ms\n";
			std::cout << "Number of checked boards without symmetries: \t" << counter << "\n";
			std::cout << "Number of checked boards with symmetries: \t" << all_checked->size() << "\n";
			std::cout << "Current Board:\n" + boardstr + "\n\n";

			last_update = now;
		}

		//std::string boardstr = board_struct_to_readable_string(board) + "\n";
		//std::cout << boardstr + "\n\n";

		check_solutions_and_mark_all_symmetries_if_unkown(&board, all_checked, possible_solutions);

		
		return;
	}

	if (strong_remaining == 0) pos = 25;
	bool new_strong = strong_remaining > 0;

	int max_pos = strong_remaining > 0 ? 25 : 65;
	int max_clue = strong_remaining > 0 ? 5 : 2;

	for (int new_pos = pos; new_pos < max_pos; new_pos++) {
		for (int clue_nr = 1; clue_nr <= max_clue; clue_nr++) {
			board_struct* relevant_atomic = new_strong ? &atomic_strong_clue_boards[new_pos][clue_nr - 1] : &atomic_weak_clue_boards[new_pos - 25][clue_nr - 1];
			board_struct new_board = join_boards(&board, relevant_atomic);

			if (has_obvious_contradictions(&new_board)) continue;

			if (clues_remaining > 2) {
				std::vector<board_struct> new_possible_solutions;
				for (int i = 0; i < possible_solutions->size(); i++) {
					if (solves(&new_board, &(*possible_solutions)[i])) {
						new_possible_solutions.push_back((*possible_solutions)[i]);
					}
				}
				recursively_add_clue(new_board, new_pos + 1, clues_remaining - 1, strong_remaining - 1, all_checked, &new_possible_solutions);
			}
			else{
				recursively_add_clue(new_board, new_pos + 1, clues_remaining - 1, strong_remaining - 1, all_checked, possible_solutions);
			}


		}
	}
}

void demo_with_one_strong() {
	fs.open("few_sol_two_strong.txt", std::fstream::out | std::fstream::trunc);
	init();

	begin = std::chrono::steady_clock::now();
	board_struct board = create_empty_board();

	uor_set_set all_checked = {};
	counter = 0;
	recursively_add_clue(board, 0, 5, 2, &all_checked, &all_solutions);

	std::cout << "There are " + std::to_string(few_solutions.size()) + " boards with few solutions:\n";
}

void generate_hint_boards() {

	fs.open("few_sol_hint_boards.txt", std::fstream::out | std::fstream::trunc);

	init();

	begin = std::chrono::steady_clock::now();
	board_struct board = create_empty_board();
	
	for (int strong_clues = 0; strong_clues <= 5; strong_clues++) {
		uor_set_set all_checked = {};
		recursively_add_clue(board, 0, 5, strong_clues, &all_checked, &all_solutions);
	}
	
	std::cout << "There are " + std::to_string(few_solutions.size()) + " boards with few solutions:\n";

	for (int i = 0; i < few_solutions.size(); i++) {
		std::string out = "";
		std::set<uint_fast16_t> code = few_solutions[i];
		
		for (auto content : code) out += content + " ";

		std::cout << "\n" + out + "\n";
	}

	fs.close();

	return;

}

void test_equals() {
	init();

	board_struct a = create_empty_board();
	board_struct b = create_empty_board();

	a = join_boards(&a, &atomic_strong_clue_boards[0][0]);
	a = join_boards(&a, &atomic_weak_clue_boards[8][0]);
	a = join_boards(&a, &atomic_weak_clue_boards[9][0]);
	a = join_boards(&a, &atomic_weak_clue_boards[24][0]);
	a = join_boards(&a, &atomic_weak_clue_boards[29][1]);

	b = join_boards(&b, &atomic_strong_clue_boards[0][0]);
	b = join_boards(&b, &atomic_weak_clue_boards[29][1]);
	b = join_boards(&b, &atomic_weak_clue_boards[8][0]);
	b = join_boards(&b, &atomic_weak_clue_boards[9][0]);
	b = join_boards(&b, &atomic_weak_clue_boards[24][0]);

	std::cout << board_struct_to_readable_string(a);
	std::cout << board_struct_to_readable_string(b);

	bool res = board_equals(&a, &b);

	std::cout << res;

}

void test() {

	std::cout << std::to_string(int_div(128, 5)) << " " << std::to_string(int_mod(128, 5));

}

void test_knuth_board() {
	init();

	//std::cout << board_struct_to_readable_string(knuth_board);

	for (auto f : generate_code_rotations(&knuth_board)) std::cout << board_struct_to_readable_string(short_code_to_hint_board(f)) + "\n";
	for (auto f : generated_code_transformations(&knuth_board)) std::cout << board_struct_to_readable_string(short_code_to_hint_board(f)) + "\n";


	uor_set_set all_checked = {};
	//check_solutions_and_mark_all_symmetries_if_unkown(&knuth_board, &all_checked, &all_solutions);

	for (auto f : all_checked) std::cout << board_struct_to_readable_string(short_code_to_hint_board(f)) << "\n";



	int sol_count = 0;

	for (int i = 0; i < all_solutions.size(); i++) {
		if (solves(&knuth_board, &all_solutions[i])) {
			sol_count++;
			//std::cout << board_struct_to_readable_string(all_solutions[i]) << "\n";
		}
	}

	std::cout << sol_count;

	std::cout << has_fewer_than_n_solutions(&knuth_board, &all_solutions, 5);

}

void run() {
	fs.open("boards_with_few_solutions.txt", std::fstream::out | std::fstream::trunc);
	init();

	begin = std::chrono::steady_clock::now();
	board_struct board = create_empty_board();

	uor_set_set all_checked = {};
	counter = 0;
	recursively_add_clue(board, 0, 5, 5, &all_checked, &all_solutions);

	std::cout << "There are " + std::to_string(few_solutions.size()) + " boards with few solutions:\n";
}