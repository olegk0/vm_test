#ifndef VM_EXE_H_
#define VM_EXE_H_

#define DEBUG 1
#define DEBUG_LVL 2
// #define WRITE_LOG

#include <stdint.h>
#include "debug.h"
#include "../shared/vm_config.h"

typedef enum {
    ec_ok,
    ec_end,
    ec_break_point,
    ec_inval_code,
    ec_call_unknown_func,
    ec_stack_corrupted,
    ec_div_except,
} err_code_t;

err_code_t Run(uint8_t *code_block, uint8_t *const_block, vm_header_t *vm_header, char print_stat);

#endif