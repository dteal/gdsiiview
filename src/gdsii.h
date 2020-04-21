#ifndef GDSII_H
#define GDSII_H

// GDSII file format parser
// very minimal; currently only extracts layer geometry
// written partly in ANSI C style for future possibilities

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

////////// GDSII CONSTANTS ////////////////////////////////////////////////////

const uint8_t RECORD_TYPE_HEADER = 0x00;
const uint8_t RECORD_TYPE_BGNLIB = 0x01;
const uint8_t RECORD_TYPE_LIBNAME = 0x02;
const uint8_t RECORD_TYPE_UNITS = 0x03;
const uint8_t RECORD_TYPE_ENDLIB = 0x04;
const uint8_t RECORD_TYPE_BGNSTR = 0x05;
const uint8_t RECORD_TYPE_STRNAME = 0x06;
const uint8_t RECORD_TYPE_ENDSTR = 0x07;
const uint8_t RECORD_TYPE_BOUNDARY = 0x08;
const uint8_t RECORD_TYPE_PATH = 0x09;
const uint8_t RECORD_TYPE_SREF = 0x0a;
const uint8_t RECORD_TYPE_AREF = 0x0b;
const uint8_t RECORD_TYPE_TEXT = 0x0c;
const uint8_t RECORD_TYPE_LAYER = 0x0d;
const uint8_t RECORD_TYPE_DATATYPE = 0x0e;
const uint8_t RECORD_TYPE_WIDTH = 0x0f;
const uint8_t RECORD_TYPE_XY = 0x10;
const uint8_t RECORD_TYPE_ENDEL = 0x11;
const uint8_t RECORD_TYPE_SNAME = 0x12;
const uint8_t RECORD_TYPE_COLROW = 0x13;
const uint8_t RECORD_TYPE_TEXTNODE = 0x14;
const uint8_t RECORD_TYPE_NODE = 0x15;
const uint8_t RECORD_TYPE_TEXTTYPE = 0x16;
const uint8_t RECORD_TYPE_PRESENTATION = 0x17;
const uint8_t RECORD_TYPE_STRING = 0x19;
const uint8_t RECORD_TYPE_STRANS = 0x1a;
const uint8_t RECORD_TYPE_MAG = 0x1b;
const uint8_t RECORD_TYPE_ANGLE = 0x1c;
const uint8_t RECORD_TYPE_REFLIBS = 0x1f;
const uint8_t RECORD_TYPE_FONTS = 0x20;
const uint8_t RECORD_TYPE_PATHTYPE = 0x21;
const uint8_t RECORD_TYPE_GENERATIONS = 0x22;
const uint8_t RECORD_TYPE_ATTRTABLE = 0x23;
const uint8_t RECORD_TYPE_ELFLAGS = 0x26;
const uint8_t RECORD_TYPE_NODETYPE = 0x2a;
const uint8_t RECORD_TYPE_PROPATTR = 0x2b;
const uint8_t RECORD_TYPE_PROPVALUE = 0x2c;
const uint8_t RECORD_TYPE_BOX = 0x2d;
const uint8_t RECORD_TYPE_BOXTYPE = 0x2e;
const uint8_t RECORD_TYPE_PLEX = 0x2f;
const uint8_t RECORD_TYPE_TAPENUM = 0x32;
const uint8_t RECORD_TYPE_TAPECODE = 0x33;
const uint8_t RECORD_TYPE_FORMAT = 0x36;
const uint8_t RECORD_TYPE_MASK = 0x37;
const uint8_t RECORD_TYPE_ENDMASKS = 0x38;

const uint8_t DATA_TYPE_NODATA = 0x00;
const uint8_t DATA_TYPE_BITARRAY = 0x01;
const uint8_t DATA_TYPE_INT16 = 0x02;
const uint8_t DATA_TYPE_INT32 = 0x03;
const uint8_t DATA_TYPE_REAL32 = 0x04;
const uint8_t DATA_TYPE_REAL64 = 0x05;
const uint8_t DATA_TYPE_ASCII = 0x06;

const uint8_t PATH_TYPE_FLUSH = 0x00;
const uint8_t PATH_TYPE_ROUND = 0x01;
const uint8_t PATH_TYPE_SQUARE = 0x02;

const uint8_t ELEMENT_TYPE_UNKNOWN = 0x00;
const uint8_t ELEMENT_TYPE_BOUNDARY = 0x01;
const uint8_t ELEMENT_TYPE_PATH = 0x02;
const uint8_t ELEMENT_TYPE_SREF = 0x03;
const uint8_t ELEMENT_TYPE_AREF = 0x04;
const uint8_t ELEMENT_TYPE_BOX = 0x05;

////////// DATA STRUCTURES ////////////////////////////////////////////////////

typedef float REAL32;       // note: GDSII has its own 32-bit real format!
typedef double REAL64;      // note: as before. This is for convenience.

