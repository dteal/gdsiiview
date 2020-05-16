#ifndef GDSII_PARSE_H
#define GDSII_PARSE_H

#include <stdlib.h>      // malloc
#include <stdio.h>       // fopen
#include <stdbool.h>     // true
#include <string.h>      // strcmp
#include "gdsii_types.h" // GDSII

// parse int16 numbers into buffer allocated at given location
// and return the count of numbers read
size_t gdsii_parse_int16(GDSII_RECORD* record, int16_t** result){

    // sanity checks
    if(record == NULL){ return 0; }
    if(record->datatype != GDSII_DATATYPE_INT16 ){ return 0; }
    if(record->length == 0 || (record->length % 2 != 0)){ return 0; }

    // allocate space for results
    size_t read = record->length/2;
    *result = (int16_t*)malloc(sizeof(int16_t)*read);

    // loop through data
    uint8_t* data = record->data;
    for(unsigned int i=0; i<read; i++){

        // generate result
        (*result)[i] = (data[0] << 8) +
                       (data[1] << 0);

        data += 2;
    }
    return read;
}

// parse int32 numbers into buffer allocated at given location
// and return the count of numbers read
size_t gdsii_parse_int32(GDSII_RECORD* record, int32_t** result){

    // sanity checks
    if(record == NULL){ return 0; }
    if(record->datatype != GDSII_DATATYPE_INT32 ){ return 0; }
    if(record->length == 0 || (record->length % 4 != 0)){ return 0; }

    // allocate space for results
    size_t read = record->length/4;
    *result = (int32_t*)malloc(sizeof(int32_t)*read);

    // loop through data
    uint8_t* data = record->data;
    for(unsigned int i=0; i<read; i++){

        // generate result
        (*result)[i] = (data[0] << 24) +
                       (data[1] << 16) +
                       (data[2] <<  8) +
                       (data[3] <<  0);

        data += 4;
    }
    return read;
}

// parse real64 numbers into buffer allocated at given location
// and return the count of numbers read
size_t gdsii_parse_real64(GDSII_RECORD* record, double** result){

    // sanity checks
    if(record == NULL){ return 0; }
    if(record->datatype != GDSII_DATATYPE_REAL64 ){ return 0; }
    if(record->length == 0 || (record->length % 8 != 0)){ return 0; }

    // allocate space for results
    size_t read = record->length/8;
    *result = (double*)malloc(sizeof(double)*read);

    // loop through data
    uint8_t* data = record->data;
    for(unsigned int i=0; i<read; i++){

        // extract sign, exponent, mantissa
        int8_t sign = (data[0] & 0x80) >> 7;
        if(sign == 1){ sign = -1; }
        if(sign == 0){ sign = 1; }
        int8_t exponent = (data[0] & 0x7f)-64;
        uint64_t mantissa = ((uint64_t)(data[1]) << 48) +
                            ((uint64_t)(data[2]) << 40) +
                            ((uint64_t)(data[3]) << 32) +
                            ((uint64_t)(data[4]) << 24) +
                            ((uint64_t)(data[5]) << 16) +
                            ((uint64_t)(data[6]) <<  8) +
                            ((uint64_t)(data[7]) <<  0);
        double base = ((double)mantissa)/0x0100000000000000;

        // generate result
        double power = 1;
        while(exponent > 0){ power *= 16; exponent -= 1; }
        while(exponent < 0){ power /= 16; exponent += 1; }
        (*result)[i] = sign * base * power;

        data += 8;
    }
    return read;
}

// parse strings into buffer allocated at given location
// and return the count of characters read
size_t gdsii_parse_ascii(GDSII_RECORD* record, char** result){

    // sanity checks
    if(record == NULL){ return 0; }
    if(record->datatype != GDSII_DATATYPE_ASCII ){ return 0; }
    if(record->length == 0 || record->length % 2 == 1){ return 0; }

    // allocate space for results
    size_t read = record->length;
    if(record->data[read-1] == 0x00){ read -= 1; } // string padded with null byte
    *result = (char*)malloc(sizeof(char)*(read + 1));

    // loop through data
    for(unsigned int i=0; i<read; i++){
        (*result)[i] = record->data[i];
    }
    (*result)[read] = 0x00; // terminate string with null byte

    return read;
}

// generates point from given record
// returns NULL if unsuccessful
GDSII_POINT* gdsii_parse_point(GDSII_RECORD* record){

    // sanity checks
    if(record == NULL){ return NULL; }
    if(record->recordtype != GDSII_RECORDTYPE_XY){ return NULL; }

    // read values
    int32_t* result;
    size_t read = gdsii_parse_int32(record, &result);
    if(read == 0){ return NULL; }
    if(read % 2 == 1){ free(result); return NULL; }

    // create linked list
    GDSII_POINT* point = NULL;
    size_t num_points = read / 2;
    for(unsigned int i=0; i<num_points; i++){
        point = gdsii_create_point();
        point->x = result[i*2 + 0];
        point->y = result[i*2 + 1];
        point = point->next;
    }

    free(result);
    return point;
}

