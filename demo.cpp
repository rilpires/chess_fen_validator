#include "chess_validator.h"

#include <iostream>
#include <string>

using namespace std;

int main(){
    while(1){
        string state,move;

        cout << "State?" << endl;
        getline( cin , state );
        cout << "Move?" << endl;
        getline( cin , move );
        
        chess_validator::INVALID_REASON invalid = chess_validator::is_move_valid( state.c_str() , move.c_str() );
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
            cout << "Valid move! New state:\n" << chess_validator::apply_move( state.c_str() , move.c_str() ) << endl;
        }
    }
}