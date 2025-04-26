/* 
    Game Engine
    This game engine calculates and then draws the game state onto the N5110 LCD
    Thomas A. Groom, University of Leeds, April 2021
*/

#include <vector>

#include "N5110.h"
#include "Utils.h"
#include "Game.h"
#include "Object.h"

Game::Game()
        :
        _tile_list(),
        _elapsed_secs(),
        _display_secs(),
        _counter(),
        
        _location(),
        _rotation(),
        _scale(),
        
        _loc_index(),
        _rot_index()
{}


const Vector3 verts[8] = {{-1.0, -1.0, -1.0}, {-1.0, -1.0, 1.0}, {-1.0, 1.0, -1.0}, {-1.0, 1.0, 1.0}, {1.0, -1.0, -1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, -1.0}, {1.0, 1.0, 1.0}};
const unsigned short faces[6][4] = {{0, 1, 3, 2}, {2, 3, 7, 6}, {6, 7, 5, 4}, {4, 5, 1, 0}, {2, 6, 4, 0}, {7, 3, 1, 5}};

ObjData cube_object = {verts, faces[0], 6};

const Vector3 verts3[20] = {{-1.0, 1.0, 1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0}, {1.0, 1.0, -1.0}, {1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {-1.0, 1.0, 0.3333333333333333}, {-1.0, 1.0, -0.3333333333333333}, {1.0, 1.0, -0.3333333333333333}, {1.0, 1.0, 0.3333333333333333}, {-0.3333333333333333, 1.0, 1.0}, {0.3333333333333333, 1.0, 1.0}, {0.3333333333333333, 1.0, -1.0}, {-0.3333333333333333, 1.0, -1.0}, {0.3333333333333333, 1.0, 0.3333333333333333}, {-0.3333333333333333, 1.0, 0.3333333333333333}, {0.3333333134651184, 1.0, -0.3333330750465393}, {-0.33333325386047363, 1.0, -0.3333330750465393}};
const unsigned short faces3[1][4] = {{0, 1, 5, 4}};

ObjData game_space_object = {verts3, faces3[0], 1};

// initialise global variables
unsigned int game_space = 0;

Object Cube;
Object GameSpace;


void Game::setButtonAflag(bool buttonA_flag){
    _buttonA_flag = buttonA_flag;
}
void Game::setButtonBflag(bool buttonB_flag){
    _buttonB_flag = buttonB_flag;
}
void Game::setButtonCflag(bool buttonC_flag){
    _buttonC_flag = buttonC_flag;
}
void Game::setButtonDflag(bool buttonD_flag){
    _buttonD_flag = buttonD_flag;
}
void Game::setButtonJflag(bool buttonJ_flag){
    _buttonJ_flag = buttonJ_flag;
}


// displays a double floating point number to the LCD display (to display fps)
void Game::print(double val, int row)
{
    char buffer[14];
    sprintf(buffer, "%.0lf", val);
    lcd.printString(buffer, 0, row);
}


// returns the bit at location i, j, k for an encoded game space integer
// the encoded int is a 1D representation of the 27 discrete  locations in a 3x3x3 cube
bool Game::getSpaceBit(int input_space, int i, int j, int k)
{
    return (bool)(input_space & (((0x7FC0000 >> i*9) & (0x70381C0 >> j*3)) & (0x4924924 >> k)));
}


// this function will translate a 3x3x3 game space by i, j, k
int Game::translateSpace(int input_space, int i, int j, int k)
{
    int output_space = input_space;
    for(int n = 0; n < i; n++){
        // translate by 1 bit in x direction
        output_space = (output_space & 0x6DB6DB6) >> 1;
    }
    for(int n = 0; n < j; n++){
        // translate by 1 bit in y direction
        output_space = (output_space & 0x7E3F1F8) >> 3;
    }
    for(int n = 0; n < k; n++){
        // translate by 1 bit in z direction
        output_space = output_space >> 9;
    }
    return output_space;
}


// this function rotates a tile by i, j and k
// the function rotates both the encoded int representation of the shape of the
// tile, AND the size of the tile
Tile Game::rotateSpace(Tile input_tile, int i, int j, int k)
{
    Tile output = input_tile;
    int temp = 0; // variable to store intermediate rotations
    
    // rotation in  about the x axis
    for(int n = 0; n < i; n++){
        int temp0 = output.size[0];
        output.size[0] = output.size[1];
        output.size[1] = temp0;
        
        for(int b = 0; b < 27; b++){
            // the formula below rearranges bit at index b to index b'
            // corrisponding to a rotation
            temp |= ((output.shape & (0x4000000 >> b)) << b) 
                    >> ((2-((b%9)/3)) + ((b%3)*3) + (9*(b/9)));
        }
        output.shape = temp;
        temp = 0;
    }
    
    // rotation in  about the y axis
    for(int n = 0; n < j; n++){
        int temp1 = output.size[1];
        output.size[1] = output.size[2];
        output.size[2] = temp1;
        
        for(int b = 0; b < 27; b++){
            // bit-wise rotation formula for y axis
            temp |= ((output.shape & (0x4000000 >> b)) << b) 
                    >> b + (6*((-2*(b/9)) + ((b%9)/3)+1));
        }
        output.shape = temp;
        temp = 0;
    }
    
    // rotation in  about the z axis
    for(int n = 0; n < k; n++){
        int temp2 = output.size[2];
        output.size[2] = output.size[0];
        output.size[0] = temp2;
        
        for(int b = 0; b < 27; b++){
            // bit-wise rotation formula for z axis
            temp |= ((output.shape & (0x4000000 >> b)) << b) 
                    >> b + ((9*(2-(b/9) - (b%3))) + (b/9) - (b%3));
        }
        output.shape = temp;
        temp = 0;
    }
    return output;
}


// returns all possible positions for the tile piece in a variable length vector
// for any given tile and the current game space
vector<int> Game::validTranslations(Tile input_tile)
{
    unsigned int tile_game_space = 0;
    int attempt_loc[3] = {0};
    vector<int> pos_list;
    
    // for all possible locations given the size constraint
    for(int i = 0; i < (4-input_tile.size[1])*(4-input_tile.size[2]); i++){
        
        // get the location of the current attempt
        int operand = 4 - input_tile.size[1];
        attempt_loc[0] = 4 - input_tile.size[0];
        attempt_loc[1] = (int)(i%operand);
        attempt_loc[2] = (int)(i/operand);
        
        int collision_count = 1;
        while(collision_count != 0 && attempt_loc[0] > 0){
            attempt_loc[0] -= 1;
            
            // set tile_game_space to have tile in it at relative location
            tile_game_space = translateSpace(input_tile.shape, attempt_loc[0], attempt_loc[1], attempt_loc[2]);
            collision_count = tile_game_space & game_space; // detect collisions
        }
        if(collision_count == 0){
            int n = translateSpace(input_tile.shape, attempt_loc[0], attempt_loc[1], attempt_loc[2]);
            pos_list.push_back(n);
        }
    }
    return pos_list;
}


// Returns a list of all valid tile rotations for a given tile in the current game space
vector<Tile> Game::validRotations(Tile input_tile)
{
    vector<Tile> rot_list;
    
    const int all_rot[24][3] =  {{0,0,0},{0,0,1},{0,0,2},{0,0,3},
                           {1,0,0},{1,0,1},{1,0,2},{1,0,3},
                           {2,0,0},{2,0,1},{2,0,2},{2,0,3},
                           {3,0,0},{3,0,1},{3,0,2},{3,0,3},
                           {0,1,0},{0,1,1},{0,1,2},{0,1,3},
                           {0,3,0},{0,3,1},{0,3,2},{0,3,3}};
    
    for(int i = 0; i < 24; i++){
        
        // [NOTE 1] replacing this code with a check of x row, y row and z row
        // for any 2 or more coincidenceshence allows the removal of the 'size'
        // from the Tile struct
        
        Tile convex_hull;
        convex_hull.shape = 0x4000000;
        
        // create convex hull shape (minimum bounding box)
        for(int b = 0; b < input_tile.size[0]-1; b++){
            convex_hull.shape |= convex_hull.shape >> 1;
        }
        for(int b = 0; b < input_tile.size[1]-1; b++){
            convex_hull.shape |= convex_hull.shape >> 3;
        }
        for(int b = 0; b < input_tile.size[2]-1; b++){
            convex_hull.shape |= convex_hull.shape >> 9;
        }
        
        Tile valid_tile = rotateSpace(input_tile, all_rot[i][0], all_rot[i][1], all_rot[i][2]);
        convex_hull = rotateSpace(convex_hull, all_rot[i][0], all_rot[i][1], all_rot[i][2]);
        valid_tile.shape = valid_tile.shape << (__clz(convex_hull.shape)-5); // Move to 0,0,0 corner
        
        // check if there are valid locations for the tile
        if(validTranslations(valid_tile).size() > 0){
            // remove identical rotations cause by the symmetry of the tile shape
            bool duplicate_check = 0;
            for(int n = 0; n < rot_list.size(); n++){
                if(rot_list[n].shape == valid_tile.shape){
                    duplicate_check = 1;
                }
            }
            if(duplicate_check == 0){
                rot_list.push_back(valid_tile);
            }
        }
    }
    return rot_list;
}


// updates the rotation of the view based on the joystick inputs as a quaternion
void Game::setNewRotation(Quaternion rotation)
{
    _rotation = rotation;
}


// initialises all variable to their default values at the beginning of a game
void Game::init(vector<Tile> tile_list)
{
    _tile_list.clear();
    _tile_list = tile_list;
    
    // variables used to calculate fps
    _elapsed_secs = 0;
    _display_secs = 0;
    _counter = 0;
    
    // transformation variables for scale, position & rotation
    _location.x = 42;
    _location.y = 24;
    _location.z = 0;
    
    _rotation.x = 0;
    _rotation.y = 0;
    _rotation.z = 0;
    _rotation.w = 1;
    
    _scale.x = 35;
    _scale.y = 35;
    _scale.z = 35;
    
    Cube.setObjData(cube_object);
    GameSpace.setObjData(game_space_object);

    _valid_rotations = validRotations(_tile_list[0]);
    _valid_loc = validTranslations(_tile_list[0]);
    
    _tile_count = 0;
    _loc_index = 0;
    _rot_index = 0;
    game_space = 0;
}


// Draws the tile list on the right of the screen
void Game::drawTileList()
{
    lcd.drawRect(71,5,12,38,FILL_TRANSPARENT);
    
    for(int index = 0; index < min((int)(_tile_list.size()-_tile_count-1), 4); index++){
        Tile tile2draw = _tile_list[index+_tile_count+1];
        for(int n = 0; n < 9; n++){
            if(getSpaceBit(tile2draw.shape, 0, n/3, n%3)){
                lcd.drawRect((n/3)*2+74+(3-tile2draw.size[1]),
                            (n%3)*2+(index*8)+9+(3-tile2draw.size[0]),
                            2,2,FILL_BLACK);
            }
        }
    } 
}


int Game::getFrame()
{
    clock_t begin = clock(); // begin the timer
    _counter++; // increment frame counter
    int status = 0;
        
    if (_buttonA_flag == 1){
        _buttonA_flag = 0;
        _loc_index += 1;
    }
    if (_buttonB_flag == 1){
        _rot_index += 1;
    }
    if (_buttonC_flag == 1){
        _buttonC_flag = 0;
        _loc_index -= 1;
    }
    if (_buttonD_flag == 1){
        _rot_index -= 1;
    }

    // if a new rotation is requested
    if(_buttonB_flag == 1 || _buttonD_flag == 1){
        _valid_loc = validTranslations(_valid_rotations[_rot_index % _valid_rotations.size()]);
        _buttonB_flag = 0;
        _buttonD_flag = 0;
    }
    
    _rot_index = _rot_index >= 0 ? _rot_index:_valid_rotations.size()-1;
    _loc_index = _loc_index >= 0 ? _loc_index:_valid_loc.size()-1;
    
    unsigned int tile_game_space = 0;
    if(_valid_rotations.size() > 0){
        tile_game_space = _valid_loc[_loc_index % _valid_loc.size()];
    }else if(game_space == 0x7FFFFFF){
        status = WIN;
    }else{
        status = LOSE;
    }
    
    if (_buttonJ_flag == 1)
    {
        game_space |= tile_game_space;
        //_rot_index = 0;
        //_loc_index = 0;
        if(_tile_count < _tile_list.size()-1){
            _tile_count++;
        }
        _valid_rotations = validRotations(_tile_list[_tile_count]);
        _valid_loc = validTranslations(_valid_rotations[_rot_index % _valid_rotations.size()]);
        _buttonJ_flag = 0;
    }
    
    lcd.clear();  // clear the lcd
    clearDepth(); // clear the depth buffer
    
    // draws the fixed game space and current tile on the lcd using the 3D engine
    for(int i = 0; i <= 2; i++){
        for(int j = 0; j <= 2; j++){
            for(int k = 0; k <= 2; k++){
                Vector3 relative_loc = {(-i+1)*2, (k-1)*2, (j-1)*2};
                
                if(getSpaceBit(game_space, 2-i, j, k)  == 1)
                {
                    // if the game space if filled (== 1) then draw a white cube
                    Cube.setLocRotScale(_location, relative_loc, _rotation, _scale/6.0);
                    Cube.draw(FILL_WHITE);
                }
                else if(getSpaceBit(tile_game_space, 2-i, j, k) == 1)
                {
                    // if the tile space if filled (== 1) then draw a black cube
                    Cube.setLocRotScale(_location, relative_loc, _rotation, _scale/6.0);
                    Cube.draw(FILL_BLACK);
                }
            }
        }
    }
    
    Vector3 relative_loc = {0,0,0};
    Vector3 scale_factor = {1.90, 1.90, 2.0};
    GameSpace.setLocRotScale(_location, relative_loc, _rotation, _scale/scale_factor);
    GameSpace.draw(FILL_WHITE);
    
    // calculate the elapsed seconds
    clock_t end = clock();
    _elapsed_secs += double(end - begin) / CLOCKS_PER_SEC;
    
    if(_counter % 40 == 0)
    {
        _display_secs = _elapsed_secs / 40; // average fps over 40 frames
        _elapsed_secs = 0;
    }
    
    print(1.0 / _display_secs,0); // display the fps
    drawTileList();
    lcd.refresh();
    return status;
}
