
#ifndef PARSE_PRIV_H_
#define PARSE_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parser_pub.h"
#include <stdio.h>

#define DEBUG 1
#define DEBUG_LVL 5

#include "debug.h"

/*enum {
    INT_PROC_ID_MIN = 0,
    INT_PROC_ID_MAX = 19,
    INT_FUNC_ID_MIN,
    INT_FUNC_ID_MAX = 255,
};*/

#define VAR_IN_EQ(var, from, to) (var >= from && var <= to)

typedef enum {
    // pcs_unknown = -1,
    pcs_none,
    //   pcs_begin_line,
    pcs_token,       // token
    pcs_repeated,    // repeat if need, splitted by comma
    pcs_command,     // command
                     //   pcs_command_or_label,  // command or label
    pcs_expression,  // expression
    pcs_number,      // number
    pcs_string,      // string
    pcs_array,
    // pcs_var_and_arr,   // variable include array
    pcs_variable,  // variable
    pcs_byte,
    pcs_multi_type,  // multi val - string or expression or number or var
                     // pcs_any_sym,     // any syms
    pcs_function,    // internal func
                     // pcs_block,       // func block
    pcs_var_last,    // var in stack
    pcs_var_declare,
    pcs_else,

    pcs_space = 0x20,
    // 0x20 - 0x7f reserved ASCII symbs
    pcs_IND_optional = 0x7f,
    pcs_MOD_group = 0x80,
} parse_type_t;

enum {
    efl_inline_const_string = 1 << 0,
    efl_inline_const_array = 1 << 1,
    efl_const_array = 1 << 2,
    efl_var_array = 1 << 3,
    efl_compile_calc_expr = 1 << 4,
    efl_expr = 1 << 5,

    efl_any_array = efl_inline_const_string | efl_inline_const_array | efl_const_array | efl_var_array,
    efl_any_expr = efl_compile_calc_expr | efl_expr,
    efl_ALL = 0xff,
};

typedef enum {
    // internal procedures

    // cmd_id_int_last = INT_PROC_ID_MAX,

    // reserved words
    cmd_id_reserved = 100 - 1,
    cmd_id_BYTE,
    cmd_id_CHAR,
    cmd_id_VAR,
    cmd_id_IF,
    cmd_id_WHILE,
    cmd_id_BREAK,
    cmd_id_GOTO,
    cmd_id_RETURN,
    cmd_id_STOP,
    cmd_id_BRKPNT,
    cmd_id_CONST,

    // syntax elements
    cmd_id_PROC = 150,
    cmd_id_BLOCK_BEGIN,
    cmd_id_BLOCK_END,

    cmd_id_end = 200,
} cmd_id_list_t;

#define EXT_OPER_LIST CONV_TYPE(ELSE, 0, {pcs_IND_optional, pcs_command, 0}),               \
                      CONV_TYPE(LET, efl_any_expr, {pcs_variable, '=', pcs_expression, 0}), \
                      CONV_TYPE(CALL, 0, {pcs_token, 0}), /*all funcs have one templ*/

#define LET_pnt_expr_flags (efl_any_array)

#define CONV_TYPE(id, ...) cmd_ext_id_##id
typedef enum { cmd_ext_pre_begin = cmd_id_end - 1,
               EXT_OPER_LIST
                   cmd_ext_id_end,
} cmd_ext_id_list_t;
#undef CONV_TYPE

typedef struct {
    parse_type_t type;
    // int link;
    fpt value;  // if number
    ctx_var_info_t ctx_var_info;
    bool calc_comptime;
    int *brackets_cnt;
#ifdef DEBUG
    char name[TOKEN_MAX_LEN];
#endif
} expr_info_t;

typedef enum {
    vsm_DECLARE_VAR,
    vsm_DECLARE_BYTE_ARRAY,
    vsm_DECLARE_CHAR_ARRAY,
    vsm_DECLARE_VAR_ARRAY,
    vsm_DECLARE_end,
    vsm_GET_VAR,
    vsm_SET_VAR,    // set generic var
    vsm_SET_ARRAY,  // as whole array
    vsm_GET_ARRAY,  // as whole array
} var_set_mode_t;

#define MAX_OF(type) \
    (((type)(~0U) > (type)((1U << ((sizeof(type) << 3) - 1)) - 1U)) ? (unsigned int)(type)(~0U) : (unsigned int)(type)((1U << ((sizeof(type) << 3) - 1)) - 1U))
#define MIN_OF(type) \
    (((type)(1U << ((sizeof(type) << 3) - 1)) < (type)1) ? (int)((~0U) - ((1U << ((sizeof(type) << 3) - 1)) - 1U)) : 0)

#ifdef __cplusplus
}
#endif
#endif