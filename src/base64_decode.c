#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "riscv_vector.h"
#include <time.h>

#define NO_ERROR -1

static char *decoding_table = NULL;

static char encoding_table[65] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                  'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                  '4', '5', '6', '7', '8', '9', '+', '/'};

void build_decoding_table()
{

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char)encoding_table[i]] = i;
}

unsigned char *base64_decode(const unsigned char *data,
                             size_t input_length,
                             size_t *output_length)
{

    if (decoding_table == NULL)
        build_decoding_table();

    if (input_length % 4 != 0)
        return NULL;

    *output_length = input_length / 4 * 3;

    // == possible on end of input stream
    if (data[input_length - 1] == '=')
        (*output_length)--;
    if (data[input_length - 2] == '=')
        (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL)
        return NULL;

    for (int i = 0, j = 0; i < input_length;)
    {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < *output_length)
            decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length)
            decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length)
            decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

void base64_cleanup()
{
    free(decoding_table);
}

vint8m1_t vector_lookup_naive(vint8m1_t data, size_t vl)
{

    vint8m1_t offset_reg = __riscv_vmv_v_x_i8m1(0, vl);

    vbool8_t mask_gt_A = __riscv_vmsgt_vx_i8m1_b8(data, 64, vl);
    vbool8_t mask_lt_Z = __riscv_vmslt_vx_i8m1_b8(data, 91, vl);
    vbool8_t mask_AZ = __riscv_vmand_mm_b8(mask_gt_A, mask_lt_Z, vl);
    offset_reg = __riscv_vmerge_vxm_i8m1(offset_reg, -65, mask_AZ, vl);

    vbool8_t mask_gt_a = __riscv_vmsgt_vx_i8m1_b8(data, 96, vl);
    vbool8_t mask_lt_z = __riscv_vmslt_vx_i8m1_b8(data, 123, vl);
    vbool8_t mask_az = __riscv_vmand_mm_b8(mask_gt_a, mask_lt_z, vl);
    offset_reg = __riscv_vmerge_vxm_i8m1(offset_reg, -71, mask_az, vl);

    vbool8_t mask_gt_0 = __riscv_vmsgt_vx_i8m1_b8(data, 47, vl);
    vbool8_t mask_lt_9 = __riscv_vmslt_vx_i8m1_b8(data, 58, vl);
    vbool8_t mask_09 = __riscv_vmand_mm_b8(mask_gt_0, mask_lt_9, vl);
    offset_reg = __riscv_vmerge_vxm_i8m1(offset_reg, 4, mask_09, vl);

    vbool8_t mask_eq_plus = __riscv_vmseq_vx_i8m1_b8(data, '+', vl);
    offset_reg = __riscv_vmerge_vxm_i8m1(offset_reg, 19, mask_eq_plus, vl);

    vbool8_t mask_eq_slash = __riscv_vmseq_vx_i8m1_b8(data, '/', vl);
    offset_reg = __riscv_vmerge_vxm_i8m1(offset_reg, 16, mask_eq_slash, vl);

    // if any of the elements of offset_reg is 0, the input contains invalid characters
    int error = __riscv_vfirst_m_b8(__riscv_vmseq_vx_i8m1_b8(offset_reg, 0, vl), vl);

    if (error != NO_ERROR)
    {
        printf("ERROR!\n");
    }

    return offset_reg;
}

void base64_decode_rvv(const unsigned char *data, uint8_t *output, size_t input_length, size_t output_length)
{
    size_t vlmax_8 = __riscv_vsetvlmax_e8m1();

    vuint8m1_t data_reg = __riscv_vle8_v_u8m1(data, vlmax_8);

    vint8m1_t offset_reg = vector_lookup_naive(__riscv_vreinterpret_v_u8m1_i8m1(data_reg), vlmax_8);

       // __riscv_vse8_v_u8m1(output, data_reg, vlmax_8);
    // __riscv_vse8_v_u8m1_m(mask_az, output, data_reg, vlmax_8);
    __riscv_vse8_v_i8m1(output, offset_reg, vlmax_8);
}

int main(void)
{
    const unsigned char *base64_data = (unsigned char *)"QUJDREVGR2FiY2RlZmcxMjM0NTY3";
    size_t output_length = 0;
    size_t output_length_rvv = 0;

    int8_t *output_scalar = (int8_t *)malloc(30 * sizeof(uint8_t));
    int8_t *output_rvv = (int8_t *)malloc(30 * sizeof(uint8_t));

    build_decoding_table();

    // for (int i = 0; i < 256; i++)
    // {
    //     if (decoding_table[i] != 0 || i == 65)
    //     {
    //         printf("%d: 0x%02X\n", i, decoding_table[i]);
    //     }
    // }

    output_scalar = (uint8_t *)base64_decode(base64_data, 28, &output_length);
    base64_decode_rvv(base64_data, output_rvv, 28, output_length_rvv);
    // unsigned char *decoded = base64_decode(base64_data, 28, &output_length);

    printf("Original: %s\n", base64_data);

    for (int i = 0; i < 16; i++)
    {
        printf("%d ", base64_data[i]);
    }

    printf("\n\nDecoded:\n");
    for (int i = 0; i < 16; i++)
    {
        printf("%d ", output_scalar[i]);
    }

    printf("\nDecoded_rvv:\n", output_rvv);

    for (int i = 0; i < 16; i++)
    {
        printf("%d ", output_rvv[i]);
    }
    printf("\n");

    free(output_rvv);
    free(output_scalar);

    base64_cleanup();
    return 0;
}