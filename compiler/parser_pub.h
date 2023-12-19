#ifndef PARSER_PUB_H_
#define PARSER_PUB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LINKS_BY_LABEL_MAX 20

#define FPT2STR

enum {
    FALSE,
    TRUE
};
typedef unsigned char bool;

#include <stdio.h>
#include <stdint.h>
#include "../shared/vector.h"
#include "../shared/vm_config.h"

#define ERRORS_LIST CONV_TYPE(no_error, "Ok"),                                               \
                    CONV_TYPE(stop, "STOP command"),                                         \
                    CONV_TYPE(token_max_len_exceeded, "maximum token length exceeded"),      \
                    CONV_TYPE(token_start_with_num, "token start with num"),                 \
                    CONV_TYPE(token_error_parse, "error parse token"),                       \
                    CONV_TYPE(object_max_count_exceeded, "objects max count exceeded"),      \
                    CONV_TYPE(object_exist, "object exist"),                                 \
                    CONV_TYPE(object_unknown, "object is unknown"),                          \
                    CONV_TYPE(command_invalid, "invalid command"),                           \
                    CONV_TYPE(expression_invalid, "invalid expression"),                     \
                    CONV_TYPE(syntax_invalid, "invalid syntax"),                             \
                    CONV_TYPE(error_eval_exp, "error evaluate expression at compile time"),  \
                    CONV_TYPE(stack_corrupted, "stack corrupted"),                           \
                    CONV_TYPE(label_invalid, "label invalid"),                               \
                    CONV_TYPE(var_error_parse, "var error parse"),                           \
                    CONV_TYPE(array_or_string_too_long, "array or string is too long"),      \
                    CONV_TYPE(number_error_parse, "number error parse"),                     \
                    CONV_TYPE(var_redeclare, "var redeclaration with different parameters"), \
                    CONV_TYPE(var_incompatible, "variable incompatible type"),               \
                    CONV_TYPE(array_index_overrange, "array index overrange"),               \
                    CONV_TYPE(parameters_mismatch, "parameters mismatch"),                   \
                    CONV_TYPE(array_index_invalid, "invalid array index"),

#define CONV_TYPE(err, str) pe_##err
typedef enum { ERRORS_LIST } parse_error_t;
#undef CONV_TYPE

VECTOR(uint8_t, bytes_vect_t);

VECTOR(int, ints_vect_t);

typedef enum {
    vt_none = 0,
    vt_generic,  // fixed point variable
    vt_arrays = 0x80,
    vt_array_of_byte,
    vt_array_of_generic,
} var_type_t;

typedef struct {
    bool subs_body;
    int in_block;
    var_type_t type;
    int mem_offs;
    int mem_size;
    int elm_count;  // array max size 255
    char name[TOKEN_MAX_LEN];
} var_info_t;
VECTOR(var_info_t, var_vect_t);

typedef struct {
    char name[TOKEN_MAX_LEN];
    fpt data;
    // int mem_offs;
} const_gen_info_t;
VECTOR(const_gen_info_t, const_gen_vect_t);

typedef struct {
    char name[TOKEN_MAX_LEN];
    fpt data[260];
    int mem_size;
    int elm_count;  // array max size 255
    int mem_offs;
    var_type_t type;
} const_array_info_t;
VECTOR(const_array_info_t, const_array_vect_t);

typedef struct {
    bool is_pointer;
    int array_compile_time_idx;
    var_info_t *var_info;
} ctx_var_info_t;

typedef struct {
    int in_block;
    // var_vect_t vars_vect;
    ctx_var_info_t params_info[FUNC_MAX_PARAMS];
    uint8_t params_cnt;
    uint8_t func_id;
    int vm_code_offset;
    char name[TOKEN_MAX_LEN];
    // char main_func;
    // char have_return;
    ctx_var_info_t return_var;
} func_info_t;
VECTOR(func_info_t, func_vect_t);

typedef struct {
    int line_num;
    int code_offs;
    bool subs_body;
    int jmp_code_offs_arr[LINKS_BY_LABEL_MAX];
    int jmp_code_offs_cnt;
    int id;
    char name[TOKEN_MAX_LEN];
    func_info_t *func_info;
} label_info_t;
VECTOR(label_info_t, label_vect_t);

typedef enum {
    bt_none = 0,
    bt_proc,
    bt_loop,
    bt_condition,  // if else block
    bt_if,
    bt_else,
} block_type_t;

typedef struct {
    block_type_t block_type;
    label_info_t *label_begin;
    label_info_t *label_end;
    // var_vect vars_vect;
} block_info_t;
VECTOR(block_info_t, block_vect_t);

typedef struct {
    char *data;
    // char past_sym;
    // char cur_sym;
    // char new_line_fl;
    int line_num;
    int line_len;
    int line_pnt_ro;
    // int line_pnt_back;
} line_str_t;

typedef struct {
    char data[TOKEN_MAX_LEN];
    int size;
    uint8_t parsed_cmd;
    bool token_valid;
} token_str_t;

typedef struct {
    int params_cnt;
    uint8_t params_type[FUNC_MAX_PARAMS];
} params_str_t;

typedef struct {
    bool enable_code_gen;
    // char enable_code_gen_back;
    line_str_t line_str;
    // bytes_vect_t line_pnts_vect;
    token_str_t token;
    char token_back[TOKEN_MAX_LEN];
    // int labels_max;
    label_vect_t labels_vect;
    // int vars_max;
    // int vars_memory;
    var_vect_t vars_vect;
    func_vect_t funcs_vect;
    // int const_gen_memory;
    const_gen_vect_t const_gens_vect;
    int const_memory;
    const_array_vect_t const_arrays_vect;

    // runtime state
    int block_id;
    // int cur_block;
    bool early_opened_block;
    block_vect_t block_vect;

    func_info_t *func_info;
    int params_lvl;
    params_str_t params_str;
    const uint8_t *cmd_templ_pnt;
    uint8_t cur_cmd;
    // uint8_t past_cmd;
    // int vm_code_offset;
    void (*put_sym)(uint8_t sym);
    ctx_var_info_t ctx_var_info;
    // Expression vars
    int brackets_cnt;
    bool calc_comptime;
    // int stack_size;
    int heap_memory_max;
    int heap_memory_max_back;
    int heap_memory_max_funcs;
    int heap_pnt;
    int heap_pnt_back;
    bytes_vect_t obj_main_vect;
    bytes_vect_t obj_subs_vect;
    bool subs_body;
    bool declare_scope;
} parse_result_t;

#ifdef __cplusplus
}
#endif
#endif