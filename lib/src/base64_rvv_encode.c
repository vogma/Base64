#include <libb64rvv.h>

const unsigned char b64chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64encode_len(int len)
{
    return ((len + 2) / 3 * 4) + 1;
}

int Base64encode(char *encoded, const char *string, int len)
{
    int i;
    char *p;

    p = encoded;
    for (i = 0; i < len - 2; i += 3)
    {
        *p++ = b64chars[(string[i] >> 2) & 0x3F];
        *p++ = b64chars[((string[i] & 0x3) << 4) |
                        ((int)(string[i + 1] & 0xF0) >> 4)];
        *p++ = b64chars[((string[i + 1] & 0xF) << 2) |
                        ((int)(string[i + 2] & 0xC0) >> 6)];
        *p++ = b64chars[string[i + 2] & 0x3F];
    }
    if (i < len)
    {
        *p++ = b64chars[(string[i] >> 2) & 0x3F];
        if (i == (len - 1))
        {
            *p++ = b64chars[((string[i] & 0x3) << 4)];
            *p++ = '=';
        }
        else
        {
            *p++ = b64chars[((string[i] & 0x3) << 4) |
                            ((int)(string[i + 1] & 0xF0) >> 4)];
            *p++ = b64chars[((string[i + 1] & 0xF) << 2)];
        }
        *p++ = '=';
    }

    *p++ = '\0';
    return p - encoded;
}

const uint8_t gather_index_lmul4[32] = {1, 0, 2, 1, 4, 3, 5, 4, 7, 6, 8, 7, 10, 9, 11, 10, 13, 12, 14, 13, 16, 15, 17, 16, 19, 18, 20, 19, 22, 21, 23, 22};

const int8_t offsets[32] = {71, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -19, -16, 65, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

vuint8m4_t __attribute__((always_inline)) inline table_lookup_m4(vuint8m4_t vec_indices, vint8m1_t offset_vec, size_t vl)
{
    // reduce values 0-64 to 0-13
    vuint8m4_t result = __riscv_vssubu_vx_u8m4(vec_indices, 51, vl);
    vbool2_t vec_lt_26 = __riscv_vmsltu_vx_u8m4_b2(vec_indices, 26, vl);
    const vuint8m4_t vec_lookup = __riscv_vadd_vx_u8m4_mu(vec_lt_26, result, result, 13, vl);

    // shuffle registers one by one
    vint8m1_t offset_vec_0 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m4_u8m1(vec_lookup, 0), vl);
    vint8m1_t offset_vec_1 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m4_u8m1(vec_lookup, 1), vl);
    vint8m1_t offset_vec_2 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m4_u8m1(vec_lookup, 2), vl);
    vint8m1_t offset_vec_3 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m4_u8m1(vec_lookup, 3), vl);

    vint8m4_t offset_vec_bundle = __riscv_vcreate_v_i8m1_i8m4(offset_vec_0, offset_vec_1, offset_vec_2, offset_vec_3);

    vint8m4_t ascii_vec = __riscv_vadd_vv_i8m4(__riscv_vreinterpret_v_u8m4_i8m4(vec_indices), offset_vec_bundle, vl);

    return __riscv_vreinterpret_v_i8m4_u8m4(ascii_vec);
}

vuint8m2_t __attribute__((always_inline)) inline table_lookup_m2(vuint8m2_t vec_indices, vint8m1_t offset_vec, size_t vl)
{
    // reduce values 0-64 to 0-13
    vuint8m2_t result = __riscv_vssubu_vx_u8m2(vec_indices, 51, vl);
    vbool4_t vec_lt_26 = __riscv_vmsltu_vx_u8m2_b4(vec_indices, 26, vl);
    const vuint8m2_t vec_lookup = __riscv_vadd_vx_u8m2_mu(vec_lt_26, result, result, 13, vl);

    // shuffle registers one by one
    vint8m1_t offset_vec_0 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m2_u8m1(vec_lookup, 0), vl);
    vint8m1_t offset_vec_1 = __riscv_vrgather_vv_i8m1(offset_vec, __riscv_vget_v_u8m2_u8m1(vec_lookup, 1), vl);

    vint8m2_t offset_vec_bundle = __riscv_vcreate_v_i8m1_i8m2(offset_vec_0, offset_vec_1);

    vint8m2_t ascii_vec = __riscv_vadd_vv_i8m2(__riscv_vreinterpret_v_u8m2_i8m2(vec_indices), offset_vec_bundle, vl);

    return __riscv_vreinterpret_v_i8m2_u8m2(ascii_vec);
}

