/* 
    3D Puzzle Game
    Thomas A. Groom, University of Leeds, April 2021
*/

#include <ctime>
#include <vector>

#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "Utils.h"
#include "Game.h"
#include "Object.h"
#include "Menu.h"
#include "Data.h"

// initialise pin allocations
Joystick joystick(p20, p19);
InterruptIn buttonA(p29);
InterruptIn buttonB(p28);
InterruptIn buttonC(p27);
InterruptIn buttonD(p26);
InterruptIn buttonJ(p17);
Ticker debounceDelay;
Ticker jsRead;

Object Cube2;
Object Trophy;
Object Obj;


// initialise global variables
volatile bool g_buttonA_flag = 0; //set in interupts
volatile bool g_buttonB_flag = 0;
volatile bool g_buttonC_flag = 0;
volatile bool g_buttonD_flag = 0;
volatile bool g_buttonJ_flag = 0;

volatile bool g_buttonA = 0; //set at the beginning of each frame
volatile bool g_buttonB = 0;
volatile bool g_buttonC = 0;
volatile bool g_buttonD = 0;
volatile bool g_buttonJ = 0;
Quaternion g_rotation = ToQuaternion(-PI/4, PI/4, -PI/4);
Quaternion g_diff_rotation = {0,0,0,1};

void buttonJ_isr();

volatile bool g_jsRead_flag = 0;

struct State {
    int enumerated;
    int next_state[4];
};

Game GlobalGame; // global game object

// set the button A flag
void buttonA_isr()
{
    g_buttonA_flag = 1; // set button A flag
}

// set the button B flag
void buttonB_isr()
{
    g_buttonB_flag = 1; // set button B flag
}

// set the button C flag
void buttonC_isr()
{
    g_buttonC_flag = 1; // set button C flag
}

// set the button D flag
void buttonD_isr()
{
    g_buttonD_flag = 1; // set button D flag
}


// read the joystick values at constant intervals
void jsRead_isr()
{
    Vector2D coord = joystick.get_mapped_coord();
    g_diff_rotation = g_rotation;
    g_rotation = ToQuaternion(0, coord.x/30, coord.y/30) * g_diff_rotation;
}


void resetRotation(){
    g_rotation = ToQuaternion(-PI/4, PI/4, -PI/4);
}


void delay_isr(){
     // detach the timer and read the joystick button
     debounceDelay.detach();
     g_buttonJ_flag = buttonJ.read();
}


void buttonJ_isr() {
    // set a timer interupt to be executed in 10 ms
    debounceDelay.attach(&delay_isr, 0.01);
}


void setGlobalButtons()
{
    // reset the button variables
    g_buttonA = 0;
    g_buttonB = 0;
    g_buttonC = 0;
    g_buttonD = 0;
    g_buttonJ = 0;
    
    // conditionally set the button variables
    if(g_buttonA_flag == 1){
        g_buttonA = 1;
        g_buttonA_flag = 0;
    }
    if(g_buttonB_flag == 1){
        g_buttonB = 1;
        g_buttonB_flag = 0;
    }
    if(g_buttonC_flag == 1){
        g_buttonC = 1;
        g_buttonC_flag = 0;
    }
    if(g_buttonD_flag == 1){
        g_buttonD = 1;
        g_buttonD_flag = 0;
    }
    if(g_buttonJ_flag == 1){
        g_buttonJ = 1;
        g_buttonJ_flag = 0;
    }
}


