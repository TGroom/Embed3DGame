
#include "Utils.h"
#include "mbed.h"
#include <vector>

using namespace std; 


// Converts z, y and x rotation (roll, pitch and yaw) into a Quaternion
// Source: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
Quaternion ToQuaternion(double yaw, double pitch, double roll)
{
    // Abbreviations for the various angular functions
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);

    Quaternion q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

// Returns a rotated point given an initial point position and Quaternion
// Source: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
Vector3 rotateVector(const Vector3& v, const Quaternion& q)
{
    Vector3 u;
    u.x = q.x;
    u.y = q.y;
    u.z = q.z;
    float s = q.w;

    Vector3 out = 2.0f * dot(u, v) * u
                  + (s*s - dot(u, u)) * v
                  + 2.0f * s * cross(u, v);
    return out;
}


// Performs the dot product between two vectors
float dot(Vector3 vector_a, Vector3 vector_b) {
   float product = 0;
   product += vector_a.x * vector_b.x;
   product += vector_a.y * vector_b.y;
   product += vector_a.z * vector_b.z;
   return product;
}


// Performs the cross product between two vectors
Vector3 cross( Vector3 a,  Vector3 b)
{
    Vector3 Product;
    Product.x = (a.y * b.z) - (a.z * b.y);
    Product.y = (a.z * b.x) - (a.x * b.z);
    Product.z = (a.x * b.y) - (a.y * b.x);
    return Product;
}


// gets the magnitude of a 3D vector
float magnitude(Vector3 vect)
{
    return sqrt(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
}


// This function calulcates the normal vector of a face given 3 of its verticies
// the code is based on http://www.cplusplus.com/forum/general/77959/
Vector3 calculateNormal(Vector3 p1, Vector3 p2, Vector3 p3)
{
    Vector3 vect, vect2;
    vect = p3 - p1;
    vect2 = p2 - p1;
    Vector3 normal = cross(vect,vect2);

    // normalise
    double length = magnitude(normal);
    normal.x = normal.x/length;
    normal.y = normal.y/length;
    normal.z = normal.z/length;
    
    return normal;
}


// Level definitions
vector<Tile> levelSelect(int sel){
    vector<Tile> lvl_tile_list;
    Tile O_tile =  {0x4000000, {1,1,1}};
    Tile i_tile =  {0x6000000, {2,1,1}};
    Tile l_tile =  {0x6800000, {2,2,1}};
    Tile I_tile =  {0x7000000, {3,1,1}};
    Tile L_tile =  {0x7200000, {3,2,1}};
    Tile T_tile =  {0x7400000, {3,2,1}};
    Tile C_tile =  {0x7A00000, {3,2,1}};
    Tile Z_tile =  {0x6600000, {3,2,1}};
    Tile X_tile =  {0x6820000, {2,2,2}};
    lvl_tile_list.clear();
    switch(sel){
        case 0:
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(L_tile);
            lvl_tile_list.push_back(O_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(i_tile);
            lvl_tile_list.push_back(i_tile);
            lvl_tile_list.push_back(i_tile);
            lvl_tile_list.push_back(O_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(i_tile);
            break;
        case 1:
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            break;
        case 2:
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(L_tile);
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(l_tile);
            break;
        case 3:
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(L_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(i_tile);
            break;
        case 4:
            lvl_tile_list.push_back(Z_tile);
            lvl_tile_list.push_back(Z_tile);
            lvl_tile_list.push_back(Z_tile);
            lvl_tile_list.push_back(Z_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(I_tile);
            break;
        case 5:
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(O_tile);
            lvl_tile_list.push_back(O_tile);
            break;
        case 6:
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(l_tile);
            lvl_tile_list.push_back(I_tile);
            lvl_tile_list.push_back(O_tile);
            break;
        case 7:
            lvl_tile_list.push_back(Z_tile);
            lvl_tile_list.push_back(T_tile);
            lvl_tile_list.push_back(L_tile);
            lvl_tile_list.push_back(C_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(X_tile);
            lvl_tile_list.push_back(i_tile);
            break;
        case 8:
            break;
        case 9:
            break;
        case 10:
            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            break;
        case 14:
            break;
        case 15:
            break;
        case 16:
            break;
        case 17:
            break;
        case 18:
            break;
        case 19:
            break;
        case 20:
            break;
        case 21:
            break;
        case 22:
            break;
        case 23:
            break;
        case 24: 
            break;
            
    }
    
    return lvl_tile_list;
}