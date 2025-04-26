#ifndef UTILS_H
#define UTILS_H
#define PI 3.141596

#define WIN 2
#define LOSE 1
#include <vector>
using namespace std;

struct Tile{
    int shape;
    char size[3];
};

struct Vector3{
    float x;
    float y;
    float z;
    
    friend Vector3 operator+(const Vector3& a, const Vector3& b) {
        Vector3 out;
        out.x = a.x + b.x;
        out.y = a.y + b.y;
        out.z = a.z + b.z;
        return out;
    }
    
    friend Vector3 operator+(const Vector3& a, float b) {
        Vector3 out;
        out.x = a.x + b;
        out.y = a.y + b;
        out.z = a.z + b;
        return out;
    }
    
    friend Vector3 operator-(const Vector3& a, const Vector3& b) {
        Vector3 out;
        out.x = a.x - b.x;
        out.y = a.y - b.y;
        out.z = a.z - b.z;
        return out;
    }
    
    friend Vector3 operator*(const Vector3& a, const Vector3& b) {
        Vector3 out;
        out.x = a.x * b.x;
        out.y = a.y * b.y;
        out.z = a.z * b.z;
        return out;
    }
    
    friend Vector3 operator*(float b, const Vector3& a) {
        Vector3 out;
        out.x = a.x * b;
        out.y = a.y * b;
        out.z = a.z * b;
        return out;
    }
    
    friend Vector3 operator/(const Vector3& a, const Vector3& b) {
        Vector3 out;
        out.x = a.x / b.x;
        out.y = a.y / b.y;
        out.z = a.z / b.z;
        return out;
    }
    
    friend Vector3 operator/(const Vector3& a, const float b) {
        Vector3 out;
        out.x = a.x / b;
        out.y = a.y / b;
        out.z = a.z / b;
        return out;
    }
    
};


struct Obj{
    Vector3* verts;
    unsigned short* faces;
    unsigned short faces_len;
};


struct Quaternion {
    // used to define quaternions
    float x;
    float y;  
    float z;  
    float w; 
    
    // Defines the multiplication operation of Quaternions
    // Source: https://stackoverflow.com/questions/19956555/how-to-multiply-two-quaternions
    friend Quaternion operator*(const Quaternion& a, const Quaternion& b) {
        
        Quaternion Q;
        Q.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;  // i
        Q.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;  // j
        Q.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;  // k
        Q.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
        
        return Q;
    }
};


struct ObjData{
    const Vector3 *verts_ptr;
    const unsigned short *faces_ptr;
    const unsigned short faces_len;
};


Quaternion ToQuaternion(double yaw, double pitch, double roll);
Vector3 rotateVector(const Vector3& v, const Quaternion& q);

float dot(Vector3 vector_a, Vector3 vector_b);
Vector3 cross(Vector3 a, Vector3 b);
float magnitude(Vector3 point);
Vector3 calculateNormal(Vector3 p1, Vector3 p2, Vector3 p3);

vector<Tile> levelSelect(int sel);

#endif
