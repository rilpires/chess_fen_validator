#include "chess_validator.h"

#include <iostream>
#include <string>

using namespace std;

int main(){
    while(1){
        string fen_string,move;

        cout << "State?" << endl;
        getline( cin , fen_string );
        cout << "Move?" << endl;
        getline( cin , move );
        
        chess_validator::TableState table_state;
        chess_validator::get_table_state( fen_string.c_str() , &table_state );
        chess_validator::INVALID_REASON invalid = chess_validator::is_move_invalid( &table_state , move.c_str() );
        if( invalid ){
            switch(invalid){
                case chess_validator::INVALID_FEN_STRING:
                    cout << "Invalid move... reason: INVALID_FEN_STRING" << endl; break;
                case chess_validator::NOT_PLAYER_TURN:
                    cout << "Invalid move... reason: NOT_PLAYER_TURN" << endl; break;
                case chess_validator::NO_UNIT:
                    cout << "Invalid move... reason: NO_UNIT" << endl; break;
                case chess_validator::SAME_PLACE:
                    cout << "Invalid move... reason: SAME_PLACE" << endl; break;
                case chess_validator::CAPTURING_SAME_COLOR:
                    cout << "Invalid move... reason: CAPTURING_SAME_COLOR" << endl; break;
                case chess_validator::INVALID_UNIT_MOVE:
                    cout << "Invalid move... reason: INVALID_UNIT_MOVE" << endl; break;
                case chess_validator::KING_IN_CHECK:
                    cout << "Invalid move... reason: KING_IN_CHECK" << endl; break;
            }
        } else {
            chess_validator::TableState new_table_state = chess_validator::apply_move( &table_state , move.c_str() );
            cout << "Valid move! New state:\n" << chess_validator::to_fen_string(&new_table_state) << endl;
        }
    }
}