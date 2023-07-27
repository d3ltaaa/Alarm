#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include "pitches.h"


// headers
void set_time();
void update_time();
void print_time();
void edit_elem(struct time_element* te);
void print_to_display(byte row, byte col, byte val);
void print_layout();

bool check_time();
void play_alarm();

byte check_buttons();


// Flags
bool Flag_play_alarm = false;
bool Flag_set_time = false;


// Structs
struct time_element {

    byte id; 
    byte val;
    byte row;
    byte col;
    byte spaces;

};


struct s_time {
    // setting up positions
    // row 0
    struct time_element day;
    struct time_element month;
    struct time_element year;
    struct time_element doW;
    struct time_element hour;
    struct time_element mins;
    struct time_element sec;


};

struct s_alarm {
    struct time_element hour;
    struct time_element mins;
    struct time_element index;
};

struct s_time ct;
struct s_alarm ca;

// Pins
const byte mainPin = 18; 
const byte piezoPin = 7;
const byte plusPin = 9;
const byte minsPin = 10;
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


DS3231 myRTC;

// Constants for DS3231
bool century; // needed for overflowing
bool H12;     // needs to be 0
bool PM_TIME; // needs to be 0

char days_of_week[7][3] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
char alarm_indicator[3] = {'0', 'A', 'W'};

// ###############################################################

void setup() {

    Serial.begin(9600);
    Wire.begin();
    lcd.begin(16, 2);

    // Pin-Activation
    pinMode(mainPin, INPUT_PULLUP);
    pinMode(piezoPin, OUTPUT);
    pinMode(plusPin, INPUT_PULLUP);
    pinMode(minsPin, INPUT_PULLUP);

    // setup attributes ct
    ct.day.id = 0;
    ct.day.row = 0;
    ct.day.col = 0;
    ct.day.spaces = 2;
    ct.month.id = 1;
    ct.month.row = 0;
    ct.month.col = 3;
    ct.month.spaces = 2;
    ct.year.id = 2;
    ct.year.row = 0;
    ct.year.col = 6;
    ct.year.spaces = 2;
    ct.doW.id = 3;
    ct.doW.row = 0;
    ct.doW.col = 11;
    ct.doW.spaces = 1;
    ct.hour.id = 4;
    ct.hour.row = 1;
    ct.hour.col = 0;
    ct.hour.spaces = 2;
    ct.mins.id = 5;
    ct.mins.row = 1;
    ct.mins.col = 3;
    ct.mins.spaces = 2;
    ct.sec.id = 6;
    ct.sec.row = 1;
    ct.sec.col = 6;
    ct.sec.spaces = 2;
    
    ca.hour.id = 7;
    ca.hour.row = 1;
    ca.hour.col = 11;
    ca.hour.spaces = 2;
    ca.mins.id = 8;
    ca.mins.row = 1;
    ca.mins.col = 14;
    ca.mins.spaces = 2;
    ca.index.id = 9;
    ca.index.row = 1;
    ca.index.col = 9;
    ca.index.spaces = 1;

}


void loop() {

    update_time();
    print_time();

    check_buttons();

    if (check_time()) Flag_play_alarm = true;

    if (Flag_play_alarm && !Flag_set_time) play_alarm();

    if (Flag_set_time && !Flag_play_alarm) set_time();

    Serial.print("Flag_play_alarm: ");
    Serial.print(Flag_play_alarm);
    Serial.print("  Flag_set_time:  ");
    Serial.println(Flag_set_time);

}

// ###############################################################

void update_time() {

    ct.day.val   = myRTC.getDate();
    ct.month.val = myRTC.getMonth(century);
    ct.year.val  = myRTC.getYear();
    ct.doW.val   = myRTC.getDoW();
    ct.hour.val  = myRTC.getHour(H12, PM_TIME);
    ct.mins.val  = myRTC.getMinute();
    ct.sec.val   = myRTC.getSecond();

}

void print_time() {
    
    print_to_display(ct.day.row,   ct.day.col,   ct.day.val,   ct.day.spaces);
    print_to_display(ct.month.row, ct.month.col, ct.month.val, ct.month.spaces);
    print_to_display(ct.year.row,  ct.year.col,  ct.year.val,  ct.year.spaces);

    lcd.setCursor(ct.doW.col, ct.doW.row);
    char day[4];
    for (int i = 0; i < 3; i++) {
        day[i] = days_of_week[ct.doW.val][i]; 
    }
    day[3] = '\0';
    lcd.print(day);

    print_to_display(ct.hour.row,  ct.hour.col,  ct.hour.val,  ct.hour.spaces);
    print_to_display(ct.mins.row,  ct.mins.col,  ct.mins.val,  ct.mins.spaces);
    print_to_display(ct.sec.row,   ct.sec.col,   ct.sec.val,   ct.sec.spaces);

    print_to_display(ca.hour.row,  ca.hour.col,  ca.hour.val,  ca.hour.spaces);
    print_to_display(ca.mins.row,  ca.mins.col,  ca.mins.val,  ca.mins.spaces);

    lcd.setCursor(ca.index.col, ca.index.row);
    char indicator = alarm_indicator[ca.index.val];
    lcd.print(indicator);

    print_layout();

}

