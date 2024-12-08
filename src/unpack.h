#ifndef CDATAPACK_UNPACK_H
#define CDATAPACK_UNPACK_H

#include "include.h"

int cdpk_get_data(const char* path, char** output_buf, size_t* output_size);

int cdpk_get_data_size(const char* path, size_t* output_size);

#endif