vuint8m1_t __attribute__((always_inline)) inline table_lookup_m1(vuint8m1_t vec_indices, vint8m1_t offset_vec, size_t vl)
{
    // reduce values 0-64 to 0-13
    vuint8m1_t result = __riscv_vssubu_vx_u8m1(vec_indices, 51, vl);
    vbool8_t vec_lt_26 = __riscv_vmsltu_vx_u8m1_b8(vec_indices, 26, vl);
    const vuint8m1_t vec_lookup = __riscv_vadd_vx_u8m1_mu(vec_lt_26, result, result, 13, vl);

    offset_vec = __riscv_vrgather_vv_i8m1(offset_vec,vec_lookup, vl);

    vint8m1_t ascii_vec = __riscv_vadd_vv_i8m1(__riscv_vreinterpret_v_u8m1_i8m1(vec_indices), offset_vec, vl);

    return __riscv_vreinterpret_v_i8m1_u8m1(ascii_vec);
}

vuint32m4_t __attribute__((always_inline)) inline lookup_m4(vuint8m4_t data, size_t vl)
{

    const vuint32m4_t const_vec_ac = __riscv_vmv_v_x_u32m4(0x04000040, vl);
    const vuint32m4_t const_vec_bd = __riscv_vmv_v_x_u32m4(0x01000010, vl);

    vuint32m4_t input32 = __riscv_vreinterpret_v_u8m4_u32m4(data);

    // mask out so that only a and c bits remain
    vuint32m4_t index_a_c = __riscv_vand_vx_u32m4(input32, 0x0FC0FC00, vl);

    // mask out so that only a and c bits remain
    vuint32m4_t index_b_d = __riscv_vand_vx_u32m4(input32, 0x003F03F0, vl);

    vl = __riscv_vsetvlmax_e16m4();
    // multiply 16-bit integers and store high 16 bits of 32-bit result
    vuint16m4_t vec_shifted_ac = __riscv_vmulhu_vv_u16m4(__riscv_vreinterpret_v_u32m4_u16m4(index_a_c), __riscv_vreinterpret_v_u32m4_u16m4(const_vec_ac), vl);

    // multiply 16-bit integers and store low 16 bits of 32-bit result
    vuint16m4_t vec_shifted_bd = __riscv_vmul_vv_u16m4(__riscv_vreinterpret_v_u32m4_u16m4(index_b_d), __riscv_vreinterpret_v_u32m4_u16m4(const_vec_bd), vl);

    vl = __riscv_vsetvlmax_e32m4();

    return __riscv_vor_vv_u32m4(__riscv_vreinterpret_v_u16m4_u32m4(vec_shifted_ac), __riscv_vreinterpret_v_u16m4_u32m4(vec_shifted_bd), vl);
}

vuint32m2_t __attribute__((always_inline)) inline lookup_m2(vuint8m2_t data, size_t vl)
{

    const vuint32m2_t const_vec_ac = __riscv_vmv_v_x_u32m2(0x04000040, vl);
    const vuint32m2_t const_vec_bd = __riscv_vmv_v_x_u32m2(0x01000010, vl);

    vuint32m2_t input32 = __riscv_vreinterpret_v_u8m2_u32m2(data);

    // mask out so that only a and c bits remain
    vuint32m2_t index_a_c = __riscv_vand_vx_u32m2(input32, 0x0FC0FC00, vl);

    // mask out so that only a and c bits remain
    vuint32m2_t index_b_d = __riscv_vand_vx_u32m2(input32, 0x003F03F0, vl);

    vl = __riscv_vsetvlmax_e16m2();
    // multiply 16-bit integers and store high 16 bits of 32-bit result
    vuint16m2_t vec_shifted_ac = __riscv_vmulhu_vv_u16m2(__riscv_vreinterpret_v_u32m2_u16m2(index_a_c), __riscv_vreinterpret_v_u32m2_u16m2(const_vec_ac), vl);

    // multiply 16-bit integers and store low 16 bits of 32-bit result
    vuint16m2_t vec_shifted_bd = __riscv_vmul_vv_u16m2(__riscv_vreinterpret_v_u32m2_u16m2(index_b_d), __riscv_vreinterpret_v_u32m2_u16m2(const_vec_bd), vl);

    vl = __riscv_vsetvlmax_e32m2();

    return __riscv_vor_vv_u32m2(__riscv_vreinterpret_v_u16m2_u32m2(vec_shifted_ac), __riscv_vreinterpret_v_u16m2_u32m2(vec_shifted_bd), vl);
}

