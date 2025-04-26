#include <iostream>
#include <vector>
#include "mbed.h"
#include "Menu.h"
#include <string>


// Returns the x location required to center align the input string
int centerAlign(std::string str)
{
    return (84-(str.length()*6))/2;
}


Menu::Menu()
    : 
    _name("Default"),
    _options()
{}


// Sets the title of the menu
void Menu::set_name(std::string name)
{
    _name = name;
}


// Sets the options of a menu
void Menu::set_options(std::string opt1, std::string opt2, std::string opt3)
{
    _options.push_back(opt1);
    _options.push_back(opt2);
    _options.push_back(opt3);
}


// Inverts all the pixels at the specified selected option
void Menu::drawSelected(int sel)
{
    for(int x = 14; x < 70; x++ ) {
        for(int y = 15 + (sel*8); y < 23 + (sel*8); y++ ) {
            if(lcd.getPixel(x, y) == 1) {
                lcd.setPixel(x, y, 0); 
            }else{
                lcd.setPixel(x, y, 1);
            }
        }
    }
}


// draws a rectangular box filled with white
void Menu::drawBox(int x0, int y0, int x1, int y1)
{
    lcd.drawRect(x0,y0,(x1-x0),(y1-y0),FILL_BLACK);
    lcd.drawRect(x0+1,y0+1,(x1-x0)-2,(y1-y0)-2,FILL_WHITE);
}


// Draws the menu inlcuding the title, option list and currently selected option
void Menu::draw(int selected)
{
    drawBox(10,5,74,14);
    drawBox(10,13,74,43);
    int textPos = 6;
    lcd.printString(_name.c_str(),centerAlign(_name),textPos);
    textPos += 9;
    lcd.printString(_options[0].c_str(),centerAlign(_options[0]),textPos);
    textPos += 8;
    lcd.printString(_options[1].c_str(),centerAlign(_options[1]),textPos);
    textPos += 8;
    lcd.printString(_options[2].c_str(),centerAlign(_options[2]),textPos);
    
    drawSelected(selected);
}

