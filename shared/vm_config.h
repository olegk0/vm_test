#ifndef VM_CONFIG_H_
#define VM_CONFIG_H_

#define FPT_BITS 32   // fixed point numeric length
#define FPT_WBITS 24  // int part of fixed point numeric
#define VARS_ADDR_BITS 16
#define PRG_ADDR_BITS 16
// #define ARRAY_INDEX_BITS 8
#define FUNC_MAX_PARAMS 5
#define TOKEN_MAX_LEN 10

#include "fptc.h"

typedef struct {
    uint16_t entry_point;
    uint16_t prg_size;
    uint16_t heap_min_size;
    uint16_t const_block_size;
    uint8_t const_block[];
} vm_header_t;

#define FUNC_WITH_RETURN_FL 128

enum {
    // internal procedures
    cmd_id_NOP = 0,
    cmd_id_PRINT,
    cmd_id_PRINT_LN,
    /*cmd_id_SET_CURS,
    cmd_id_CLS,  // ClearScreen
    cmd_id_BEEP,
    cmd_id_OUT,
    cmd_id_LOAD,*/
    // cmd_id_MEM,  // get used mem
};

// #include <math.h>
// #define BITS_TO_MAX_VALUE(bits) pow(2, bits)

#define BITS_TO_BYTES(bits) bits / 8

typedef enum {
    print_ss_none,
    print_ss_var,
    print_ss_expr,
    print_ss_POINTERS = 8,
    print_ss_const = print_ss_POINTERS,
    print_ss_MAX = 0xf,  // max
} print_part_type_t;

#endif