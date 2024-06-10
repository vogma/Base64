#ifndef LIBB64RVV_H
#define LIBCALC_H

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "riscv_vector.h"
#include <time.h>

// scalar code
int Base64encode(char *encoded, const char *string, int len);
unsigned char *base64_decode(const unsigned char *data, size_t input_length, unsigned char *output_data, size_t *output_length);
unsigned int base64_decode_tail(const char *in, unsigned int inlen, unsigned char *out);

// vectorized code
void base64_encode_rvv(uint8_t *restrict input, uint8_t *output, size_t length);
size_t base64_decode_rvv(const char *data, int8_t *output, size_t input_length);

size_t DecodeChunk(const char *in, size_t inLen,uint8_t *out);

int Base64encode_len(int len);

// void build_decoding_table();
// void base64_cleanup();

#endif