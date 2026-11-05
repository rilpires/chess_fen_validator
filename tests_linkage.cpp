// This translation unit exists only to make sure chess_validator.h can be
// included from more than one .cpp file. If the header defines non-inline
// functions or globals, linking tests.cpp together with this file fails.

#include "chess_validator.h"

void chess_validator_linkage_test(){
    chess_validator::TableState table_state;
    chess_validator::get_table_state(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" ,
        &table_state );
}
