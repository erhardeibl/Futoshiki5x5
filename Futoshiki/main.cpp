/*
	This file is part of Futoshiki5x5 and is licensed under the GNU General Public License v3.0.
	See the LICENSE.txt file in the root of this project for complete terms.

	Copyright (c) Erhard Eibl 2024
*/

#include "futoshiki_sol_gen.h"
#include "futoshiki_puzzle_gen.h"
#include "board_type.h"

int main() {
	/*
	std::vector<board_struct> all_boards = generate_solutions();

	for (int i = 0; i < all_boards.size(); i++) {

		std::cout << board_struct_to_readable_string(all_boards[i]);
		std::cout << board_struct_to_readable_string(invert_board(all_boards[i]));
		std::cout << board_struct_to_readable_string(transpose_board(all_boards[i]));
		std::cout << board_struct_to_readable_string(rotate_board_clockwise(all_boards[i]));

		std::cout << "=================\n\n";

	}
	*/
	//test();
	//generate_hint_boards();

	run();

	//test_knuth_board();

	//demo_with_one_strong();

	//test_equals();

	return 0;

}