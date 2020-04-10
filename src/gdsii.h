#ifndef GDSII_H
#define GDSII_H

// GDSII file format parser

#include <iostream>
#include <fstream>
#include <string>


const uint8_t DATA_TYPE_NODATA = 0x00;
const uint8_t DATA_TYPE_BITARRAY = 0x01;
const uint8_t DATA_TYPE_INT16 = 0x02;
const uint8_t DATA_TYPE_INT32 = 0x03;
const uint8_t DATA_TYPE_REAL32 = 0x04; // not used?
const uint8_t DATA_TYPE_REAL64 = 0x05;
const uint8_t DATA_TYPE_ASCII = 0x06;

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
// ...

struct RECORD_HEADER{
    uint8_t datatype = DATA_TYPE_INT16;
};

struct BGNLIB{
    uint8_t datatype = DATA_TYPE_INT16;
    
    

};

struct PATH{};
struct BOUNDARY{};
struct SREF{};
struct AREF{};
struct TEXT{};
struct NODE{};
struct BOX{};

struct cell{
    // name
    // boundary
    // sref
};

struct layout{

    struct cell* cells;
};

void read(std::string filepath){
    std::ifstream file(filepath, std::ifstream::binary);
    if(!file.good()){ std::cout << "Error: could not read GDS file." << std::endl; return; }

    while(file.good()){
        char buffer[10];
        file.read(buffer, 10);
        std::cout << buffer << std::endl;

        // bgn str
        // name
    }

}

#endif