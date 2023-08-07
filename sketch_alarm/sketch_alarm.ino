#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#include "pitches.h"


// headers
void set_time();
void update_time();
void print_time();
void edit_elem(struct time_element* te);
void print_to_display(byte row, byte col, byte val);
void print_layout();
void leds(byte level);

byte check_time();
void play_alarm();

byte check_buttons();


// Flags
bool Flag_play_alarm = false;
bool Flag_set_time = false;
byte Flag_alarm_light_level = 0;


// Structs
struct time_element {

    byte id; 
    byte val;
    byte row;
    byte col;
    byte max_num;
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
const byte LED_PIN = 6;
const byte MAIN_PIN = 18; 
const byte PIEZO_PIN = 7;
const byte PLUS_PIN = 9;
const byte MINS_PIN = 10;
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Constants for DS3231
bool century; // needed for overflowing
bool H12;     // needs to be 0
bool PM_TIME; // needs to be 0

const byte LED_COUNT = 12;
byte LIGHT_LEVELS[8] = {0, 255, 155, 100, 70, 40, 20, 10};

char DAYS_OF_WEEK[7][3] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
char ALARM_INDICATOR[3] = {'0', 'A', 'W'};



DS3231 myRTC;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


// ###############################################################

void setup() {

    Serial.begin(9600);
    Wire.begin();
    lcd.begin(16, 2);
    strip.begin();
    strip.show();

    // Pin-Activation
    pinMode(MAIN_PIN, INPUT_PULLUP);
    pinMode(PIEZO_PIN, OUTPUT);
    pinMode(PLUS_PIN, INPUT_PULLUP);
    pinMode(MINS_PIN, INPUT_PULLUP);

    // setup attributes ct
    ct.day.id = 0;
    ct.day.row = 0;
    ct.day.col = 0;
    ct.day.spaces = 2;
    ct.day.max_num = 31;

    ct.month.id = 1;
    ct.month.row = 0;
    ct.month.col = 3;
    ct.month.spaces = 2;
    ct.month.max_num = 12;

    ct.year.id = 2;
    ct.year.row = 0;
    ct.year.col = 6;
    ct.year.spaces = 2;
    ct.year.max_num = 0;

    ct.doW.id = 3;
    ct.doW.row = 0;
    ct.doW.col = 11;
    ct.doW.spaces = 1;
    ct.doW.max_num = 6;

    ct.hour.id = 4;
    ct.hour.row = 1;
    ct.hour.col = 0;
    ct.hour.spaces = 2;
    ct.hour.max_num = 24;

    ct.mins.id = 5;
    ct.mins.row = 1;
    ct.mins.col = 3;
    ct.mins.spaces = 2;
    ct.mins.max_num = 60;

    ct.sec.id = 6;
    ct.sec.row = 1;
    ct.sec.col = 6;
    ct.sec.spaces = 2;
    
    ca.hour.id = 7;
    ca.hour.row = 1;
    ca.hour.col = 11;
    ca.hour.spaces = 2;
    ca.hour.max_num = 24;

    ca.mins.id = 8;
    ca.mins.row = 1;
    ca.mins.col = 14;
    ca.mins.spaces = 2;
    ca.mins.max_num = 60;

    ca.index.id = 9;
    ca.index.row = 1;
    ca.index.col = 9;
    ca.index.spaces = 1;
    ca.index.max_num = 2;

    //temp
    ca.index.val = 1;
    ca.hour.val = 6;
    ca.mins.val = 0;

}


void loop() {

    update_time();
    print_time();

    check_buttons();

    byte time_check = check_time();
    Serial.println();
    Serial.print("Status: ");
    Serial.println(time_check);

    // ca.index.val != 0 bc alarm must be enabled
    if (time_check == 1 && ca.index.val != 0) {
        switch (ca.index.val) {
            case 1:
                Flag_play_alarm = true;
            case 2:
                if (ct.doW.val != 5 && ct.doW.val != 6) Flag_play_alarm = true;
        }
    }
    else if (time_check != 1) leds(time_check);

    if (Flag_play_alarm && !Flag_set_time) play_alarm();

    if (Flag_set_time && !Flag_play_alarm) set_time();
    
    /*
    Serial.print("Flag_play_alarm: ");
    Serial.print(Flag_play_alarm);
    Serial.print("  Flag_set_time:  ");
    Serial.println(Flag_set_time);
    */
    Serial.println("--------------------");
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
        day[i] = DAYS_OF_WEEK[ct.doW.val][i]; 
    }
    day[3] = '\0';
    lcd.print(day);

    print_to_display(ct.hour.row,  ct.hour.col,  ct.hour.val,  ct.hour.spaces);
    print_to_display(ct.mins.row,  ct.mins.col,  ct.mins.val,  ct.mins.spaces);
    print_to_display(ct.sec.row,   ct.sec.col,   ct.sec.val,   ct.sec.spaces);

    print_to_display(ca.hour.row,  ca.hour.col,  ca.hour.val,  ca.hour.spaces);
    print_to_display(ca.mins.row,  ca.mins.col,  ca.mins.val,  ca.mins.spaces);

    lcd.setCursor(ca.index.col, ca.index.row);
    char indicator = ALARM_INDICATOR[ca.index.val];
    lcd.print(indicator);

    print_layout();