struct GDSII_POINT{         // a 2D point
    REAL32 x, y;            // all data types should be converted to REAL32
    GDSII_POINT* next;      // (to implement linked list of points)
};

struct GDSII_ELEMENT{       // a boundary, path, reference, or box
    uint8_t type;           // type of element
    int16_t layer;          // GDSII layer number (0-255)
    int32_t width;          // width of path (negative means absolute)
    uint8_t path_type;      // type of path
    GDSII_ELEMENT* ref;     // referenced structure (for type SREF, AREF)
    GDSII_POINT* point;     // linked list of points in this element
    bool transform;         // whether transformations are applied
    bool reflect;           // whether to reflect about x-axis
    bool rotate;            // whether to rotate
    REAL64 angle;           // rotation angle (degrees)
    bool angle_is_absolute; // whether angle is absolute (vs. hierarchical)
    bool magnify;           // whether to magnify
    REAL64 magnification;   // magnification amount (1 = no effect)
    bool mag_is_absolute;   // whether magnification is absolute
    GDSII_ELEMENT* next;    // (to implement linked list of elements)
};

struct GDSII_STRUCTURE{     // a collection of elements
    char* name;             // structure name (ASCII, dynamically allocated)
    GDSII_ELEMENT* element; // linked list of elements in this structure
    GDSII_STRUCTURE* next;  // (to implement linked list of structures)
};

struct GDSII{               // a complete GDSII file
    GDSII_STRUCTURE* structure; // linked list of structures
};

////////// DATA STRUCTURE HANDLERS ////////////////////////////////////////////

GDSII_POINT* gdsii_create_point(){
    GDSII_POINT* point;
    point = (GDSII_POINT*)malloc(sizeof(GDSII_POINT));
    (*point).x = 0;
    (*point).y = 0;
    (*point).next = NULL;
    return point;
}

void gdsii_delete_point(GDSII_POINT* point){
    while(point != NULL){
        GDSII_POINT* next = (*point).next;
        free(point);
        point = next;
    }
}

GDSII_ELEMENT* gdsii_create_element(){
    GDSII_ELEMENT* element;
    element = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
    (*element).type = ELEMENT_TYPE_UNKNOWN;
    (*element).layer = 0;
    (*element).width = 0;
    (*element).path_type = 0;
    (*element).ref = NULL;
    (*element).point = NULL;
    (*element).transform = false;
    (*element).reflect = false;
    (*element).rotate = false;
    (*element).angle = 0;
    (*element).angle_is_absolute = false;
    (*element).magnify = false;
    (*element).magnification = 1;
    (*element).mag_is_absolute = false;
    (*element).next = NULL;
    return element;
}

void gdsii_delete_element(GDSII_ELEMENT* element){
    while(element != NULL){
        gdsii_delete_element((*element).ref);
        gdsii_delete_point((*element).point);
        GDSII_ELEMENT* next = (*element).next;
        free(element);
        element = next;
    }
}

GDSII_STRUCTURE* gdsii_create_structure(){
    GDSII_STRUCTURE* structure;
    structure = (GDSII_STRUCTURE*)malloc(sizeof(GDSII_STRUCTURE));
    (*structure).name = NULL;
    (*structure).element = NULL;
    (*structure).next = NULL;
    return structure;
}

void gdsii_delete_structure(GDSII_STRUCTURE* structure){
    while(structure != NULL){
        gdsii_delete_element((*structure).element);
        if((*structure).name != NULL){
            free((*structure).name);
        }
        GDSII_STRUCTURE* next = (*structure).next;
        free(structure);
        structure = next;
    }
}

GDSII* gdsii_create_gdsii(){
    GDSII* gdsii;
    gdsii = (GDSII*)malloc(sizeof(GDSII));
    (*gdsii).structure = NULL;
    return gdsii;
}

void gdsii_delete_gdsii(GDSII* gdsii){
    gdsii_delete_structure((*gdsii).structure);
    free(gdsii);
}

////////// DATA PARSING ///////////////////////////////////////////////////////

std::vector<int16_t> gdsii_parse_int16(uint8_t* data, uint16_t length){
    assert(length%2==0);
    std::vector<int16_t> numbers;
    for(unsigned int i=0; i<length/2; i++){
        numbers.push_back((data[2*i+0] << 8) + data[2*i+1]);
    }
    return numbers;
}

std::vector<int32_t> gdsii_parse_int32(uint8_t* data, uint16_t length){
    assert(length%4==0);
    std::vector<int32_t> numbers;
    for(unsigned int i=0; i<length/4; i++){
        int32_t num = (data[4*i+0] << 24) + (data[4*i+1] << 16) + (data[4*i+2] << 8) + data[4*i+3];
        numbers.push_back(num);
    }
    return numbers;
}

