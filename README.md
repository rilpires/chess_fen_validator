See demo.cpp, it is really straightforward:

- You construct a TableState object from a fen string using get_table_state.
- If it was valid (check value returned), you can call some functions like:
    - is_king_in_check
    - is_checkmate
    - get_draw_reason
        - Returns enum of draw reason.
    - is_move_invalid                    
        - Returns enum of invalid reason.
    - apply_move(TableState*,const char*)
        - Returns a new table state. **Check beforehand if move is valid**, unexpected behavior if not!

