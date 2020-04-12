#include "gdsii.h"

int main(){
    GDSII* gdsii = gdsii_create_gdsii();
    gdsii_read(gdsii, "test/test.gds");
    gdsii_delete_gdsii(gdsii);
}