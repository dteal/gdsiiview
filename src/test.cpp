#include "gdsii.h"

int main(){
    GDSII* gdsii = gdsii_read("test/test.gds");
    gdsii_free(gdsii);
}