/*
 * NOT IEEE754!
std::vector<REAL32> parse_real32(uint8_t* data, uint16_t length){
}
std::vector<REAL64> parse_real64(uint8_t* data, uint16_t length){
    assert(length%8==0);
    std::vector<REAL64> newdata;
    for(uint i=0; i<length/8; i++){
        int sign = 1;
        if(data[0]&0x80) sign=-1;
        // TODO: finish this
        //newdata.push_back((data[4*i] << 3) + (data[4*i+1] << 2) + (data[4*i+2] << 1) + data[4*i+3]);
    }
    return newdata;
}
*/

char* gdsii_parse_string(uint8_t* data, uint16_t length){
    // data is not necessarily null-terminated
    char* text = NULL;
    text = (char*)malloc(sizeof(char)*(length+1));
    for(unsigned int i=0; i<length; i++){
        text[i] = data[i];
    }
    text[length] = 0x00;
    return text;
}

////////// DATA PARSING ///////////////////////////////////////////////////////

bool gdsii_read(GDSII* gdsii, const char* filepath){

    FILE* file = fopen(filepath, "r");
    if(file == NULL){ perror("Error! Could not read GDS file."); return false; }

    // end of structure linked list
    GDSII_STRUCTURE** structure = &((*gdsii).structure);
    GDSII_ELEMENT** element = NULL;

    while(true){

        // read record header
        uint8_t buffer[4]; // header is always 32 bits long
        size_t read = fread(buffer, sizeof(uint8_t), 4, file);
        if(read==0){ break; } // reached EOF

        // create new record
        uint16_t length = (buffer[0] << 8) + buffer[1] - 4; // length of data, not entire record
        uint8_t record_type = buffer[2];
        uint8_t data_type = buffer[3];
        //std::cout << "RECORD: length: " << (int)(length+4) << " type: " << (int)record_type << " data: " << (int)data_type << std::endl;

        // read record data
        uint8_t* data = nullptr;
        if(length > 0){
            data = new uint8_t[length];
            fread(data, sizeof(uint8_t), length, file);
        }

        switch(record_type){
            case RECORD_TYPE_UNITS:
                //printf("UNITS\n");
                //std::cout << (int)data_type << std::endl;
                break;
            case RECORD_TYPE_BGNSTR: // create new structure
                //printf("STRUCTURE\n");
                (*structure) = gdsii_create_structure();
                element = &((**structure).element);
                break;
            case RECORD_TYPE_ENDSTR: // move marker to new end of linked list
                //printf("ENDSTRUCT\n");
                structure = &((**structure).next);
                break;
            case RECORD_TYPE_STRNAME: // assume data is null-terminated
                (**structure).name = gdsii_parse_string(data, length);
                //std::cout << (**structure).name << std::endl;
                break;
            case RECORD_TYPE_ENDEL: // end element
                //printf("\tENDEL\n");
                element = &((**element).next);
                break;
            case RECORD_TYPE_BOUNDARY:
                //printf("\tBOUNDARY\n");
                (*element) = gdsii_create_element();
                (**element).type = ELEMENT_TYPE_BOUNDARY;
                break;
            case RECORD_TYPE_PATH:
                //printf("\tPATH\n");
                (*element) = gdsii_create_element();
                (**element).type = ELEMENT_TYPE_PATH;
                break;
            case RECORD_TYPE_SREF:
                //printf("\tSREF\n");
                (*element) = gdsii_create_element();
                (**element).type = ELEMENT_TYPE_SREF;
                break;
            case RECORD_TYPE_AREF:
                //printf("\tAREF\n");
                (*element) = gdsii_create_element();
                (**element).type = ELEMENT_TYPE_AREF;
                break;
            case RECORD_TYPE_BOX:
                //printf("\tBOX\n");
                (*element) = gdsii_create_element();
                (**element).type = ELEMENT_TYPE_BOX;
                break;
            case RECORD_TYPE_XY:
                //printf("\t\tXY\n");
                if(data_type == DATA_TYPE_INT32){
                    GDSII_POINT** point = &((**element).point);
                    std::vector<int32_t>coordinates = gdsii_parse_int32(data, length);
                    assert(coordinates.size()%2==0);
                    for(unsigned int i=0; i<coordinates.size()/2; i++){
                        (*point) = gdsii_create_point();
                        (**point).x = (REAL64) coordinates[i*2+0];
                        (**point).y = (REAL64) coordinates[i*2+1];
                        point = &((**point).next);
                    }
                }
                break;
            case RECORD_TYPE_LAYER:
                //printf("\tLAYER");
                if(data_type == DATA_TYPE_INT16){
                    std::vector<int16_t>points = gdsii_parse_int16(data, length);
                    (**element).layer = points[0];
                }
                break;
        }

        delete[] data;
    }

    fclose(file);

    return true;
}

#endif