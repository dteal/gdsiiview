#ifndef GDSII_H
#define GDSII_H

// GDSII file format parser
// very minimal; extracts layer geometry only

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// just in case
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef float REAL32;
typedef double REAL64;

// possibly non-exaustive list of record types
const UINT8 RECORD_TYPE_HEADER = 0x00;
const UINT8 RECORD_TYPE_BGNLIB = 0x01;
const UINT8 RECORD_TYPE_LIBNAME = 0x02;
const UINT8 RECORD_TYPE_UNITS = 0x03;
const UINT8 RECORD_TYPE_ENDLIB = 0x04;
const UINT8 RECORD_TYPE_BGNSTR = 0x05;
const UINT8 RECORD_TYPE_STRNAME = 0x06;
const UINT8 RECORD_TYPE_ENDSTR = 0x07;
const UINT8 RECORD_TYPE_BOUNDARY = 0x08;
const UINT8 RECORD_TYPE_PATH = 0x09;
const UINT8 RECORD_TYPE_SREF = 0x0a;
const UINT8 RECORD_TYPE_AREF = 0x0b;
const UINT8 RECORD_TYPE_TEXT = 0x0c;
const UINT8 RECORD_TYPE_LAYER = 0x0d;
const UINT8 RECORD_TYPE_DATATYPE = 0x0e;
const UINT8 RECORD_TYPE_WIDTH = 0x0f;
const UINT8 RECORD_TYPE_XY = 0x10;
const UINT8 RECORD_TYPE_ENDEL = 0x11;
const UINT8 RECORD_TYPE_SNAME = 0x12;
const UINT8 RECORD_TYPE_COLROW = 0x13;
const UINT8 RECORD_TYPE_TEXTNODE = 0x14;
const UINT8 RECORD_TYPE_NODE = 0x15;
const UINT8 RECORD_TYPE_TEXTTYPE = 0x16;
const UINT8 RECORD_TYPE_PRESENTATION = 0x17;
const UINT8 RECORD_TYPE_STRING = 0x19;
const UINT8 RECORD_TYPE_STRANS = 0x1a;
const UINT8 RECORD_TYPE_MAG = 0x1b;
const UINT8 RECORD_TYPE_ANGLE = 0x1c;
const UINT8 RECORD_TYPE_REFLIBS = 0x1f;
const UINT8 RECORD_TYPE_FONTS = 0x20;
const UINT8 RECORD_TYPE_PATHTYPE = 0x21;
const UINT8 RECORD_TYPE_GENERATIONS = 0x22;
const UINT8 RECORD_TYPE_ATTRTABLE = 0x23;
const UINT8 RECORD_TYPE_ELFLAGS = 0x26;
const UINT8 RECORD_TYPE_NODETYPE = 0x2a;
const UINT8 RECORD_TYPE_PROPATTR = 0x2b;
const UINT8 RECORD_TYPE_PROPVALUE = 0x2c;
const UINT8 RECORD_TYPE_BOX = 0x2d;
const UINT8 RECORD_TYPE_BOXTYPE = 0x2e;
const UINT8 RECORD_TYPE_PLEX = 0x2f;
const UINT8 RECORD_TYPE_TAPENUM = 0x32;
const UINT8 RECORD_TYPE_TAPECODE = 0x33;
const UINT8 RECORD_TYPE_FORMAT = 0x36;
const UINT8 RECORD_TYPE_MASK = 0x37;
const UINT8 RECORD_TYPE_ENDMASKS = 0x38;

const UINT8 DATA_TYPE_NODATA = 0x00;
const UINT8 DATA_TYPE_BITARRAY = 0x01;
const UINT8 DATA_TYPE_INT16 = 0x02;
const UINT8 DATA_TYPE_INT32 = 0x03;
const UINT8 DATA_TYPE_REAL32 = 0x04;
const UINT8 DATA_TYPE_REAL64 = 0x05;
const UINT8 DATA_TYPE_ASCII = 0x06;

const UINT8 PATH_TYPE_FLUSH = 0x00;
const UINT8 PATH_TYPE_ROUND = 0x01;
const UINT8 PATH_TYPE_SQUARE = 0x02;

enum ELEMENT_TYPE{ boundary, path, sref, aref, box };

struct XY{ REAL32 x; REAL32 y; XY* next; };

