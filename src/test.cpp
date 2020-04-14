#include "gdsii.h"

int main(){
    GDSII* gdsii = gdsii_create_gdsii();
    gdsii_read(gdsii, "example/itjra.gds");
    gdsii_delete_gdsii(gdsii);
}