/* 

    3D Engine
    
    This 3D engine draws an object onto the N5110 LCD
    
    Thomas A. Groom, University of Leeds, March 2021

*/


#include "N5110.h"
#include "Utils.h"
#include "Object.h"
#include "mbed.h"

Object::Object()
        :
         _faces_len()

{}

// initialise global variables

Vector3 g_face_normal;
volatile unsigned char fill_buffer[84][6];  // pixel buffer used to determine the inside area of faces
float depth_buffer[84][48] = {{-INFINITY}}; // pixel depth buffer


// interpolates the depth value for any x,y position on a face
// defined by a vertex and the vector normal to the face
float Object::interpolateDepth(int x, int y, Vector3 vert)
{   
    return vert.z - ((x-vert.x) * g_face_normal.x + (y-vert.y) * g_face_normal.y) / g_face_normal.z;
}


// sets a bit in the fill_buffer array
void Object::setFillBuffer(int x, int y, bool val)
{
    if(val){
        fill_buffer[x][y/8] = fill_buffer[x][y/8] | 0b10000000 >> y%8;
    }else{
        fill_buffer[x][y/8] = fill_buffer[x][y/8] & 0b01111111 >> y%8;
    }
}


// returns the value of a bit in the fill_buffer array
bool Object::getFillBuffer(int x, int y)
{
    return fill_buffer[x][y/8] & (0b10000000 >> y%8);
}


// fillByLine removes the lines drawn by drawLineArr in fill_buffer and
// compares the interpolated values of the z position of pixels within the current face.
// If a pixel in the new face is closer than the z value already stored in the depth_buffer,
// then the depth_buffer is updated and the new pixel colour is plotted
void Object::fillByLine(int left_x, int right_x, int top_y, int bottom_y, Vector3 vert)
{
    //if (_fill == FILL_WHITE)
    for(int x = left_x; x <= right_x; x++){
        unsigned int fill_test[2];
        fill_test[0] = (fill_buffer[x][0] << 24) | (fill_buffer[x][1] << 16) | (fill_buffer[x][2] << 8) | fill_buffer[x][3];
        fill_test[1] = (fill_buffer[x][4] << 24) | (fill_buffer[x][5] << 16) | (0x00 << 8) | 0x00;
        
        unsigned char first_bit = __clz(fill_test[0]) + ((__clz(fill_test[0])>>5) * __clz(fill_test[1]));
        unsigned char last_bit =  63 - (__clz(__rbit(fill_test[1])) + ((__clz(__rbit(fill_test[1]))>>5) * __clz(__rbit(fill_test[0]))));
        
        for (int y = first_bit; y <= last_bit; y++){
            
            float interpolatedZ = interpolateDepth(x,y,vert);
            if (depth_buffer[x][y] < interpolatedZ)
            {   
                if(_fill == FILL_WHITE)
                {
                    depth_buffer[x][y] = interpolatedZ;
                    lcd.setPixel(x, y, getFillBuffer(x, y));
                }
                else if(_fill == FILL_BLACK)
                {
                    depth_buffer[x][y] = interpolatedZ;
                    lcd.setPixel(x, y, !getFillBuffer(x, y));
                }
                else
                {
                    if(getFillBuffer(x, y) == 1){
                        lcd.setPixel(x, y, 1);
                    }
                }
            }
            setFillBuffer(x, y, 0);
            
        }
        //printf("\n");
    }

 
    // The following code is the older fill by line implementation where
    // each pixel needed to be checked
    /*
    for(int y = top_y; y <= bottom_y; y++){ 
        int end = -1;
        
        // start from left hand side and increment across the x axis
        for(int x = left_x; x <= right_x; x++){
            if(getFillBuffer(x,y) == 1)
            {// if there is an edge (i.e. fill_buffer set 1)
            
                setFillBuffer(x, y, 0); // clear the buffer at this pixel
                end = x; // update end position
                
                // update depth_buffer & draw the edge pixel accordingly
                float interpolatedZ = interpolateDepth(x,y,vert);
                if (depth_buffer[x][y] < interpolatedZ)
                {
                    depth_buffer[x][y] = interpolatedZ;
                    lcd.setPixel(x,y, 1);
                }

            }else if(end != -1){
                break;
            }
        }
        
        bool inside_face_flag = 0; // is 1 when inside of a face
        
        // start from right hand side and increment backwards across the x axis
        for(int x = right_x; x >= left_x; x--){
            if(x == end){
                break;
            }
            if(getFillBuffer(x,y) == 0)
            {
                if(inside_face_flag == 1){
                    // if within a face, update depth_buffer and set lcd pixels accordingly
                    float interpolatedZ = interpolateDepth(x,y,vert);
                    if (depth_buffer[x][y] < interpolatedZ)
                    {
                        depth_buffer[x][y] = interpolatedZ;
                        lcd.setPixel(x,y, 0);
                    }
                }
                
            }else{
                // i.e. when fill_beffer == 1
                inside_face_flag = 1;
                setFillBuffer(x, y, 0); // reset fill_buffer
                
                // update depth_buffer & draw the edge pixel accordingly
                float interpolatedZ = interpolateDepth(x,y,vert);
                if (depth_buffer[x][y] < interpolatedZ)
                {
                    depth_buffer[x][y] = interpolatedZ;
                    lcd.setPixel(x,y, 1);
                }
            }
        }
    }
    */
}