vuint32m1_t __attribute__((always_inline)) inline lookup_m1(vuint8m1_t data, size_t vl)
{

    const vuint32m1_t const_vec_ac = __riscv_vmv_v_x_u32m1(0x04000040, vl);
    const vuint32m1_t const_vec_bd = __riscv_vmv_v_x_u32m1(0x01000010, vl);

    vuint32m1_t input32 = __riscv_vreinterpret_v_u8m1_u32m1(data);

    // mask out so that only a and c bits remain
    vuint32m1_t index_a_c = __riscv_vand_vx_u32m1(input32, 0x0FC0FC00, vl);

    // mask out so that only a and c bits remain
    vuint32m1_t index_b_d = __riscv_vand_vx_u32m1(input32, 0x003F03F0, vl);

    vl = __riscv_vsetvlmax_e16m1();
    // multiply 16-bit integers and store high 16 bits of 32-bit result
    vuint16m1_t vec_shifted_ac = __riscv_vmulhu_vv_u16m1(__riscv_vreinterpret_v_u32m1_u16m1(index_a_c), __riscv_vreinterpret_v_u32m1_u16m1(const_vec_ac), vl);

    // multiply 16-bit integers and store low 16 bits of 32-bit result
    vuint16m1_t vec_shifted_bd = __riscv_vmul_vv_u16m1(__riscv_vreinterpret_v_u32m1_u16m1(index_b_d), __riscv_vreinterpret_v_u32m1_u16m1(const_vec_bd), vl);

    vl = __riscv_vsetvlmax_e32m1();

    return __riscv_vor_vv_u32m1(__riscv_vreinterpret_v_u16m1_u32m1(vec_shifted_ac), __riscv_vreinterpret_v_u16m1_u32m1(vec_shifted_bd), vl);
}

void base64_encode_rvv_m4(uint8_t *input, uint8_t *output, size_t length)
{
    size_t vl;

    size_t vlmax_e8m4 = __riscv_vsetvlmax_e8m4();
    size_t vlmax_e8m1 = __riscv_vsetvlmax_e8m1();

    const vuint8m1_t vec_index_e8m1 = __riscv_vle8_v_u8m1(gather_index_lmul4, vlmax_e8m1);

    vint8m1_t offset_vec = __riscv_vle8_v_i8m1(offsets, vlmax_e8m1);

    size_t input_slice_e8m4 = (vlmax_e8m4 / 4) * 3;
    size_t input_slice_e8m1 = (vlmax_e8m1 / 4) * 3;

    for (; length >= input_slice_e8m4; length -= input_slice_e8m4)
    {

        vl = __riscv_vsetvl_e8m1(input_slice_e8m1);

        /**
         * Load (vlmax_e8m1 / 4) * 3 elements into each vector register.
         */
        vuint8m1_t vec_input_0 = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vuint8m1_t vec_input_1 = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vuint8m1_t vec_input_2 = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vuint8m1_t vec_input_3 = __riscv_vle8_v_u8m1(input, vl);

        vl = __riscv_vsetvl_e8m1(vlmax_e8m1);

        //  the vrgather operation is cheaper at lmul=1 (4*4=16 cycles) than at lmul=4 (64 cycles), therefore each register gets shuffled seperately (https://camel-cdr.github.io/rvv-bench-results/bpi_f3/index.html)
        vuint8m1_t vec_gather_0 = __riscv_vrgather_vv_u8m1(vec_input_0, vec_index_e8m1, vl);
        vuint8m1_t vec_gather_1 = __riscv_vrgather_vv_u8m1(vec_input_1, vec_index_e8m1, vl);
        vuint8m1_t vec_gather_2 = __riscv_vrgather_vv_u8m1(vec_input_2, vec_index_e8m1, vl);
        vuint8m1_t vec_gather_3 = __riscv_vrgather_vv_u8m1(vec_input_3, vec_index_e8m1, vl);

        vuint8m4_t vec_gather = __riscv_vcreate_v_u8m1_u8m4(vec_gather_0, vec_gather_1, vec_gather_2, vec_gather_3);

        vl = __riscv_vsetvlmax_e32m4();

        vuint32m4_t vec_lookup_indices = lookup_m4(vec_gather, vl);

        vl = __riscv_vsetvlmax_e8m4();

        // two different ways to calculate the lookup step
        // vuint8m4_t base64_chars = __riscv_vluxei8_v_u8m4(b64chars, __riscv_vreinterpret_v_u32m4_u8m4(vec_lookup_indices), vl);
        vuint8m4_t base64_chars = table_lookup_m4(__riscv_vreinterpret_v_u32m4_u8m4(vec_lookup_indices), offset_vec, vl);

        __riscv_vse8_v_u8m4(output, base64_chars, vl);

        vl = __riscv_vsetvl_e8m2(length);
        input += (vlmax_e8m1 / 4) * 3;

        output += vlmax_e8m4;
    }
    Base64encode((char *)output, (char *)input, length);
}

