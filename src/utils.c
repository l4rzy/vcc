#include "utils.h"

FILE *utils_open_file(const char *fname) {
    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        fatals("Error opening file\n");
        return NULL;
    }
    return fp;
}

