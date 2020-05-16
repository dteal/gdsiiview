#ifndef GDSII_TYPES_H
#define GDSII_TYPES_H

#include <stdlib.h>  // malloc
#include <stdint.h>  // uint8_t
#include <stdbool.h> // bool

//========== GDSII File Constants ==========//

// possible record types
#define GDSII_RECORDTYPE_HEADER       0x00
#define GDSII_RECORDTYPE_BGNLIB       0x01
#define GDSII_RECORDTYPE_LIBNAME      0x02
#define GDSII_RECORDTYPE_UNITS        0x03
#define GDSII_RECORDTYPE_ENDLIB       0x04
#define GDSII_RECORDTYPE_BGNSTR       0x05
#define GDSII_RECORDTYPE_STRNAME      0x06
#define GDSII_RECORDTYPE_ENDSTR       0x07
#define GDSII_RECORDTYPE_BOUNDARY     0x08
#define GDSII_RECORDTYPE_PATH         0x09
#define GDSII_RECORDTYPE_SREF         0x0a
#define GDSII_RECORDTYPE_AREF         0x0b
#define GDSII_RECORDTYPE_TEXT         0x0c
#define GDSII_RECORDTYPE_LAYER        0x0d
#define GDSII_RECORDTYPE_DATATYPE     0x0e
#define GDSII_RECORDTYPE_WIDTH        0x0f
#define GDSII_RECORDTYPE_XY           0x10
#define GDSII_RECORDTYPE_ENDEL        0x11
#define GDSII_RECORDTYPE_SNAME        0x12
#define GDSII_RECORDTYPE_COLROW       0x13
#define GDSII_RECORDTYPE_TEXTNODE     0x14
#define GDSII_RECORDTYPE_NODE         0x15
#define GDSII_RECORDTYPE_TEXTTYPE     0x16
#define GDSII_RECORDTYPE_PRESENTATION 0x17
#define GDSII_RECORDTYPE_STRING       0x19
#define GDSII_RECORDTYPE_STRANS       0x1a
#define GDSII_RECORDTYPE_MAG          0x1b
#define GDSII_RECORDTYPE_ANGLE        0x1c
#define GDSII_RECORDTYPE_REFLIBS      0x1f
#define GDSII_RECORDTYPE_FONTS        0x20
#define GDSII_RECORDTYPE_PATHTYPE     0x21
#define GDSII_RECORDTYPE_GENERATIONS  0x22
#define GDSII_RECORDTYPE_ATTRTABLE    0x23
#define GDSII_RECORDTYPE_STYPTABLE    0x24
#define GDSII_RECORDTYPE_STRTYPE      0x25
#define GDSII_RECORDTYPE_ELFLAGS      0x26
#define GDSII_RECORDTYPE_ELKEY        0x27
#define GDSII_RECORDTYPE_LINKTYPE     0x28
#define GDSII_RECORDTYPE_LINKKEYS     0x29
#define GDSII_RECORDTYPE_NODETYPE     0x2a
#define GDSII_RECORDTYPE_PROPATTR     0x2b
#define GDSII_RECORDTYPE_PROPVALUE    0x2c
#define GDSII_RECORDTYPE_BOX          0x2d
#define GDSII_RECORDTYPE_BOXTYPE      0x2e
#define GDSII_RECORDTYPE_PLEX         0x2f
#define GDSII_RECORDTYPE_BGNEXTN      0x30
#define GDSII_RECORDTYPE_ENDEXTN      0x31
#define GDSII_RECORDTYPE_TAPENUM      0x32
#define GDSII_RECORDTYPE_TAPECODE     0x33
#define GDSII_RECORDTYPE_STRCLASS     0x34
#define GDSII_RECORDTYPE_RESERVED     0x35
#define GDSII_RECORDTYPE_FORMAT       0x36
#define GDSII_RECORDTYPE_MASK         0x37
#define GDSII_RECORDTYPE_ENDMASKS     0x38
#define GDSII_RECORDTYPE_LIBDIRSIZE   0x39
#define GDSII_RECORDTYPE_SRFNAME      0x3a
#define GDSII_RECORDTYPE_LIBSECUR     0x3b
#define GDSII_RECORDTYPE_UNKNOWN      0xff

// possible record data types
#define GDSII_DATATYPE_NODATA         0x00
#define GDSII_DATATYPE_BITARRAY       0x01
#define GDSII_DATATYPE_INT16          0x02
#define GDSII_DATATYPE_INT32          0x03
#define GDSII_DATATYPE_REAL32         0x04
#define GDSII_DATATYPE_REAL64         0x05
#define GDSII_DATATYPE_ASCII          0x06

// possible path types
#define GDSII_PATHTYPE_FLUSH          0x00
#define GDSII_PATHTYPE_ROUND          0x01
#define GDSII_PATHTYPE_SQUARE         0x02

//========== Data Storage Structures ==========//

// store all information from file record
typedef struct GDSII_RECORD GDSII_RECORD;
struct GDSII_RECORD{            // a GDSII file record
    uint16_t length;            // number of bytes in data (besides 4-byte header)
    uint8_t recordtype;         // type of record
    uint8_t datatype;           // type of data
    uint8_t* data;              // record data
    GDSII_RECORD* next;         // (to implement linked list of pointers)
};

