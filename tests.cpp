// Regression tests for the issues reported in #3, #4 and #5.
//
// Build and run:
//   g++ -std=c++17 -o tests tests.cpp tests_linkage.cpp && ./tests

#include "chess_validator.h"

#include <cstdio>
#include <cstring>

using namespace chess_validator;

static int failures = 0;

static int count_moves( const TableState& table_state ){
    int count = 0;
    for( auto it = table_state.first ; it ; it = it->next ) count++;
    return count;
}

static void expect_move_list_semantics(){
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    TableState a , b;
    get_table_state( fen , &a );
    get_table_state( fen , &b );
    fill_valid_moves( &a );
    fill_valid_moves( &b );

    int a_count = count_moves( a );
    int b_count = count_moves( b );
    if( a_count == 0 || b_count == 0 ){
        printf( "FAIL: fill_valid_moves produced no moves (%d/%d)\n" , a_count , b_count );
        failures++;
        return;
    }

    b = a;
    if( count_moves( b ) != a_count ){
        printf( "FAIL: assignment did not copy the move list (%d vs %d)\n" , count_moves( b ) , a_count );
        failures++;
    }

    a = a; // self-assignment must keep the list intact
    if( count_moves( a ) != a_count ){
        printf( "FAIL: self-assignment changed the move list (%d vs %d)\n" , count_moves( a ) , a_count );
        failures++;
    }

    // A state produced by apply_move must not carry the previous move cache.
    TableState moved = apply_move( &a , "e2e4" );
    if( count_moves( moved ) != 0 ){
        printf( "FAIL: apply_move carried stale available moves\n" );
        failures++;
    }

    // Re-parsing a FEN into the same object must also invalidate the cache.
    get_table_state( fen , &a );
    if( count_moves( a ) != 0 ){
        printf( "FAIL: get_table_state kept stale available moves\n" );
        failures++;
    }
}

static void expect_missing_king_is_not_check(){
    TableState table_state;
    if( !get_table_state( "k7/8/8/8/8/8/8/8 w - - 0 1" , &table_state ) ){
        printf( "FAIL: could not parse FEN without a white king\n" );
        failures++;
        return;
    }
    if( is_king_in_check( &table_state , COLOR_WHITE ) ){
        printf( "FAIL: a missing king was reported as in check\n" );
        failures++;
    }
}

static void expect_malformed_moves_rejected(){
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    TableState table_state;
    get_table_state( fen , &table_state );

    const char* moves[] = { "" , "e" , "e2" , "e2e" , "z9z9" , "e2e9" };
    for( const char* move : moves ){
        INVALID_REASON result = is_move_invalid( &table_state , move );
        if( result != INVALID_UNIT_MOVE ){
            printf( "FAIL: malformed move \"%s\" returned %d\n" , move , result );
            failures++;
        }
    }
}

static void expect_null_and_range_safety(){
    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    TableState table_state;
    get_table_state( fen , &table_state );

    if( get_table_state( nullptr , &table_state ) ){
        printf( "FAIL: get_table_state accepted a null FEN\n" );
        failures++;
    }
    if( get_table_state( fen , nullptr ) ){
        printf( "FAIL: get_table_state accepted a null state\n" );
        failures++;
    }
    if( to_fen_string( nullptr )[0] != '\0' ){
        printf( "FAIL: to_fen_string(nullptr) did not return an empty string\n" );
        failures++;
    }
    if( is_king_in_check( nullptr , COLOR_WHITE ) ){
        printf( "FAIL: is_king_in_check(nullptr) returned true\n" );
        failures++;
    }
    if( is_move_invalid( (TableState*)nullptr , "e2e4" ) != INVALID_UNIT_MOVE ){
        printf( "FAIL: is_move_invalid(nullptr, ...) did not reject the move\n" );
        failures++;
    }
    if( is_check_mate( (TableState*)nullptr ) ){
        printf( "FAIL: is_check_mate(nullptr) returned true\n" );
        failures++;
    }
    if( get_draw_reason( nullptr ) != NO_DRAW ){
        printf( "FAIL: get_draw_reason(nullptr) did not return NO_DRAW\n" );
        failures++;
    }
    fill_valid_moves( nullptr );
    push_available_move( nullptr , 0 , 0 );

    char notation[2] = { 'x' , 'x' };
    get_notation_from_index( -1 , notation );
    get_notation_from_index( 64 , notation );
    get_notation_from_index( 0 , nullptr );
    if( notation[0] != 'x' || notation[1] != 'x' ){
        printf( "FAIL: get_notation_from_index wrote to an out-of-range index\n" );
        failures++;
    }

    if( is_move_invalid( &table_state , -1 , 0 ) != INVALID_UNIT_MOVE ){
        printf( "FAIL: is_move_invalid accepted a negative source index\n" );
        failures++;
    }
    if( is_move_invalid( &table_state , 0 , 64 ) != INVALID_UNIT_MOVE ){
        printf( "FAIL: is_move_invalid accepted an out-of-range target index\n" );
        failures++;
    }
    TableState unchanged = apply_move( &table_state , -1 , 0 , TYPE_QUEEN );
    if( unchanged.map[0] != table_state.map[0] || unchanged.map[63] != table_state.map[63] ){
        printf( "FAIL: apply_move with an out-of-range index changed the board\n" );
        failures++;
    }
}

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

static void expect_fen_after_move( const char* fen , const char* move , const char* expected ){
    TableState table_state;
    if( !get_table_state( fen , &table_state ) ){
        printf( "FAIL: could not parse FEN: %s\n" , fen );
        failures++;
        return;
    }
    TableState next_state = apply_move( &table_state , move );
    const char* result = to_fen_string( &next_state );
    if( strcmp( result , expected ) != 0 ){
        printf( "FAIL: after %s expected \"%s\" but got \"%s\"\n" , move , expected , result );
        failures++;
    }
}

int main(){
    // Memory management and input hardening regressions.
    expect_move_list_semantics();
    expect_missing_king_is_not_check();
    expect_malformed_moves_rejected();
    expect_null_and_range_safety();

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

    // En passant capture must remove the captured pawn from the board.
    expect_move( "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" , "d4e3" , VALID );
    expect_fen_after_move( "rnbqkbnr/ppp1pppp/8/8/3pP3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" , "d4e3" ,
                           "rnbqkbnr/ppp1pppp/8/8/8/4p3/PPPP1PPP/RNBQKBNR w KQkq - 0 2" );
    expect_move( "rnbqkbnr/pppp1ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 2" , "d5e6" , VALID );
    expect_fen_after_move( "rnbqkbnr/pppp1ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 2" , "d5e6" ,
                           "rnbqkbnr/pppp1ppp/4P3/8/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2" );
    // Both the capturing pawn and the captured pawn leave the 5th rank, so
    // the en passant capture exposes the white king to the rook and is illegal.
    expect_move( "k7/8/8/K2pP2r/8/8/8/8 w - d6 0 1" , "e5d6" , KING_IN_CHECK );

    if( failures ){
        printf( "\n%d test(s) failed\n" , failures );
        return 1;
    }
    printf( "All tests passed\n" );
    return 0;
}
