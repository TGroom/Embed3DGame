
#ifndef GAME_H
#define GAME_H

#include <vector>

#include "N5110.h"
#include "Utils.h"
#include "Object.h"
//using std::vector;


class Game
{
public:
    Game();
    void init(vector<Tile> tile_list);
    int getFrame();
    
    void setNewRotation(Quaternion rotation);
    Quaternion getNewRotation();
    
    void setButtonAflag(bool buttonA_flag);
    void setButtonBflag(bool buttonB_flag);
    void setButtonCflag(bool buttonC_flag);
    void setButtonDflag(bool buttonD_flag);
    void setButtonJflag(bool buttonJ_flag);

private:
    bool getSpaceBit(int input_space, int i, int j, int k);
    int translateSpace(int input_space, int i, int j, int k);
    Tile rotateSpace(Tile input_tile, int i, int j, int k);
    vector<int> validTranslations(Tile input_tile);
    vector<Tile> validRotations(Tile input_tile);
    void print(double val, int row);
    void drawTileList();
    
    vector<Tile> _tile_list;
    double _elapsed_secs;
    double _display_secs;
    volatile int _counter;
    
    Vector3 _location;
    Quaternion _rotation;
    Vector3 _scale;
    Quaternion _new_rotation;
    
    //Tile _test_tile;
    
    vector<Tile> _valid_rotations;
    vector<int> _valid_loc;
    
    short _tile_count;
    
    short _loc_index;
    short _rot_index;
    
    bool _buttonA_flag;
    bool _buttonB_flag;
    bool _buttonC_flag;
    bool _buttonD_flag;
    bool _buttonJ_flag;
};

#endif



