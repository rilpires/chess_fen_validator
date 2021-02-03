#ifndef CHESS_VALIDATOR_H
#define CHESS_VALIDATOR_H

#include <cstdio>
#include <cstdlib>


namespace chess_validator {

typedef long long int int64;

template<typename T>
T abs( T t ){ return( (t>0)?(t):(-t) ); }

enum SQUARE : unsigned char {
    NONE        = 0,
    
    COLOR_WHITE = (1<<0),
    COLOR_BLACK = (1<<1) ,
    
    TYPE_PAWN   = (1<<2) ,
    TYPE_ROOK   = (1<<3) ,
    TYPE_KNIGHT = (1<<4) ,
    TYPE_BISHOP = (1<<5) ,
    TYPE_KING   = (1<<6) ,
    TYPE_QUEEN  = (1<<7) ,

};

enum INVALID_REASON : int {
    VALID  = 0,
    INVALID_FEN_STRING ,
    NOT_PLAYER_TURN ,
    NO_UNIT ,
    SAME_PLACE ,
    CAPTURING_SAME_COLOR ,
    INVALID_UNIT_MOVE ,
    KING_IN_CHECK
};

enum DRAW_REASON : int {
    NO_DRAW = 0 ,
    STALEMATE ,
    INSUFFICIENT_MATERIAL ,
    SEVENTYFIVE_MOVES 
};

struct TableState {
    unsigned char map[64];
    char next_color_to_play;
    bool white_king_castling = false;
    bool white_queen_castling = false;
    bool black_king_castling = false;
    bool black_queen_castling = false;
    unsigned char en_passant = -1;
    char half_move_clock = 0;
    int move_counter = 0;

    struct AvailableMoveIterator {
        unsigned char source_index , target_index;
        AvailableMoveIterator* next = nullptr;
    };
    AvailableMoveIterator *first = nullptr , *last = nullptr;
    
    void operator=( const TableState& other ){
        for( int i = 0 ; i < 64 ; i++ )map[i] = other.map[i];
        next_color_to_play = other.next_color_to_play;
        white_king_castling = other.white_king_castling;
        white_queen_castling = other.white_queen_castling;
        black_king_castling = other.black_king_castling;
        black_queen_castling = other.black_queen_castling;
        en_passant = other.en_passant;
        half_move_clock = other.half_move_clock;
        move_counter = other.move_counter;
        first = nullptr;
        last = nullptr;
    }

