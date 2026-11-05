// Regression tests for the issues reported in #3, #4 and #5.
//
// Build and run:
//   g++ -std=c++17 -o tests tests.cpp && ./tests

#include "chess_validator.h"

#include <cstdio>

using namespace chess_validator;

static int failures = 0;

static void expect_parse( const char* fen , bool expected ){
    TableState table_state;
    bool result = get_table_state( fen , &table_state );
    if( result != expected ){
        printf( "FAIL: get_table_state returned %d (expected %d): %s\n" , result , expected , fen );
        failures++;
    }
}

static void expect_counters( const char* fen , int half_move_clock , int move_counter ){
    TableState table_state;
    bool parsed = get_table_state( fen , &table_state );
    if( !parsed || table_state.half_move_clock != half_move_clock || table_state.move_counter != move_counter ){
        printf( "FAIL: counters are %d/%d (expected %d/%d): %s\n" ,
                parsed ? (int)table_state.half_move_clock : -1 ,
                parsed ? table_state.move_counter : -1 ,
                half_move_clock , move_counter , fen );
        failures++;
    }
}

static void expect_en_passant_index( const char* fen , int expected ){
    TableState table_state;
    bool parsed = get_table_state( fen , &table_state );
    if( !parsed || table_state.en_passant_index != expected ){
        printf( "FAIL: en_passant_index is %d (expected %d): %s\n" ,
                parsed ? (int)table_state.en_passant_index : -1 , expected , fen );
        failures++;
    }
}

static void expect_move( const char* fen , const char* move , INVALID_REASON expected ){
    TableState table_state;
    if( !get_table_state( fen , &table_state ) ){
        printf( "FAIL: could not parse FEN: %s\n" , fen );
        failures++;
        return;
    }
    INVALID_REASON result = is_move_invalid( &table_state , move );
    if( result != expected ){
        printf( "FAIL: move %s returned %d (expected %d): %s\n" , move , result , expected , fen );
        failures++;
    }
}

int main(){
    // Issue #4: the en-passant field was only reading the file letter, so the
    // rank digit was consumed as the half-move clock.
    expect_parse( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" , true );
    expect_parse( "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2" , true );
    expect_parse( "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e3 0 2" , true );
    expect_counters( "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2" , 0 , 2 );
    expect_counters( "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e3 7 42" , 7 , 42 );
    // 'a' used to collapse to the "no en-passant" sentinel.
    expect_en_passant_index( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a3 0 1" , 40 );
    expect_en_passant_index( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq a6 0 1" , 16 );

    // Issue #5: a pawn that is not on its first move could advance any
    // number of squares.
    const char* pawn_fen = "rnbqkbnr/pppppppp/8/8/8/7P/PPPPPPP1/RNBQKBNR w KQkq - 0 1";
    expect_move( pawn_fen , "h3h4" , VALID );
    expect_move( pawn_fen , "h3h5" , INVALID_UNIT_MOVE );
    expect_move( pawn_fen , "h3h6" , INVALID_UNIT_MOVE );
    expect_move( pawn_fen , "h3h8" , INVALID_UNIT_MOVE );

    // Issue #3: castling out of check or through an attacked square was
    // accepted.
    expect_move( "k3r3/8/8/8/8/8/8/4K2R w K - 0 1" , "e1g1" , INVALID_UNIT_MOVE ); // out of check
    expect_move( "k4r2/8/8/8/8/8/8/4K2R w K - 0 1" , "e1g1" , INVALID_UNIT_MOVE ); // through f1
    expect_move( "3r3k/8/8/8/8/8/8/R3K3 w Q - 0 1" , "e1c1" , INVALID_UNIT_MOVE ); // through d1
    expect_move( "4k2r/8/8/8/8/8/8/5R1K b k - 0 1" , "e8g8" , INVALID_UNIT_MOVE ); // black, through f8
    // Legal castling must still be accepted.
    expect_move( "k7/8/8/8/8/8/8/4K2R w K - 0 1" , "e1g1" , VALID );
    expect_move( "7k/8/8/8/8/8/8/R3K3 w Q - 0 1" , "e1c1" , VALID );

    if( failures ){
        printf( "\n%d test(s) failed\n" , failures );
        return 1;
    }
    printf( "All tests passed\n" );
    return 0;
}