struct GDSII_ELEMENT{
    ELEMENT_TYPE type;
    INT16 layer;
    INT16 width;
    UINT8 path_type;   
    GDSII_ELEMENT* selement;
    XY* xy;
    bool use_strans;
    bool reflection;
    bool absolute_magnification;
    bool absolute_angle;
    bool use_magnification;
    REAL64 magnification;
    bool use_angle;
    REAL64 angle;
    GDSII_ELEMENT* next;   
};

struct GDSII_STRUCTURE{
    char* sname;
    GDSII_ELEMENT* element;
    GDSII_STRUCTURE* next;
};

struct GDSII{
    GDSII_STRUCTURE* structure;
};

std::vector<int16_t> parse_int16(uint8_t* data, uint16_t length){
    assert(length%2==0);
    std::vector<int16_t> newdata;
    for(uint i=0; i<length/2; i++){
        newdata.push_back((data[2*i] << 1) + data[2*i+1]);
    }
    return newdata;
}


void gdsii_free (GDSII* gdsii){
    GDSII_STRUCTURE* structure = (*gdsii).structure;
    while(structure != NULL){
        GDSII_ELEMENT* element = (*structure).element;
        while(element != NULL){
            GDSII_ELEMENT* next_e = (*element).next;
            free(element);
            element = next_e;
        }
        GDSII_STRUCTURE* next_s = (*structure).next;
        free(structure);
        structure = next_s;
    }
    free(gdsii);
}

GDSII* gdsii_read(const char* filepath){
    GDSII* gdsii = (GDSII*)malloc(sizeof(gdsii));

    FILE* file = fopen(filepath, "r");
    if(file == NULL){ perror("Error! Could not read GDS file."); return NULL; }

    bool first_structure = true;
    bool first_element = true;
    GDSII_STRUCTURE* structure = (GDSII_STRUCTURE*)malloc(sizeof(GDSII_STRUCTURE));
    (*gdsii).structure = structure;
    GDSII_ELEMENT* element = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
    (*structure).element = element;

    while(true){

        // read record header
        uint8_t buffer[4]; // header is always 32 bits long
        size_t read = fread(buffer, sizeof(uint8_t), 4, file);
        if(read==0){ break; } // reached EOF

        // create new record
        uint16_t length = (buffer[0] << 1) + buffer[1] - 4; // length of data, not entire record
        uint8_t record_type = buffer[2];
        uint8_t data_type = buffer[3];
        std::cout << "RECORD: length: " << (int)length << " type: " << (int)record_type << " data: " << (int)data_type << std::endl;

        // read record data
        uint8_t* data = nullptr;
        if(length > 0){
            data = new uint8_t[length];
            fread(data, sizeof(uint8_t), length, file);
        }

        switch(record_type){
            case RECORD_TYPE_UNITS:
                printf("units\n");
                break;
            case RECORD_TYPE_BGNSTR:
                printf("begin structure\n");
                if(first_structure){ first_structure = false; }else{
                    (*structure).next = (GDSII_STRUCTURE*)malloc(sizeof(GDSII_STRUCTURE));
                    structure = structure->next;
                    element = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    (*structure).element = element; }
                break;
            case RECORD_TYPE_ENDSTR:
                printf("end structure\n");
                structure->next = NULL;
                break;
            case RECORD_TYPE_STRNAME:
                printf("structure name\n");
                break;
            case RECORD_TYPE_ENDEL:
                printf("endel\n");
                element->next = NULL;
                break;
            case RECORD_TYPE_BOUNDARY:
                printf("boundary\n");
                if(first_element){ first_element = false; }else{
                    element->next = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    element = element->next; }
                break;
            case RECORD_TYPE_PATH:
                printf("path\n");
                if(first_element){ first_element = false; }else{
                    element->next = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    element = element->next; }
                break;
            case RECORD_TYPE_SREF:
                printf("sref\n");
                if(first_element){ first_element = false; }else{
                    element->next = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    element = element->next; }
                break;
            case RECORD_TYPE_AREF:
                printf("aref\n");
                if(first_element){ first_element = false; }else{
                    element->next = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    element = element->next; }
                break;
            case RECORD_TYPE_BOX:
                printf("box\n");
                if(first_element){ first_element = false; }else{
                    element->next = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
                    element = element->next; }
                break;
            case RECORD_TYPE_XY:
                printf("xy\n");
                break;
            case RECORD_TYPE_LAYER:
                printf("layer\n");
                //(*element).layer = 0;
                break;
        }

        delete[] data;
    }

    fclose(file);

    return gdsii;
}

#endif