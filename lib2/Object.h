
#ifndef OBJECT_H
#define OBJECT_H

#include "N5110.h"
#include "Utils.h"


void clearDepth();


class Object
{
public:
    Object();
    void draw(FillType fill);
    void setObjData(ObjData& obj);
    void setLocRotScale(Vector3 location, Vector3 relative_location, Quaternion rotation, Vector3 scale);

private:
    float interpolateDepth(int x, int y, Vector3 vert);
    void setFillBuffer(int x, int y, bool val);
    bool getFillBuffer(int x, int y);
    void fillByLine(int left_x, int right_x, int top_y, int bottom_y, Vector3 vert);
    void drawLineArr(Vector3 start_point, Vector3 end_point);
    void drawQuad(int faceIndex);
    
    const Vector3 *_verts_ptr;
    const unsigned short *_faces_ptr;
    unsigned short _faces_len;
    
    Vector3 _location;
    Vector3 _relative_location;
    Quaternion _rotation;
    Vector3 _scale;
    FillType _fill;
};

#endif