// store GDSII XY structure
typedef struct GDSII_POINT GDSII_POINT;
struct GDSII_POINT{             // a 2D point (XY)
    int32_t x, y;               // coordinates in database units
    GDSII_POINT* next;          // (to implement linked list of points)
};

// store any element with physical geometry
typedef struct GDSII_ELEMENT GDSII_ELEMENT;
typedef struct GDSII_STRUCTURE GDSII_STRUCTURE;
struct GDSII_ELEMENT{           // a boundary, path, SREF, or AREF
    uint8_t type;               // type of element
    int16_t layer_num;          // GDSII layer number (0-255)
    int16_t data_num;           // GDSII data number (0-255)
    GDSII_POINT* point;         // linked list of points in this element
    uint8_t path_type;          // type of path
    int32_t path_width;         // width of path (negative means absolute)
    GDSII_STRUCTURE* ref;       // referenced structure (for type SREF, AREF)
    char* ref_name;             // name of referenced structure (dynamically allocated)
    int16_t num_cols;           // number of columns in AREF array
    int16_t num_rows;           // number of rows in AREF array
    bool transform;             // whether transformations are applied (STRANS)
    bool reflect;               // whether to reflect about x-axis
    double angle;               // rotation angle (degrees)
    bool angle_is_absolute;     // whether angle is absolute (vs. hierarchical)
    double magnification;       // magnification amount (1 = no effect)
    bool mag_is_absolute;       // whether magnification is absolute
    GDSII_ELEMENT* next;        // (to implement linked list of elements)
};

// store a structure with multiple elements
struct GDSII_STRUCTURE{         // a collection of elements
    char* name;                 // structure name (ASCII, dynamically allocated)
    GDSII_ELEMENT* element;     // linked list of elements in this structure
    GDSII_STRUCTURE* next;      // (to implement linked list of structures)
};

// store a complete GDSII file
typedef struct GDSII GDSII;
struct GDSII{                   // a complete GDSII file
    double model_units_per_database_unit;
    double meters_per_database_unit;
    GDSII_RECORD* record;       // linked list of records
    GDSII_STRUCTURE* structure; // linked list of structures
};

//========== Data Storage Structure Handlers ==========//

GDSII_RECORD* gdsii_create_record(){
    GDSII_RECORD* record;
    record = (GDSII_RECORD*)malloc(sizeof(GDSII_RECORD));
    record->length = 0;
    record->recordtype = 0;
    record->datatype = 0;
    record->data = NULL;
    record->next = NULL;
    return record;
}

void gdsii_delete_record(GDSII_RECORD* record){
    if(record == NULL){ return; }
    free(record->data);
    gdsii_delete_record(record->next);
    free(record);
}

GDSII_POINT* gdsii_create_point(){
    GDSII_POINT* point;
    point = (GDSII_POINT*)malloc(sizeof(GDSII_POINT));
    point->x = 0;
    point->y = 0;
    point->next = NULL;
    return point;
}

void gdsii_delete_point(GDSII_POINT* point){
    if(point == NULL){ return; }
    gdsii_delete_point(point->next);
    free(point);
}

GDSII_ELEMENT* gdsii_create_element(){
    GDSII_ELEMENT* element;
    element = (GDSII_ELEMENT*)malloc(sizeof(GDSII_ELEMENT));
    element->type = GDSII_RECORDTYPE_UNKNOWN;
    element->layer_num = 0;
    element->data_num = 0;
    element->point = NULL;
    element->path_type = GDSII_PATHTYPE_FLUSH;
    element->path_width = 0;
    element->ref = NULL;
    element->ref_name = NULL;
    element->num_cols = 1;
    element->num_rows = 1;
    element->transform = false;
    element->reflect = false;
    element->angle = 0;
    element->angle_is_absolute = false;
    element->magnification = 1;
    element->mag_is_absolute = false;
    element->next = NULL;
    return element;
}

void gdsii_delete_element(GDSII_ELEMENT* element){
    if(element == NULL){ return; }
    gdsii_delete_point(element->point);
    if(element->ref_name != NULL){ free(element->ref_name); }
    gdsii_delete_element(element->next);
    free(element);
}

GDSII_STRUCTURE* gdsii_create_structure(){
    GDSII_STRUCTURE* structure;
    structure = (GDSII_STRUCTURE*)malloc(sizeof(GDSII_STRUCTURE));
    structure->name = NULL;
    structure->element = NULL;
    structure->next = NULL;
    return structure;
}

void gdsii_delete_structure(GDSII_STRUCTURE* structure){
    if(structure == NULL){ return; }
    if(structure->name != NULL){ free(structure->name); }
    gdsii_delete_element(structure->element);
    gdsii_delete_structure(structure->next);
    free(structure);
}

GDSII* gdsii_create_gdsii(){
    GDSII* gdsii;
    gdsii = (GDSII*)malloc(sizeof(GDSII));
    gdsii->model_units_per_database_unit = 1;
    gdsii->meters_per_database_unit = 1;
    gdsii->record = NULL;
    gdsii->structure = NULL;
    return gdsii;
}

void gdsii_delete_gdsii(GDSII* gdsii){
    if(gdsii == NULL){ return; }
    gdsii_delete_record(gdsii->record);
    gdsii_delete_structure(gdsii->structure);
    free(gdsii);
}

#endif // GDSII_TYPES_H