    TableState(){
        first = nullptr;
        last = nullptr;
    }
    TableState( const TableState& other ) : TableState() {
        (*this) = other;
        first = nullptr;
        last = nullptr;
    }
    ~TableState(){
        auto it = first;
        while(it){
            auto temp = it->next;
            delete it;
            it = temp;
        }
    };
};

char buff[512];

bool            get_table_state( const char* fen_string , TableState* ptr_table_state ){
    unsigned int string_iterator    = 0; 
    unsigned int map_iterator   = 0;
    unsigned int string_size    = 0;

    while( fen_string[string_size++] != '\0' ){};
    
    TableState& table_state = *ptr_table_state;

    for( int i = 0 ; i< 64 ; i++ ) table_state.map[i] = SQUARE::NONE;

    // First field: map description
    while(  string_iterator < string_size && fen_string[string_iterator] != ' ' ){
        char next_c = fen_string[string_iterator];
        
        if( map_iterator >= 64 ){
            return false;
        }
        
        switch (next_c)
        {
            case 'p':
                table_state.map[map_iterator] |= TYPE_PAWN;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            case 'n':
                table_state.map[map_iterator] |= TYPE_KNIGHT;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            case 'b':
                table_state.map[map_iterator] |= TYPE_BISHOP;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            case 'r':
                table_state.map[map_iterator] |= TYPE_ROOK;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            case 'q':
                table_state.map[map_iterator] |= TYPE_QUEEN;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            case 'k':
                table_state.map[map_iterator] |= TYPE_KING;
                table_state.map[map_iterator] |= COLOR_BLACK;
                map_iterator++;
                break;
            

            case 'P':
                table_state.map[map_iterator] |= TYPE_PAWN;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case 'N':
                table_state.map[map_iterator] |= TYPE_KNIGHT;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case 'B':
                table_state.map[map_iterator] |= TYPE_BISHOP;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case 'R':
                table_state.map[map_iterator] |= TYPE_ROOK;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case 'Q':
                table_state.map[map_iterator] |= TYPE_QUEEN;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case 'K':
                table_state.map[map_iterator] |= TYPE_KING;
                table_state.map[map_iterator] |= COLOR_WHITE;
                map_iterator++;
                break;
            case '/':
                break;
            
            case '1': map_iterator += 1; break;
            case '2': map_iterator += 2; break;
            case '3': map_iterator += 3; break;
            case '4': map_iterator += 4; break;
            case '5': map_iterator += 5; break;
            case '6': map_iterator += 6; break;
            case '7': map_iterator += 7; break;
            case '8': map_iterator += 8; break;
            
            default: return false;
        }
        
        string_iterator++;
        if( map_iterator == 64 ) break;
    }
    if(  map_iterator != 64 )return false;

    while( fen_string[string_iterator] == ' ' )string_iterator++;

    // Second field: Active color
    switch (fen_string[string_iterator++])
    {
        case 'b':
        case 'B':
            table_state.next_color_to_play = COLOR_BLACK;
            break;
        case 'w':
        case 'W':
            table_state.next_color_to_play = COLOR_WHITE;
            break;
        default:
            return false;
    }


    while( fen_string[string_iterator] == ' ' )string_iterator++;

    // Third field: Castling
    for( int i = 0 ; i < 5 ; i++ ){
        if( i==5 ) return false; 
        switch (fen_string[string_iterator++]){
            case 'K':   table_state.white_king_castling  = true ; break;
            case 'Q':   table_state.white_queen_castling = true ; break;
            case 'k':   table_state.black_king_castling  = true ; break;
            case 'q':   table_state.black_queen_castling = true ; break;
            case '-':   i = 6 ; break;
            case ' ':   i = 6 ; break;
            default: return false;
        }
    }

    while( fen_string[string_iterator] == ' ' )string_iterator++;

    // Fourth field: EN passant field
    int en_passant_square = 0;
    switch (fen_string[string_iterator++]){
        case '-': break;
        case 'h': en_passant_square += 7; break;
        case 'g': en_passant_square += 6; break;
        case 'f': en_passant_square += 5; break;
        case 'e': en_passant_square += 4; break;
        case 'd': en_passant_square += 3; break;
        case 'c': en_passant_square += 2; break;
        case 'b': en_passant_square += 1; break;
        case 'a': en_passant_square += 0; break;
        default:
            switch( fen_string[string_iterator++] ){
                case '3': en_passant_square += 40; break;
                case '6': en_passant_square += 16; break;
                default: return false;
            }
    }
    if( en_passant_square != 0 )    table_state.en_passant = en_passant_square;
    else                            table_state.en_passant = -1;
    
    while( fen_string[string_iterator] == ' ' )string_iterator++;
    
    // Fifth field: halfmove clock
    table_state.half_move_clock = atoi( &fen_string[string_iterator++] );

    while( fen_string[string_iterator] == ' ' )string_iterator++;

    // Sixth field: moves counter
    table_state.move_counter = atoi( &fen_string[string_iterator++] );

    return true;

}
void            push_available_move( TableState* ptr_table_state , unsigned char source_index , unsigned char target_index ){
    TableState::AvailableMoveIterator* n = new TableState::AvailableMoveIterator();
    n->source_index = source_index;
    n->target_index = target_index;
    if( ptr_table_state->first == nullptr ){
        ptr_table_state->first = n;
        ptr_table_state->last = n;
    } else {
        ptr_table_state->last->next = n;
        ptr_table_state->last = n;
    }
};
int             get_index_from_notation( const char* str ){
    int ret = 0;
    switch( str[0] ){
        case 'a': ret += 0; break;
        case 'b': ret += 1; break;
        case 'c': ret += 2; break;
        case 'd': ret += 3; break;
        case 'e': ret += 4; break;
        case 'f': ret += 5; break;
        case 'g': ret += 6; break;
        case 'h': ret += 7; break;
    }
    switch( str[1] ){
        case '1': ret += 56;break;
        case '2': ret += 48;break;
        case '3': ret += 40;break;
        case '4': ret += 32;break;
        case '5': ret += 24;break;
        case '6': ret += 16;break;
        case '7': ret += 8; break;
        case '8': ret += 0; break;
    }
    return ret;
}
void            get_notation_from_index( int index , char* str ){
    int x = index % 8;
    int y = index / 8;
    switch (x){
        case 0: str[0] = 'a'; break;
        case 1: str[0] = 'b'; break;
        case 2: str[0] = 'c'; break;
        case 3: str[0] = 'd'; break;
        case 4: str[0] = 'e'; break;
        case 5: str[0] = 'f'; break;
        case 6: str[0] = 'g'; break;
        case 7: str[0] = 'h'; break;
    }
    str[1] = (8-y) + 48;
}
const char*     to_fen_string( TableState* ptr_table_state ){
    int string_iterator = 0;
    for( int y = 0 ; y < 8 ; y++ )
    for( int white_space_count = 0 , x = 0 ; x < 8 ; x++ )
    {
        int index = x + 8*y;
        int square = ptr_table_state->map[index];
        if( square ){
            
            if( white_space_count ){
                buff[string_iterator++] = white_space_count + 48; // int to ascii character
                white_space_count = 0;
            }

                 if( (square&COLOR_BLACK) && (square&TYPE_PAWN) )   buff[string_iterator++] = 'p';
            else if( (square&COLOR_BLACK) && (square&TYPE_KNIGHT) ) buff[string_iterator++] = 'n';
            else if( (square&COLOR_BLACK) && (square&TYPE_ROOK) )   buff[string_iterator++] = 'r';
            else if( (square&COLOR_BLACK) && (square&TYPE_BISHOP) ) buff[string_iterator++] = 'b';
            else if( (square&COLOR_BLACK) && (square&TYPE_QUEEN) )  buff[string_iterator++] = 'q';
            else if( (square&COLOR_BLACK) && (square&TYPE_KING) )   buff[string_iterator++] = 'k'; 
            else if( (square&COLOR_WHITE) && (square&TYPE_PAWN) )   buff[string_iterator++] = 'P';
            else if( (square&COLOR_WHITE) && (square&TYPE_KNIGHT) ) buff[string_iterator++] = 'N';
            else if( (square&COLOR_WHITE) && (square&TYPE_ROOK) )   buff[string_iterator++] = 'R';
            else if( (square&COLOR_WHITE) && (square&TYPE_BISHOP) ) buff[string_iterator++] = 'B';
            else if( (square&COLOR_WHITE) && (square&TYPE_QUEEN) )  buff[string_iterator++] = 'Q';
            else if( (square&COLOR_WHITE) && (square&TYPE_KING) )   buff[string_iterator++] = 'K'; 

        } else white_space_count++;

        if( x==7 && y!=7){
            if( white_space_count ) buff[string_iterator++] = white_space_count + 48; // int to ascii character
            buff[string_iterator++] = '/';
        } else if ( index == 63 && white_space_count ) {
            buff[string_iterator++] = white_space_count+48;
        }
    }

    buff[string_iterator++] = ' ';
    if( ptr_table_state->next_color_to_play == COLOR_WHITE ) buff[string_iterator++] = 'w';
    if( ptr_table_state->next_color_to_play == COLOR_BLACK ) buff[string_iterator++] = 'b';
    
    buff[string_iterator++] = ' ';
    if( !(ptr_table_state->black_king_castling 
        || ptr_table_state->black_queen_castling 
        || ptr_table_state->white_king_castling 
        || ptr_table_state->white_queen_castling))
        buff[string_iterator++] = '-';
    else{
        if( ptr_table_state->white_king_castling  ) buff[string_iterator++] = 'K';
        if( ptr_table_state->white_queen_castling ) buff[string_iterator++] = 'Q';
        if( ptr_table_state->black_king_castling  ) buff[string_iterator++] = 'k';
        if( ptr_table_state->black_queen_castling ) buff[string_iterator++] = 'q';
    }

    buff[string_iterator++] = ' ';
    if( ptr_table_state->en_passant ){
        char en_passant_notation[2];
        get_notation_from_index(ptr_table_state->en_passant,en_passant_notation);
        buff[string_iterator++] = en_passant_notation[0];
        buff[string_iterator++] = en_passant_notation[1];
    }else{
        buff[string_iterator++]='-';
    }

    buff[string_iterator++] = ' ';
    
    char aux_buff[32];
    sprintf( aux_buff , "%d." , ptr_table_state->half_move_clock );
    for( int j = 0 ; aux_buff[j] !='.' ; j++ )
        buff[string_iterator++] = aux_buff[j];
    
    buff[string_iterator++] = ' ';

    sprintf( aux_buff , "%d." , ptr_table_state->move_counter );
    for( int j = 0 ; aux_buff[j] !='.' ; j++ )
        buff[string_iterator++] = aux_buff[j];
    

    buff[string_iterator++] = '\0';

    return buff;
}
TableState      apply_move( TableState* ptr_table_state , int source_index , int target_index , int promotion_unit ){
    // Assumes that move is valid. Unexpected behavior if not!
    TableState& old_state = *ptr_table_state;
    TableState ret = *ptr_table_state;
    int delta_index = target_index - source_index;
    int source_square = old_state.map[source_index];
    int target_square = old_state.map[target_index];
    
    // Ending castling ability
    if( source_square & COLOR_WHITE ){
        if( source_square & TYPE_KING ){
            ret.white_king_castling = false;
            ret.white_queen_castling = false;
        } 
        else if ( source_square & TYPE_ROOK ){
            if( source_index == 56 ) ret.white_queen_castling = false;
            if( source_index == 63 ) ret.white_king_castling = false;
        }
    } 
    else if( source_square & COLOR_BLACK ){
        if( source_square & TYPE_KING ){
            ret.black_king_castling = false;
            ret.black_queen_castling = false;
        } 
        else if ( source_square & TYPE_ROOK ){
            if( source_index == 0 ) ret.black_queen_castling = false;
            if( source_index == 7 ) ret.black_king_castling = false;
        }
    }
    if( target_square & TYPE_ROOK ){
        if( target_square & COLOR_WHITE ){
            if(target_index == 56) ret.white_queen_castling = false;
            if(target_index == 63) ret.white_king_castling = false;
        }
        else if( target_square & COLOR_BLACK ){
            if(target_index == 0) ret.black_queen_castling = false;
            if(target_index == 7) ret.black_king_castling = false;
        }
    }

    // Counting clock on moves
    if( old_state.map[target_index] || source_square&TYPE_PAWN ){ 
        ret.half_move_clock = 0; 
    } 
    else {
        ret.half_move_clock ++;
    }
    
    // Toggling color turn
    if( old_state.next_color_to_play == COLOR_BLACK ){
        ret.next_color_to_play = COLOR_WHITE;
        ret.move_counter++;
    } 
    else {
        ret.next_color_to_play = COLOR_BLACK;
    }

    ret.map[source_index] = 0;
    ret.map[target_index] = old_state.map[source_index];
    
    // Promotion
    if( (source_square&TYPE_PAWN) ){
        bool promoting = false;
        if( source_square&COLOR_WHITE && target_index>=0 && target_index<=7 ) promoting = true ;
        if( source_square&COLOR_BLACK && target_index>=56 && target_index<=63 ) promoting = true ;
        if( promoting ) ret.map[target_index] = promotion_unit + (source_square&(COLOR_WHITE+COLOR_BLACK));
    }

    // En passant opportunity
    if( source_square&TYPE_PAWN && abs(delta_index)==16 ){
        if( source_square&COLOR_WHITE ) ret.en_passant = source_index-8;
        if( source_square&COLOR_BLACK ) ret.en_passant = source_index+8;
    } 
    else {
        ret.en_passant = -1;
    }
    
    // Castling execution
    if( source_square&TYPE_KING && abs(delta_index)==2 ){
        if( source_square&COLOR_WHITE ){
            if(delta_index>0){
                //king castling
                ret.white_king_castling = false;
                ret.map[61] = old_state.map[63];
                ret.map[63] = 0;
            } else {
                //queen castling
                ret.white_queen_castling = false;
                ret.map[59] = old_state.map[56];
                ret.map[56] = 0;
            }
        }
        if( source_square&COLOR_BLACK ){
            if(delta_index>0){
                //king castling
                ret.black_king_castling = false;
                ret.map[5] = old_state.map[7];
                ret.map[7] = 0;
            } else {
                //queen castling
                ret.black_queen_castling = false;
                ret.map[3] = old_state.map[0];
                ret.map[0] = 0;
            }
        }
    }

    return ret;
}
TableState      apply_move( TableState* ptr_table_state , const char* move ){
    // Assumes that move is valid. Unexpected behavior if not!
    int source_index = get_index_from_notation( move );
    int target_index = get_index_from_notation( &(move[2]) );
    int promotion = 0;
    switch( move[4] ){
        case 'q': promotion = TYPE_QUEEN; break;
        case 'n': promotion = TYPE_KNIGHT; break;
        case 'b': promotion = TYPE_BISHOP; break;
        case 'r': promotion = TYPE_ROOK; break;
        default:  promotion = TYPE_QUEEN; break; // If not specified, consider promotion as queen
    }
    
    return apply_move( ptr_table_state , source_index , target_index , promotion );
}
const char*     apply_move( const char* fen_string , const char* move ){
    TableState ts;
    if( !get_table_state(fen_string,&ts) ) return nullptr;
    ts = apply_move( &ts , move );
    return to_fen_string(&ts);
}
int64           fill_available_targets( TableState* ptr_table_state , int player_color ){
    int64 ret = 0;
    for( int y = 0 ; y < 8 ; y++ )
    for( int x = 0 ; x < 8 ; x++ )
    {
        int index = x + 8*y;
        int square = ptr_table_state->map[index];
        if( square && (square&player_color) ){
            if( square & TYPE_PAWN ){
                if( player_color==COLOR_WHITE && y > 0 ){
                    if(x<7) ret |= (1ULL<<(index-7));
                    if(x>0) ret |= (1ULL<<(index-9));
                }
                else 
                if( player_color==COLOR_BLACK && y < 7 ){
                    if(x>0) ret |= (1ULL<<(index+7));
                    if(x<7) ret |= (1ULL<<(index+9));
                }
            } else if( square & TYPE_KNIGHT ){
                int dxs[] = {-2,-2,-1,-1,1,1,2,2};
                int dys[] = {-1,1,-2,2,-2,2,-1,1};
                for( int j = 0 ; j < 8 ; j++ ){
                    int dx = dxs[j] , dy = dys[j];
                    if( x+dx>=0 && x+dx <=7 && y+dy>=0 && y+dy <=7 ){
                        int target_index = (x+dx) + (y+dy)*8;
                        ret |= (1ULL<<target_index);
                    }
                }
            } else if( square & TYPE_ROOK ){
                int dxs[] = {-1,1,0,0};
                int dys[] = {0,0,-1,1};
                for( int j = 0 ; j < 4 ; j++ ){
                    int dx = dxs[j] , dy = dys[j];
                    for( int steps = 1 ; x+dx*steps>=0 && x+dx*steps <=7 && y+dy*steps>=0 && y+dy*steps <=7 ; steps++ ){
                        int target_index = (x+dx*steps) + (y+dy*steps)*8;
                        int target_square = ptr_table_state->map[target_index];
                        if( target_square & player_color ) break;
                        ret |= (1ULL<<target_index);
                        if( target_square && !(target_square & player_color) ) break;
                    }
                }
            } else if( square & TYPE_BISHOP ){
                int dxs[] = {-1,-1,1,1};
                int dys[] = {-1,1,-1,1};
                for( int j = 0 ; j < 4 ; j++ ){
                    int dx = dxs[j] , dy = dys[j];
                    for( int steps = 1 ; x+dx*steps>=0 && x+dx*steps <=7 && y+dy*steps>=0 && y+dy*steps <=7 ; steps++ ){
                        int target_index = (x+dx*steps) + (y+dy*steps)*8;
                        int target_square = ptr_table_state->map[target_index];
                        if( target_square & player_color ) break;
                        ret |= (1ULL<<target_index);
                        if( target_square && !(target_square & player_color) ) break;
                    }
                }
            } else if( square & TYPE_QUEEN ){
                int dxs[] = {-1,1,0,0 , -1,-1,1,1};
                int dys[] = {0,0,-1,1 , -1,1,-1,1};
                for( int j = 0 ; j < 8 ; j++ ){
                    int dx = dxs[j] , dy = dys[j];
                    for( int steps = 1 ; x+dx*steps>=0 && x+dx*steps <=7 && y+dy*steps>=0 && y+dy*steps <=7 ; steps++ ){
                        int target_index = (x+dx*steps) + (y+dy*steps)*8;
                        int target_square = ptr_table_state->map[target_index];
                        if( target_square & player_color ) break;
                        ret |= (1ULL<<target_index);
                        if( target_square && !(target_square & player_color) ) break;
                    }
                }
            } else if( square & TYPE_KING ){
                int dxs[] = {-1,1,0,0 , -1,-1,1,1};
                int dys[] = {0,0,-1,1 , -1,1,-1,1};
                for( int j = 0 ; j < 8 ; j++ ){
                    int dx = dxs[j] , dy = dys[j];
                    if( x+dx>=0 && x+dx <=7 && y+dy>=0 && y+dy <=7 ){
                        int target_index = (x+dx) + (y+dy)*8;
                        if(  (ptr_table_state->map[target_index] & player_color) ) break;
                        ret |= (1ULL<<target_index);
                    }
                }
            }
        }
    }
    return ret;
}
bool            is_king_in_check(TableState* ptr_table_state , int king_color ){
    TableState& table_state = *ptr_table_state;
    int king_index=-1;
    for( int i = 0 ; i < 64 && king_index==-1 ; i++ )
        if( table_state.map[i] == (TYPE_KING|king_color) )
            king_index = i;
    int opponent_color = ( (king_color == COLOR_BLACK) ?(COLOR_WHITE):(COLOR_BLACK));
    int64 danger_zone = fill_available_targets(ptr_table_state,opponent_color);
    return danger_zone & (1ULL<<king_index);
}
INVALID_REASON  is_move_invalid( TableState* ptr_table_state , int source_index , int target_index  ){
    TableState& table_state = *ptr_table_state;
    if( target_index == source_index ) return SAME_PLACE;

    int source_square = table_state.map[source_index];
    int target_square = table_state.map[target_index];

    if( source_square & target_square & (COLOR_BLACK|COLOR_WHITE) ) return CAPTURING_SAME_COLOR;
    
    int source_x = source_index % 8;
    int source_y = source_index / 8;
    int target_x = target_index % 8;
    int target_y = target_index / 8;
    int delta_x = target_x - source_x;
    int delta_y = target_y - source_y;
    int delta_index = target_index - source_index;

    if( !source_square ) return NO_UNIT;

    if( (source_square&COLOR_BLACK) && table_state.next_color_to_play == COLOR_WHITE ) return NOT_PLAYER_TURN;
    if( (source_square&COLOR_WHITE) && table_state.next_color_to_play == COLOR_BLACK ) return NOT_PLAYER_TURN;
    
    if( source_square & SQUARE::TYPE_PAWN ){
        if( (source_square&COLOR_BLACK) && delta_y < 0 ) return INVALID_UNIT_MOVE;
        if( (source_square&COLOR_WHITE) && delta_y > 0 ) return INVALID_UNIT_MOVE;
        bool black_first_move = (source_square&COLOR_BLACK) && source_y == 1;
        bool white_first_move = (source_square&COLOR_WHITE) && source_y == 6;
        bool first_move = black_first_move || white_first_move;
        bool capturing = target_square;
        capturing = capturing || ( target_index == table_state.en_passant ); 
        if( capturing ){
            if( abs(delta_index)!=9 && abs(delta_index)!=7 )
                return INVALID_UNIT_MOVE;
        } else {
            if( abs(delta_x) > 0 ) return INVALID_UNIT_MOVE;
            if( first_move ){
                if( abs(delta_y) > 2 ) return INVALID_UNIT_MOVE;
                if( abs(delta_y) == 2 ){
                    if( table_state.map[source_index+delta_index/2] ) 
                        return INVALID_UNIT_MOVE; // Can't jump over units
                }
            }
        }
    } 
    else if ( source_square & SQUARE::TYPE_KNIGHT ){
        if( abs(delta_x) + abs(delta_y) != 3 ) return INVALID_UNIT_MOVE;
        if( abs(delta_y) == 3 || abs(delta_x) == 3 ) return INVALID_UNIT_MOVE;
    } 
    else if ( source_square & SQUARE::TYPE_ROOK ){
        int delta_direction; // +-1 , +-8
        if( (delta_y != 0) && (delta_x != 0) ) return INVALID_UNIT_MOVE;
        else if( delta_x > 0 ) delta_direction = 1;
        else if( delta_x < 0 ) delta_direction = -1;
        else if( delta_y > 0 ) delta_direction = 8;
        else if( delta_y < 0 ) delta_direction = -8;
        for( int intermediate_index = source_index + delta_direction ; intermediate_index != target_index ; intermediate_index += delta_direction )
            if( table_state.map[intermediate_index] ) return INVALID_UNIT_MOVE;
    } 
    else if ( source_square & SQUARE::TYPE_BISHOP ){
        int delta_direction; // +-7 , +-9
        if( abs(delta_y) != abs(delta_x) ) return INVALID_UNIT_MOVE;
        delta_direction = -9 + 16*(delta_y>0) + 2*(delta_x>0);
        for( int intermediate_index = source_index + delta_direction ; intermediate_index != target_index ; intermediate_index += delta_direction )
            if( table_state.map[intermediate_index] ) return INVALID_UNIT_MOVE;
    } 
    else if ( source_square & SQUARE::TYPE_QUEEN ){
        
        int delta_direction;
        if( (abs(delta_x)==0) || (abs(delta_y)==0) ){
            if( delta_x > 0 ) delta_direction = 1;
            else if( delta_x < 0 ) delta_direction = -1;
            else if( delta_y > 0 ) delta_direction = 8;
            else if( delta_y < 0 ) delta_direction = -8;
        } 
        else if( abs(delta_x)==abs(delta_y) ){
            delta_direction = -9 + 16*(delta_y>0) + 2*(delta_x>0);
        } else return INVALID_UNIT_MOVE;
        
        for( int intermediate_index = source_index + delta_direction ; intermediate_index != target_index ; intermediate_index += delta_direction )
            if( table_state.map[intermediate_index] ) return INVALID_UNIT_MOVE;
    
    }
    else if ( source_square & SQUARE::TYPE_KING ){
        bool black_king_castling  = table_state.black_king_castling && (source_square&COLOR_BLACK) && (table_state.map[7]&COLOR_BLACK) && (table_state.map[7]&TYPE_ROOK) && source_index == 4 && target_index == 6 && (table_state.map[5]==0) && (table_state.map[6]==0) ;
        bool black_queen_castling = table_state.black_queen_castling && (source_square&COLOR_BLACK) && (table_state.map[0]&COLOR_BLACK) && (table_state.map[0]&TYPE_ROOK) && source_index == 4 && target_index == 2 && (table_state.map[1]==0) && (table_state.map[2]==0) && (table_state.map[3]==0) ;
        bool white_king_castling  = table_state.white_king_castling && (source_square&COLOR_WHITE) && (table_state.map[63]&COLOR_WHITE) && (table_state.map[63]&TYPE_ROOK) && source_index == 60 && target_index == 62 && (table_state.map[61]==0) && (table_state.map[62]==0) ;
        bool white_queen_castling = table_state.white_queen_castling && (source_square&COLOR_WHITE) && (table_state.map[56]&COLOR_WHITE) && (table_state.map[56]&TYPE_ROOK) && source_index == 60 && target_index == 58 && (table_state.map[57]==0) && (table_state.map[58]==0) && (table_state.map[59]==0) ;
        
        bool any_castling = ( black_king_castling || black_queen_castling || white_king_castling || white_queen_castling );
        if( !any_castling && ( abs(delta_x)>1 || abs(delta_y)>1 ) ) 
            return INVALID_UNIT_MOVE;
    } 

    int current_player_color = ptr_table_state->next_color_to_play;
    TableState next_state = apply_move( ptr_table_state , source_index , target_index , TYPE_QUEEN );
    bool king_in_check = is_king_in_check(&next_state,current_player_color);

    if( king_in_check ) return KING_IN_CHECK;
    
    return VALID;
}
INVALID_REASON  is_move_invalid( TableState* ptr_table_state , const char* move  ){
    int source_index = 0 ;
    int target_index = 0 ;
    source_index = get_index_from_notation(move);
    target_index = get_index_from_notation(&move[2]);
    return is_move_invalid(ptr_table_state,source_index,target_index);
}
INVALID_REASON  is_move_invalid( const char* fen_string , const char* move  ){
    TableState table_state;
    if( !get_table_state( fen_string , &table_state ) ) return INVALID_FEN_STRING;
    return is_move_invalid(&table_state, move );
}

void            fill_valid_moves( TableState* ptr_table_state ){
    if( ptr_table_state->first ) return;
    
    for( int y = 0 ; y < 8 ; y++ )
    for( int x = 0 ; x < 8 ; x++ )
    {
        int source_index = x + 8*y;
        unsigned char source_square = ptr_table_state->map[source_index];
        if( source_square & ptr_table_state->next_color_to_play ){
            switch( source_square & ( TYPE_PAWN|TYPE_KNIGHT|TYPE_BISHOP|TYPE_ROOK|TYPE_KING|TYPE_QUEEN ) ){
                case TYPE_PAWN:
                    for( int delta_x = -1 ; delta_x <= 1 ; delta_x ++ )
                    for( int delta_y = -2 ; delta_y <= 2 ; delta_y ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;
                case TYPE_KNIGHT:
                    for( int delta_x = -2 ; delta_x <= 2 ; delta_x ++ )
                    for( int delta_y = -2 ; delta_y <= 2 ; delta_y ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;
                case TYPE_BISHOP:
                    for( int delta = 1 ; delta <= 7 ; delta++ )
                    for( int delta_x = -delta ; delta_x <= delta ; delta_x += 2*delta )
                    for( int delta_y = -delta ; delta_y <= delta ; delta_y += 2*delta )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;

                case TYPE_ROOK:
                    for( int delta_x = -7 , delta_y = 0 ; delta_x <= 7 ; delta_x ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    for( int delta_y = -7 , delta_x = 0 ; delta_y <= 7 ; delta_y ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;

                case TYPE_QUEEN:
                    for( int delta_x = -7 ; delta_x <= 7 ; delta_x ++ )
                    for( int delta_y = -7 ; delta_y <= 7 ; delta_y ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;
                case TYPE_KING:
                    for( int delta_x = -1 ; delta_x <= 1 ; delta_x ++ )
                    for( int delta_y = -1 ; delta_y <= 1 ; delta_y ++ )
                    if( x+delta_x >= 0 && x+delta_x <= 7 && y+delta_y >= 0 && y+delta_y <= 7 ){
                        int target_index = x+delta_x + 8*(y+delta_y);
                        if( is_move_invalid(ptr_table_state,source_index,target_index) == VALID ){
                            push_available_move(ptr_table_state,source_index,target_index);
                        }
                    }
                    break;

            }
        }
    }
}

bool            is_check_mate( TableState* ptr_table_state ){
    if( is_king_in_check(ptr_table_state,ptr_table_state->next_color_to_play) == false ) return false;
    fill_valid_moves( ptr_table_state );
    return ptr_table_state->first == nullptr;
}
bool            is_check_mate( const char* fen_string ){
    TableState ts;
    get_table_state(fen_string,&ts);
    return is_check_mate( &ts );
}

DRAW_REASON     get_draw_reason( TableState* ptr_table_state ){
    if( ptr_table_state->move_counter >= 75 ){
        return DRAW_REASON::SEVENTYFIVE_MOVES;
    }
    int white_even_bishops = 0;
    int white_odd_bishops = 0;
    int white_knights = 0;
    int black_even_bishops = 0;
    int black_odd_bishops = 0;
    int black_knights = 0;
    bool rook = false;
    bool queen = false;
    bool pawn = false;
    for( int index = 0 ; index < 64 ; index++ ){
        int square = ptr_table_state->map[index];
        if( square&TYPE_QUEEN ){
            queen = true;
            break;
        }
        else if( square & TYPE_ROOK ){
            rook = true;
            break;
        }
        else if( square & TYPE_PAWN ){
            pawn = true;
            break;
        }
        else if( square & COLOR_BLACK ){
            if( square & TYPE_KNIGHT ){
                black_knights++;
            } else if( square & TYPE_BISHOP ){
                if( ( index/8 + index%8 )%2==0 )black_even_bishops++;
                if( ( index/8 + index%8 )%2==1 )black_odd_bishops++;
            }
        }
        else if( square & COLOR_WHITE ){
            if( square & TYPE_KNIGHT ){
                white_knights++;
            } else if( square & TYPE_BISHOP ){
                if( ( index/8 + index%8 )%2==0 )white_even_bishops++;
                if( ( index/8 + index%8 )%2==1 )white_odd_bishops++;
            }
        }
    }
    if( !rook && !queen && !pawn ){
        bool white_lone_king = (white_odd_bishops==0 && white_even_bishops==0 && white_knights==0 );
        bool black_lone_king = (black_odd_bishops==0 && black_even_bishops==0 && black_knights==0 );

        bool white_has_material = ( white_knights==2 && black_lone_king ) || (white_odd_bishops && white_even_bishops);
        bool black_has_material = ( black_knights==2 && white_lone_king ) || (black_odd_bishops && black_even_bishops);

        if( !white_has_material && !black_has_material ){
            return DRAW_REASON::INSUFFICIENT_MATERIAL;
        }
    }

    fill_valid_moves(ptr_table_state);
    if( !is_king_in_check(ptr_table_state,ptr_table_state->next_color_to_play) && ptr_table_state->first==nullptr )
        return DRAW_REASON::STALEMATE;

    return DRAW_REASON::NO_DRAW;
}

}

#endif