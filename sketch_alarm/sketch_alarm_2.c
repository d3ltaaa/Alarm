// Structs
struct time_element_byte {
    
    byte value;
    byte row;
    byte col;
    bool two_spaces;

};

struct time_element_string {
    
    char value[3];
    byte row;
    byte col;
    bool two_spaces;

};

struct time_element_char {

    char value;
    byte row;
    byte col;
    byte spaces;

};



struct s_time {
    // setting up positions
    // row 0
    struct time_element_byte day.row = 0;
    struct time_element_byte day.col = 0;
    struct time_element_byte day.spaces = 2;

    struct time_element_byte month.row = 0;
    struct time_element_byte month.col = 3;
    struct time_element_byte month.spaces = 2;

    struct time_element_byte year.row = 0;
    struct time_element_byte year.col = 6;
    struct time_element_byte year.spaces = 2;

    struct time_element_string doW.row = 0;
    struct time_element_string doW.col = 11;
    struct time_element_string doW.spaces = 3;

    // row 1
    struct time_element_byte hour.row = 1;
    struct time_element_byte hour.col = 0;
    struct time_element_byte hour.spaces = 2;

    struct time_element_byte mins.row = 1;
    struct time_element_byte mins.col = 3;
    struct time_element_byte mins.spaces = 2;

    struct time_element_byte sec.row = 1;
    struct time_element_byte sec.col = 6;
    struct time_element_byte sec.spaces = 2;

};

struct s_alarm {

    struct time_element_byte hour.row = 1;
    struct time_element_byte hour.col = 11;
    struct time_element_byte hour.spaces = 2;

    struct time_element_byte mins.row = 1;
    struct time_element_byte mins.col = 14;
    struct time_element_byte mins.spaces = 2;
    
    struct time_element_char index.row = 1;
    struct time_element_char index.col = 9;
    struct time_element_char index.spaces = 1;

}

struct s_time ct;
struct s_time ca;