// draw a line between two points in the fill_buffer
// [modification of the drawLine function found in N5110.cpp]
void Object::drawLineArr(Vector3 start_point, Vector3 end_point)
{
    // only draw lines starting with the top and left most vertex to avoid drawing
    // the same edges in both directions as this can cause unintended thicker line widths
    int const x0 = max(start_point.x, end_point.x);
    int const y0 = max(start_point.x, end_point.x) == start_point.x ? start_point.y:end_point.y;
    int const x1 = min(start_point.x, end_point.x);
    int const y1 = min(start_point.x, end_point.x) == start_point.x ? start_point.y:end_point.y;
    
    int const y_range = y1 - y0;
    int const x_range = x1 - x0;
    
    unsigned int const step = 1;
    if (abs(x_range) > abs(y_range)) {
        
        // ensure we loop from smallest to largest or else for-loop won't run as expected
        short const start = x_range > 0 ? x0:x1;
        short const stop =  x_range > 0 ? x1:x0;

        // loop between x pixels
        for (int x = start; x <= stop; x += step) {
            // do linear interpolation
            int const dx = x - x0;
            int const y = y0 + y_range * dx / x_range;
            setFillBuffer(min(max(x,0),83), min(max(y,0),47), 1);
        }
    } else {
        short const start = y_range > 0 ? y0:y1;
        short const stop =  y_range > 0 ? y1:y0;

        for (int y = start; y <= stop; y += step) {
            int const dy = y - y0;
            int const x = x0 + x_range * dy / y_range;
            setFillBuffer(min(max(x,0),83), min(max(y,0),47), 1);
        }
    }
}


// drawQuad will draw a quadralateral face in the lcd buffer and also update
// the depth buffer
void Object::drawQuad(int faceIndex)
{
    // vector containing the four verticies of the face
    Vector3 quad_verts[4]; 
    
    for(int j = 0; j < 4; j++){
         // apply the transformations to each vertex
         
         quad_verts[j] = rotateVector(*(_verts_ptr + *(_faces_ptr + faceIndex * 4 + j)) + _relative_location, _rotation) * _scale + _location;  //verts[faces[faceIndex][j]], rotation) * scale + offset;  faces[faceIndex][j]
    }
    
    // calculate the normal vector
    g_face_normal = calculateNormal(quad_verts[0], quad_verts[1], quad_verts[2]);
    
    // backface culling, if the face is pointing towards the camera (pos z axis)
    if(g_face_normal.z < 0 || _fill == FILL_TRANSPARENT) 
    {
        // calculate the maximum and minimum X and Y vertex positions
        int leftX   = (int)(min(min(quad_verts[0].x, quad_verts[1].x), min(quad_verts[2].x, quad_verts[3].x)));
        int rightX  = (int)(max(max(quad_verts[0].x, quad_verts[1].x), max(quad_verts[2].x, quad_verts[3].x)));
        int topY    = (int)(min(min(quad_verts[0].y, quad_verts[1].y), min(quad_verts[2].y, quad_verts[3].y)));
        int bottomY = (int)(max(max(quad_verts[0].y, quad_verts[1].y), max(quad_verts[2].y, quad_verts[3].y)));
        
        if (not(rightX < 0 || leftX > 83 || topY > 47 || bottomY < 0))
        {   // if the face is not completely outside of the frame
            
            // constrain the maxima and minima to the screen space
            leftX = max(leftX,0);
            rightX = min(rightX,83);
            topY = max(topY,0);
            bottomY = min(bottomY,47);
            
            
            // draw lines in the fill_buffer
            drawLineArr(quad_verts[0], quad_verts[1]);
            drawLineArr(quad_verts[1], quad_verts[2]);
            drawLineArr(quad_verts[2], quad_verts[3]);
            drawLineArr(quad_verts[3], quad_verts[0]);
            
            // clear fill_buffer, update depth_buffer & set lcd pixel values
            fillByLine(leftX, rightX, topY, bottomY, quad_verts[0]);
        }
    }
}


void clearDepth()
{
    for(int y = 0; y < 48; y++ ){
        for(int x = 0; x < 84; x++){
            depth_buffer[x][y] = -INFINITY; // clear the depth buffer
        }
    }
}


void Object::setObjData(ObjData& obj)
{
    _verts_ptr = obj.verts_ptr;
    _faces_ptr = obj.faces_ptr;
    _faces_len = obj.faces_len;
}


void Object::setLocRotScale(Vector3 location, Vector3 relative_location, Quaternion rotation, Vector3 scale)
{
    _location = location;
    _relative_location = relative_location;
    _rotation = rotation;
    _scale = scale;
}


void Object::draw(FillType fill)
{
    _fill = fill;
    // for all faces
    for (int i = 0; i < _faces_len; i++) //sizeof(faces)/sizeof(faces[0]); i++)
    {
        drawQuad(i); // draw the quadralateral face
    }
        
}















