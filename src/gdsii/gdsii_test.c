#include "stdio.h"
#include "gdsii_parse.h"

int main(){
    //GDSII* test = gdsii_read_file("../../example/example.gds");
    GDSII* test = gdsii_read_file("test.gds");
    if(test==NULL) return 0;

    gdsii_delete_gdsii(test);
    return 0;
}