    // print to Serial
    Serial.print(ct.day.val);
    Serial.print(".");
    Serial.print(ct.month.val);
    Serial.print(".");
    Serial.print(ct.year.val);
    Serial.print("  ");
    Serial.print(DAYS_OF_WEEK[ct.doW.val][0]);
    Serial.print(DAYS_OF_WEEK[ct.doW.val][1]);
    Serial.print(DAYS_OF_WEEK[ct.doW.val][2]);
    Serial.print("(");
    Serial.print(ct.doW.val);
    Serial.print(")");
    Serial.print("  ");
    Serial.print(ct.hour.val);
    Serial.print(":");
    Serial.print(ct.mins.val);
    Serial.print(":");
    Serial.print(ct.sec.val);
    Serial.print("  ---  ");
    Serial.print(ALARM_INDICATOR[ca.index.val]);
    Serial.print("  ");
    Serial.print(ca.hour.val);
    Serial.print(":");
    Serial.print(ca.mins.val);
    if (Flag_set_time) Serial.println();

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
    // noPin   MAIN_PIN PLUS_PIN MINS_PIN
    // 0       1       2       3
    if (!Flag_play_alarm) delay(500);

    if (digitalRead(MAIN_PIN) == LOW){
        if (!Flag_set_time && !Flag_play_alarm) Flag_set_time = true;
        if (Flag_play_alarm) Flag_play_alarm = false;
        Serial.print("-->ButtonMain  ");
        return 1;
    }
    
    if (digitalRead(PLUS_PIN) == LOW) {
        Serial.print("-->Buttonplus   "); 
        return 2;
    }

    if (digitalRead(MINS_PIN) == LOW) {
        Serial.print("-->Buttonminus   "); 
        return 3;
    }

    
    // no button pressed
    return 0;
}


byte check_time() {

    // return 7 if alarm is 30 mins before
    //        6             25 mins
    //        5             20 mins
    //        4             15 mins
    //        3             10 mins
    //        2              5 mins
    //        1 if alarm matches
    //        0 if neither
   

    // calculate difference between alarm_time and current_time
    
    byte hour_diff = ca.hour.val - ct.hour.val;
    
    byte mins_diff = 0;
        

    if (hour_diff < 0 || hour_diff > 2) return 0;  

    else if (hour_diff == 0) {
        
        mins_diff = ca.mins.val - ct.mins.val;

    }

    else if (hour_diff == 1) {
        
        mins_diff = (60 - ct.mins.val) + ca.mins.val;

    } 
    
    Serial.print("Hour_diff:");
    Serial.print(hour_diff);
    Serial.print("   Mins_diff: ");
    Serial.println(mins_diff);

    if (mins_diff > 30) return 0; 
    else if (mins_diff > 25) return 7;
    else if (mins_diff > 20) return 6;
    else if (mins_diff > 15) return 5;
    else if (mins_diff > 10) return 4;
    else if (mins_diff > 5)  return 3;
    else if (mins_diff > 0)  return 2;
    else if (mins_diff == 0 && ct.sec.val == 0) {
        return 1;
    }

    return 0;
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
                delay(500);
                return;
            }
            
            // calculate note duration
            int noteDuration = 1000 / noteDurations[thisNote];

            // play tone
            tone(PIEZO_PIN, melody[thisNote], noteDuration);
            
            // play pause
            int pause_between_notes = noteDuration * 1.30;

            delay(pause_between_notes);

            // stop play
            noTone(PIEZO_PIN);
            
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
    edit_elem(&ca.index);
    edit_elem(&ca.hour);
    edit_elem(&ca.mins);

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
                case 2: 
                    Serial.print("  +10  "); 
                    te->val = te->val + 10; 

                    if (te->max_num != 0 && te->val > te->max_num)
                    {
                        te->val = 0;
                    }

                    time_setup(te); 
                    break;
                case 3: 
                    Serial.print("  -10  "); 
                    te->val = te->val - 10; 

                    if (te->max_num != 0 && te->val > te->max_num)
                    {
                        te->val = te->max_num;
                    }

    
                    time_setup(te); 
                    break; 
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
            case 2: 
                Serial.print("  +1  "); 
                te->val = te->val + 1; 

                if (te->max_num != 0 && te->val > te->max_num)
                {
                    te->val = 0;
                }

                time_setup(te); 
                break;
            case 3: 
                Serial.print("  -1  "); 
                te->val = te->val - 1; 

                if (te->max_num != 0 && te->val > te->max_num)
                {
                    te->val = te->max_num;
                }

                time_setup(te); 
                break; 
        }
    } 


    lcd.noBlink();
    


}


void time_setup(struct time_element* te){
    byte new_time = te->val;
    byte id = te->id;
    switch (id) {
        case 0: myRTC.setDate(new_time); break;
        case 1: myRTC.setMonth(new_time); break;
        case 2: myRTC.setYear(new_time); break;
        case 3: myRTC.setDoW(new_time); break;
        case 4: myRTC.setHour(new_time); break;
        case 5: myRTC.setMinute(new_time); break;
        case 7: break;
        case 8: break;
        case 9: break;
    }
}

void leds(byte level) {
    for (int i = 0; i < 12; i++) { 
        strip.setPixelColor(i, LIGHT_LEVELS[level], LIGHT_LEVELS[level], LIGHT_LEVELS[level]);
        strip.show();
    }
}
