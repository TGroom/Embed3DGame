#ifndef Menu_H
#define Menu_H
#include "mbed.h"
#include "N5110.h"
#include <string>
#include <vector>


int centerAlign(std::string str);

class Menu
{
public:
    Menu();
    void draw(int selected);
    void set_name(std::string name);
    void set_options(std::string opt1, std::string opt2, std::string opt3);

private:
    void drawBox(int x0, int y0, int x1, int y1);
    void drawSelected(int sel);
    std::string _name;
    std::vector<std::string> _options;
    
};

#endif