// generates element from given record
// returns NULL if unsuccessful
GDSII_ELEMENT* gdsii_parse_element(GDSII_RECORD* record){
    if(record == NULL){ return NULL; }

    // create new element
    GDSII_ELEMENT* element = gdsii_create_element();

    // find element type
    switch(record->recordtype){
    case GDSII_RECORDTYPE_PATH:
        element->type = GDSII_RECORDTYPE_PATH; break;
    case GDSII_RECORDTYPE_BOUNDARY:
        element->type = GDSII_RECORDTYPE_BOUNDARY; break;
    case GDSII_RECORDTYPE_SREF:
        element->type = GDSII_RECORDTYPE_SREF; break;
    case GDSII_RECORDTYPE_AREF:
        element->type = GDSII_RECORDTYPE_AREF; break;
    default:
        gdsii_delete_element(element); return NULL;
    }

    // loop through element records
    while(record != NULL && record->recordtype != GDSII_RECORDTYPE_ENDEL){
        size_t read;
        int16_t* result_int16;
        int32_t* result_int32;
        double* result_double;
        char* result_char;
        GDSII_POINT* result_point;
        switch(record->recordtype){
        case GDSII_RECORDTYPE_LAYER:
            read = gdsii_parse_int16(record, &result_int16);
            if(read == 0){ break; }
            element->layer_num = result_int16[0];
            free(result_int16);
            break;
        case GDSII_RECORDTYPE_DATATYPE:
            read = gdsii_parse_int16(record, &result_int16);
            if(read == 0){ break; }
            element->data_num = result_int16[0];
            free(result_int16);
            break;
        case GDSII_RECORDTYPE_PATHTYPE:
            read = gdsii_parse_int16(record, &result_int16);
            if(read == 0){ break; }
            element->path_type = result_int16[0];
            free(result_int16);
            break;
        case GDSII_RECORDTYPE_WIDTH:
            read = gdsii_parse_int32(record, &result_int32);
            if(read == 0){ break; }
            element->path_width = result_int32[0];
            free(result_int32);
            break;
        case GDSII_RECORDTYPE_XY:
            result_point = gdsii_parse_point(record);
            if(result_point == NULL){ break; }
            gdsii_delete_point(element->point);
            element->point = result_point;
            break;
        case GDSII_RECORDTYPE_SNAME:
            read = gdsii_parse_ascii(record, &result_char);
            if(read == 0){ break; }
            if(element->ref_name != NULL){ free(element->ref_name); }
            element->ref_name = result_char;
            break;
        case GDSII_RECORDTYPE_STRANS:
            element->transform = true;
            if(record->length < 2){ break; }
            element->reflect = ((record->data[0] & 0x80) > 0);
            element->angle_is_absolute = ((record->data[1] & 0x02) > 0);
            element->mag_is_absolute = ((record->data[1] & 0x04) > 0);
            break;
        case GDSII_RECORDTYPE_MAG:
            read = gdsii_parse_real64(record, &result_double);
            if(read == 0){ break; }
            element->magnification = result_double[0];
            free(result_double);
            break;
        case GDSII_RECORDTYPE_ANGLE:
            read = gdsii_parse_real64(record, &result_double);
            if(read == 0){ break; }
            element->angle = result_double[0];
            free(result_double);
            break;
        case GDSII_RECORDTYPE_COLROW:
            read = gdsii_parse_int16(record, &result_int16);
            if(read == 0){ break; }
            if(read < 2){ free(result_int16); break; }
            element->num_cols = result_int16[0];
            element->num_rows = result_int16[1];
            free(result_int16);
            break;
        }
        record = record->next;
    }

    //if(record == NULL){ gdsii_delete_element(element); return NULL; }

    return element;
}

// generates structure from given record
// returns NULL if unsuccessful
GDSII_STRUCTURE* gdsii_parse_structure(GDSII_RECORD* record){

    // sanity checks
    if(record == NULL){ return NULL; }
    if(record->recordtype != GDSII_RECORDTYPE_BGNSTR){ return NULL; }

    // create new structure
    GDSII_STRUCTURE* structure = gdsii_create_structure();
    GDSII_ELEMENT* element = NULL;

    // find structure name
    while(record != NULL && record->recordtype != GDSII_RECORDTYPE_STRNAME){ record = record->next; }
    if(record == NULL){ gdsii_delete_structure(structure); return NULL; }
    char* name;
    size_t read = gdsii_parse_ascii(record, &name);
    if(read == 0){ gdsii_delete_structure(structure); return NULL; }
    structure->name = name;

    // loop through remaining records
    while(record != NULL && record->recordtype != GDSII_RECORDTYPE_ENDSTR){

        // try to parse as element; if successful, add to structure 
        GDSII_ELEMENT* new_element = gdsii_parse_element(record);
        if(new_element == NULL){ record = record->next; continue; }
        if(element == NULL){
            structure->element = new_element;
            element = structure->element;
        }else{
            element->next = new_element;
            element = element->next;
        }

        // don't get stuck in infinite loop
        record = record->next;
    }
    return structure;
}