void print_layout() {

    byte pos_points[2] = {2, 5}; // row 0

    for (int i = 0; i < 2; i++) {

        lcd.setCursor(pos_points[i], 0);
        lcd.print(".");

    } 

    byte pos_double[3] = {2, 5, 13};

    for (int i = 0; i < 3; i++) {

        lcd.setCursor(pos_double[i], 1);
        lcd.print(":");

    } 
    
}

void print_to_display(byte row, byte col, byte val, byte spaces) {
    if (spaces == 2) {
        char buff[3];
        sprintf(buff, "%02d", val);
        lcd.setCursor(col, row);
        lcd.print(buff);
    }
    else {
        lcd.setCursor(col, row);
        lcd.print(val);
    }

}

byte check_buttons() {
    
    // buttons with values:
    // noPin   mainPin plusPin minsPin
    // 0       1       2       3
    delay(500);

    if (digitalRead(mainPin) == LOW){
        if (Flag_play_alarm) Flag_play_alarm = false;
        if (!Flag_set_time) Flag_set_time = true;
        Serial.println("ButtonMain");
        return 1;
    }
    
    if (digitalRead(plusPin) == LOW) {
        Serial.println("Buttonplus"); 
        return 2;
    }

    if (digitalRead(minsPin) == LOW) {
        Serial.println("Buttonminus"); 
        return 3;
    }

    
    // no button pressed
    return 0;
}


bool check_time() {

    // return 1 if alarm matches
    if (ct.hour.val != ca.hour.val) return 0;
    if (ct.mins.val != ca.mins.val) return 0;
    if (ct.sec.val != 0) return 0;
    
    return 1;
}


void play_alarm() {
    
    Serial.println("Alarm!");

    while (Flag_play_alarm) {
        
        // go through each note
        for (int thisNote = 0; thisNote < 8; thisNote++) {
            
            // check if mainButton is pressed
            check_buttons();
            
            // return if mainButton was pressed and Flag changes
            if (!Flag_play_alarm) {
                Serial.println("Alarm cancelled!");
                return;
            }
            
            // calculate note duration
            int noteDuration = 1000 / noteDurations[thisNote];

            // play tone
            tone(piezoPin, melody[thisNote], noteDuration);
            
            // play pause
            int pause_between_notes = noteDuration * 1.30;

            delay(pause_between_notes);

            // stop play
            noTone(piezoPin);
            
            // update time and display it
            update_time();
            print_time();
        }
    }
}


void set_time() {

    Serial.println("Set time!");

    edit_elem(&ct.day);
    edit_elem(&ct.month);
    edit_elem(&ct.year);
    edit_elem(&ct.doW);
    edit_elem(&ct.hour);
    edit_elem(&ct.mins);

    Flag_set_time = false;
    

}

void edit_elem(struct time_element* te) {
    
    byte col = te->col;
    // set cursor and blink 
    lcd.setCursor(col, te->row);
    lcd.blink();
    
    if (te->spaces == 2) {

        bool go_on_to_next = false;
        while (go_on_to_next == false) {

            update_time();
            print_time();

            // set cursor and blink 
            lcd.setCursor(col, te->row);
            lcd.blink();

            // check buttons
            byte button_state = check_buttons();

            switch (button_state) {
                
                case 0: break;
                case 1: go_on_to_next = true; break;
                case 2: Serial.println("+10"); time_setup((te->val) + 10, te->id); break;
                case 3: Serial.println("-10"); time_setup((te->val) - 10, te->id); break; 
            }
            
        }

        col++;
    }

    
    // loop as long as go_on_to_next is false
    bool go_on_to_next = false;
    while (go_on_to_next == false) {
        
        update_time();
        print_time();
        
        // set cursor and blink 
        lcd.setCursor(col, te->row);
        lcd.blink();

        // check buttons
        byte button_state = check_buttons();

        switch (button_state) {

            case 0: break;
            case 1: go_on_to_next = true; break;
            case 2: Serial.println("+1"); time_setup((te->val) + 1, te->id); break;
            case 3: Serial.println("-1"); time_setup((te->val) - 1, te->id); break; 
        }
    } 


    lcd.noBlink();
    


}


void time_setup(byte new_time, byte id){
    switch (id) {
        case 0: myRTC.setDate(new_time); break;
        case 1: myRTC.setMonth(new_time); break;
        case 2: myRTC.setYear(new_time); break;
        case 3: myRTC.setDoW(new_time); break;
        case 4: myRTC.setHour(new_time); break;
        case 5: myRTC.setMinute(new_time); break;
    }
}