void base64_encode_rvv_m2(uint8_t *input, uint8_t *output, size_t length)
{
    size_t vl;

    size_t vlmax_e8m2 = __riscv_vsetvlmax_e8m2();
    size_t vlmax_e8m1 = __riscv_vsetvlmax_e8m1();

    const vuint8m1_t vec_index_e8m1 = __riscv_vle8_v_u8m1(gather_index_lmul4, vlmax_e8m1);

    vint8m1_t offset_vec = __riscv_vle8_v_i8m1(offsets, vlmax_e8m1);

    size_t input_slice_e8m2 = (vlmax_e8m2 / 4) * 3;
    size_t input_slice_e8m1 = (vlmax_e8m1 / 4) * 3;

    for (; length >= input_slice_e8m2; length -= input_slice_e8m2)
    {

        vl = __riscv_vsetvl_e8m1(input_slice_e8m1);

        /**
         * Load (vlmax_e8m1 / 4) * 3 elements into each vector register.
         */
        vuint8m1_t vec_input_0 = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vuint8m1_t vec_input_1 = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vl = __riscv_vsetvl_e8m1(vlmax_e8m1);

        //  the vrgather operation is cheaper at lmul=1 (4*4=16 cycles) than at lmul=4 (64 cycles), therefore each register gets shuffled seperately (https://camel-cdr.github.io/rvv-bench-results/bpi_f3/index.html)
        vuint8m1_t vec_gather_0 = __riscv_vrgather_vv_u8m1(vec_input_0, vec_index_e8m1, vl);
        vuint8m1_t vec_gather_1 = __riscv_vrgather_vv_u8m1(vec_input_1, vec_index_e8m1, vl);

        vuint8m2_t vec_gather = __riscv_vcreate_v_u8m1_u8m2(vec_gather_0, vec_gather_1);

        vl = __riscv_vsetvlmax_e32m2();

        vuint32m2_t vec_lookup_indices = lookup_m2(vec_gather, vl);

        vl = __riscv_vsetvlmax_e8m2();

        // two different ways to calculate the lookup step
        // vuint8m2_t base64_chars = __riscv_vluxei8_v_u8m2(b64chars, __riscv_vreinterpret_v_u32m2_u8m2(vec_lookup_indices), vl);
        vuint8m2_t base64_chars = table_lookup_m2(__riscv_vreinterpret_v_u32m2_u8m2(vec_lookup_indices), offset_vec, vl);

        __riscv_vse8_v_u8m2(output, base64_chars, vl);

        vl = __riscv_vsetvl_e8m2(length);

        output += vlmax_e8m2;
    }
    Base64encode((char *)output, (char *)input, length);
}

void base64_encode_rvv_m1(uint8_t *input, uint8_t *output, size_t length)
{
    size_t vl;

    size_t vlmax_e8m1 = __riscv_vsetvlmax_e8m1();

    const vuint8m1_t vec_index_e8m1 = __riscv_vle8_v_u8m1(gather_index_lmul4, vlmax_e8m1);

    vint8m1_t offset_vec = __riscv_vle8_v_i8m1(offsets, vlmax_e8m1);

    size_t input_slice_e8m1 = (vlmax_e8m1 / 4) * 3;

    for (; length >= input_slice_e8m1; length -= input_slice_e8m1)
    {

        vl = __riscv_vsetvl_e8m1(input_slice_e8m1);

        /**
         * Load (vlmax_e8m1 / 4) * 3 elements
         */
        vuint8m1_t vec_input = __riscv_vle8_v_u8m1(input, vl);
        input += (vlmax_e8m1 / 4) * 3;

        vl = __riscv_vsetvl_e8m1(vlmax_e8m1);

        vuint8m1_t vec_gather = __riscv_vrgather_vv_u8m1(vec_input, vec_index_e8m1, vl);

        vl = __riscv_vsetvlmax_e32m1();

        vuint32m1_t vec_lookup_indices = lookup_m1(vec_gather, vl);

        vl = __riscv_vsetvlmax_e8m1();

        // two different ways to calculate the lookup step
        // vuint8m1_t base64_chars = __riscv_vluxei8_v_u8m1(b64chars, __riscv_vreinterpret_v_u32m1_u8m1(vec_lookup_indices), vl);
        vuint8m1_t base64_chars = table_lookup_m1(__riscv_vreinterpret_v_u32m1_u8m1(vec_lookup_indices), offset_vec, vl);

        __riscv_vse8_v_u8m1(output, base64_chars, vl);

        vl = __riscv_vsetvl_e8m1(length);

        output += vlmax_e8m1;
    }
    Base64encode((char *)output, (char *)input, length);
}