// Draws the level select menu boxes
void drawLvlBox(unsigned char lvl, int i, int j)
{   
    // margins and level spacings
    int x = i*11+10;
    int y = j*11+15;
    
    // if level has cursor, draw cursor
    if ((lvl & 0b00000100) == 0b0000100){ 
        lcd.drawRect(x-1 , y-1, 9, 9, FILL_TRANSPARENT);
        lcd.drawRect(x+2 , y-1, 3, 9, FILL_WHITE);
        lcd.drawRect(x-1 , y+2, 9, 3, FILL_WHITE);
    }
    // if level is locked, draw locked
    if ((lvl & 0b00000011) == 0b00000000){
        lcd.drawRect(x  , y  , 7, 7, FILL_TRANSPARENT);
        lcd.drawRect(x+2, y  , 3, 7, FILL_WHITE);
        lcd.drawRect(x  , y+2, 7, 3, FILL_WHITE);
    }
    // else if level is unlocked, draw unlocked
    else if ((lvl & 0b00000011) == 0b00000001){
        lcd.drawRect(x  , y  , 7, 7, FILL_TRANSPARENT);
    }
    // else if level has been failed, draw a failed level
    else if((lvl & 0b00000011) == 0b0000010){ // failed
        lcd.drawRect(x  , y  , 7, 7, FILL_TRANSPARENT);
        lcd.drawLine(x,y,x+6,y+6,FILL_BLACK);
        lcd.drawLine(x+6,y,x,y+6,FILL_BLACK);
    }
    // else if level has been completed successfully, draw a successful level
    else if((lvl & 0b00000011) == 0b0000011){
        lcd.drawRect(x   , y  , 7, 7, FILL_TRANSPARENT);
        lcd.drawRect(x+2, y+2 , 3, 3, FILL_BLACK);
    }
}


// Draws the title screen
void drawStartScreen(int angle){
    lcd.clear();
    clearDepth(); 
    Quaternion rotation = ToQuaternion((angle/570.0)*PI, (angle/890.0)*PI, (angle/530.0)*PI);
    Vector3 relative_loc = {0,0,0};
    Vector3 location  = {42,24,0};
    Vector3 scale = {13,13,13};
    Cube2.setLocRotScale(location, relative_loc, rotation, scale);
    Cube2.draw(FILL_WHITE);
}


// Displays a trophy at a specified angle to create an animation
void winAnimation(int angle){
    lcd.clear();
    clearDepth(); 
    Quaternion rotation = ToQuaternion(PI, (angle/36.0)*PI, 0);
    Vector3 relative_loc = {0,0,0};
    Vector3 location  = {42,26,0};
    Vector3 scale = {12,12,12};
    Trophy.setLocRotScale(location, relative_loc, rotation, scale);
    Trophy.draw(FILL_WHITE);
    lcd.drawRect(18,35,84-37, 11, FILL_TRANSPARENT);
    lcd.drawRect(18+1,35+1,84-37-2, 11-2, FILL_WHITE);
    lcd.printString("YOU WIN",centerAlign("YOU WIN"),37);
    lcd.refresh();
}


// Draws the specified object to the screen
void drawObject(float scale_factor, Object Obj){
    lcd.clear();
    clearDepth(); 
    Quaternion rotation = ToQuaternion(PI, (160.0/36.0)*PI, 0);
    Vector3 relative_loc = {0,0,0};
    Vector3 location  = {42,24,0};
    Vector3 scale = {15,15,15};
    Obj.setLocRotScale(location, relative_loc, g_rotation, scale_factor*scale);
    Obj.draw(FILL_WHITE);
    lcd.refresh();
}


// Define the menu objects
Menu mainMenu;
Menu loseMenu;