// parse records in GDSII into its structures
void gdsii_parse_gdsii(GDSII* gdsii){

    // sanity check
    if(gdsii == NULL){ return; }

    // track the current record and structure
    GDSII_RECORD* record = gdsii->record;
    gdsii_delete_structure(gdsii->structure);
    GDSII_STRUCTURE* structure = NULL;

    // find units
    while(record != NULL && record->recordtype != GDSII_RECORDTYPE_UNITS){ record = record->next; }
    if(record == NULL){ return; }
    double* units;
    size_t read = gdsii_parse_real64(record, &units);
    if(read == 0){ return; } 
    if(read != 2){ free(units); return; } 
    gdsii->model_units_per_database_unit = units[0];
    gdsii->meters_per_database_unit = units[1];
    free(units);

    // loop through remaining records
    while(record != NULL){

        // find next structure
        while(record != NULL && record->recordtype != GDSII_RECORDTYPE_BGNSTR){ record = record->next; }
        if(record == NULL){ break; }

        // add new structure to gdsii
        GDSII_STRUCTURE* new_structure = gdsii_parse_structure(record);
        if(new_structure == NULL){ record = record->next; continue; }
        if(structure == NULL){
            gdsii->structure = new_structure;
            structure = gdsii->structure;
        }else{
            structure->next = new_structure;
            structure = structure->next;
        }

        // don't get stuck in infinite loop
        record = record->next;
    }

    // loop through structures to link SREF and AREF references
    structure = gdsii->structure;
    while(structure != NULL){

        // loop through elements
        GDSII_ELEMENT* element = structure->element;
        while(element != NULL){
            if(element->type == GDSII_RECORDTYPE_SREF || element->type == GDSII_RECORDTYPE_AREF){

                // make second pass to find referenced structure
                GDSII_STRUCTURE* temp_structure = gdsii->structure;
                while(temp_structure != NULL){
                    if(strcmp(element->ref_name, temp_structure->name)){
                        element->ref = temp_structure;
                        break;
                    }
                    temp_structure = temp_structure->next;
                }

            }
            element = element->next;
        }
        structure = structure->next;
    }
}

// generates GDSII from given file
// returns NULL if unsuccessful
GDSII* gdsii_read_file(const char* filename){

    // open file
    FILE* file = fopen(filename, "rb");
    if(file == NULL){
        fprintf(stderr, "Error: could not open file \"%s\".\n", filename);
        return NULL;
    }

    GDSII* gdsii = gdsii_create_gdsii(); // new GDSII object
    GDSII_RECORD* record = NULL; // current record

    // loop through records in file
    while(true){

        // read first four bytes (record header)
        uint8_t buffer[4];
        size_t read = fread(buffer, sizeof(uint8_t), 4, file);

        if(read < 4){
            // quit on error
            if(ferror(file)){
                fprintf(stderr, "Error: error while reading \"%s\".\n", filename);
                gdsii_delete_gdsii(gdsii);
                fclose(file);
                return NULL;
            }
            // else, assume end of file was reached
            fclose(file);
            break;
        }

        uint16_t length = (buffer[0] << 8) + buffer[1]; // big-endian

        // reached optional group of null bytes after last record used to
        // fill physical block, so this is effectively the end of the file
        if(length == 0 && buffer[2] == 0 && buffer[3] == 0){
            fclose(file);
            break;
        }
        // quit on error
        if((length < 4) || (length % 2 != 0)){
            fprintf(stderr, "Error: GDSII record is incorrect length.\n");
            gdsii_delete_gdsii(gdsii);
            fclose(file);
            return NULL;
        }

        // create new record at end of linked list
        if(record == NULL){ // first item in list
            record = gdsii_create_record();
            gdsii->record = record;
        }else{ // end of list
            record->next = gdsii_create_record();
            record = record->next;
        }
        length -= 4; // get remaining number of bytes in record
        record->length = length;
        record->recordtype = buffer[2];
        record->datatype = buffer[3];

        // read remainder of record
        if(length > 0){
            record->data = (uint8_t*)malloc(length*sizeof(uint8_t));
            read = fread(record->data, sizeof(uint8_t), length, file);

            // quit on error
            if(read < length){
                fprintf(stderr, "Error: GDSII record data is incomplete.\n");
                gdsii_delete_record(record);
                gdsii_delete_gdsii(gdsii);
                fclose(file);
                return NULL;
            }
        }
    }

    // finally, parse records
    gdsii_parse_gdsii(gdsii);

    return gdsii;
}

#endif // GDSII_PARSE_H