int main()
{
    Cube2.setObjData(cube_object2);
    Trophy.setObjData(trophy_object);
    Obj.setObjData(suzanne_object);
    
    int lvl_select = 0;
    unsigned char lvl_arr[24] = 
        {0b1, 0b0, 0b0, 0b0, 0b0, 0b0,
         0b0, 0b0, 0b0, 0b0, 0b0, 0b0,
         0b0, 0b0, 0b0, 0b0, 0b0, 0b0,
         0b0, 0b0, 0b0, 0b0, 0b0, 0b0};
    
    // lcd, joystick & interrupt initialisation
    lcd.init();
    lcd.setContrast(0.4);
    lcd.backLightOn();
    joystick.init();
    buttonJ.rise(&buttonJ_isr);
    buttonJ.mode(PullNone);
    
    // attach routines to the buttons
    buttonA.fall(&buttonA_isr);
    buttonA.mode(PullNone);
    buttonB.fall(&buttonB_isr);
    buttonB.mode(PullNone);
    buttonC.fall(&buttonC_isr);
    buttonC.mode(PullNone);
    buttonD.fall(&buttonD_isr);
    buttonD.mode(PullNone);
    
    jsRead.attach(&jsRead_isr, 0.05);
    
    State g_fsm[10] = {
        {0, {1,1,1,0}}, // Title Screen
        {1, {2,7,8,1}}, // Main Menu
        {2, {2,3,3,3}}, // Level Select
        {3, {3,5,6,3}}, // Game
        {4, {1,1,1,1}}, 
        {5, {2,2,2,2}}, // Win
        {6, {3,2,1,6}}, // Lose
        {7, {7,1,1,1}}, // 3D Objects
        {8, {9,9,9,9}}, // Controls A
        {9, {1,1,1,1}}, // Controls B
    };
    
    // setup the menu display names and menu options
    mainMenu.set_name("Main Menu");
    mainMenu.set_options("Start", "3D Objects","Controls");
    loseMenu.set_name("Game Lost!");
    loseMenu.set_options("Restart", "lvl Select", "Main Menu");
    
    // variables to manipulate the FSM state
    short selected = 0;
    short state = 0;
    
    // used to create a rotating win animation
    short angle = 0;
    
    float scale = 1.0;
    int sel_object = 0;
    
    while(1) {
        
        lcd.clear(); // clear the lcd
        setGlobalButtons(); // set global variables according to the button flags
        
        // if the joystick button was pushed, then change FSM state
        if (g_buttonJ == 1 && state != 3){
            state = g_fsm[state].next_state[selected];
            g_buttonJ = 0;
        }
        
        // change which option is selected based on the button D and B inputs
        selected += g_buttonD - g_buttonB;
        
        // constrain the selected item between 0 and 2
        if(selected < 0){
            selected = 2;
        }else if(selected > 2){
            selected = 0;
        }
        
        switch(state)
        {
            case 0: // TITLE SCREEN
            {
                selected = 0;
                
                // draw title screen animation
                angle++;
                drawStartScreen(angle);
                
                // draw start button
                lcd.drawRect(20,33,84-40, 11, FILL_TRANSPARENT);
                lcd.drawRect(20+1,33+1,84-40-2, 11-2, FILL_WHITE);
                lcd.printString("START",centerAlign("START"),35);
                break;
            }
            
            case 1: // MAIN MENU
            {
                // draw the main menu visuals
                resetRotation();
                mainMenu.draw(selected);
                sel_object = 0;
                Obj.setObjData(suzanne_object);
                break;
            }
                
            case 2: // LVL SELECT
            {
                lcd.printString("lvl Select", 11, 3);
                // defines how the buttons interact with the cursor
                // levels are arranged in a 6 wide x 4 high grid
                lvl_select += g_buttonA - g_buttonC + 6*g_buttonD - 6*g_buttonB;
                lvl_select = min(max(lvl_select,0),23); // constrain selection
                
                // 1. set selected level as having cursor, 2. draw all levels,
                // 3. reset the selected level cursor bit
                lvl_arr[lvl_select] |= 0b00000100; 
                // draws the array of levels in the level select menu
                for(int i = 0; i < 4; i ++){
                    for(int j = 0; j < 6; j ++){
                        drawLvlBox(lvl_arr[(i*6)+j], j, i);
                    }
                }
                lvl_arr[lvl_select] &= 0b00000011;
                
                if(lvl_arr[lvl_select] != 0b00000000){
                    // if level is not locked, continue displaying lvl sel menu
                    selected = 1;
                }else{
                    // go to the game state
                    selected = 0;
                    // initialise game object with selected level data
                }
                resetRotation();
                GlobalGame.init(levelSelect(lvl_select));
                break;
            }
                
            case 3: // GAME
            {
                int game_state = 0;
                // pass the button flag variables into the game object
                GlobalGame.setButtonAflag(g_buttonA);
                GlobalGame.setButtonBflag(g_buttonB);
                GlobalGame.setButtonCflag(g_buttonC);
                GlobalGame.setButtonDflag(g_buttonD);
                GlobalGame.setButtonJflag(g_buttonJ);
                
                GlobalGame.setNewRotation(g_rotation);
                
                // draw frame and get the game state (continue 0, lose 1, win 2)
                game_state = GlobalGame.getFrame();
                
                // determine which state to go to according the game state
                if (game_state == WIN){
                    selected = 1;
                }else if (game_state == LOSE){
                    selected = 2;
                }else{
                    selected = 0;
                }
                // force update on state variable (bypass joystick button press)
                state = g_fsm[state].next_state[selected];
                break;
            }
                
            case 4: // Not in use
            {
                break;
            }
                
            case 5: // WIN
            {
                selected = 0;
                angle++;
                winAnimation(angle); // draw game win animation frame
                resetRotation();
                GlobalGame.init(levelSelect(lvl_select)); // reset game object
                lvl_arr[lvl_select] = 0b00000011; // set level as a win
                
                if(lvl_select < 23){
                    lvl_arr[lvl_select+1] = 0b00000001; // unlock next level
                }
                
                break;
            }
                
            case 6: // LOSE
            {
                loseMenu.draw(selected); // draw lsoe menu
                resetRotation();
                GlobalGame.init(levelSelect(lvl_select)); // reset game object
                
                // if the level has not already been won
                if((lvl_arr[lvl_select] & 0b00000011) != 0b00000011)
                {
                    // set level as a loss
                    lvl_arr[lvl_select] = 0b00000010;
                }
                break;
            }
                
            case 7: // 3D objects
            {
                scale = scale*(1.0 + (g_buttonB - g_buttonD)*0.1);
                scale = scale<0.5 ? 0.5:scale;
                scale = scale>5.0 ? 5.0:scale;
                
                selected = 0;
                sel_object += g_buttonA - g_buttonC;
                if(sel_object < 0){
                    sel_object = 3;
                }
                else if(sel_object > 3){
                    sel_object = 0;
                }
                
                if (sel_object == 3)
                {
                    selected = 1;
                    state = g_fsm[state].next_state[selected];
                }
                else
                {
                    if(g_buttonA || g_buttonC){
                        switch(sel_object){
                            case 0:{
                                Obj.setObjData(suzanne_object);
                                break;
                            } case 1:{
                                
                                Obj.setObjData(torus_object);
                                break;
                            } case 2:{
                                Obj.setObjData(sphere_object);
                                break;
                            }
                        }
                    }    
                    drawObject(scale, Obj);
                    std::string names[] = {"Monkey", "Torus", "Sphere", ""};
                    lcd.drawRect(0, 0, names[sel_object].length()*6, 8, FILL_WHITE);
                    lcd.printString(names[sel_object].c_str(), 0, 0);
                    
                    lcd.drawRect(60, 0, 24, 8, FILL_WHITE);
                    lcd.printString("x", 61, 0);
                    char buffer[14];
                    sprintf(buffer, "%.01f", scale);
                    lcd.printString(buffer, 67, 0);
                }
                break;
            }
                
            case 8: 
            {
                lcd.drawSprite(controlButtons);
                break;
            }
                
            case 9:
            {
                lcd.drawSprite(controlJoystick);
                break;
            }
                
            default:
            {
                break;
            }
        }
        
        lcd.refresh();
        
    }

}




