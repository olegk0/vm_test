#ifndef PARSE_CMD_H_
#define PARSE_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CMD_SIZE_MAX 8
#define TEMPL_SIZE_MAX 9
struct commands {
    char text[CMD_SIZE_MAX];
    uint8_t cmd_id;
    uint8_t expr_flags;
    uint8_t templ[TEMPL_SIZE_MAX];
};

const struct commands table_cmd[] = {
    {"PRINT", cmd_id_PRINT, efl_ALL, {'(', pcs_expression, pcs_repeated, ')', 0}},
    {"PRINT_LN", cmd_id_PRINT_LN, efl_ALL, {'(', pcs_expression, pcs_repeated, ')', 0}},
    {"CONST", cmd_id_CONST, efl_inline_const_string | efl_inline_const_array | efl_compile_calc_expr, {pcs_token, pcs_expression, 0}},
    {"RETURN", cmd_id_RETURN, efl_any_expr, {pcs_expression, 0}},
    {"IF", cmd_id_IF, efl_any_expr, {pcs_expression, '{', 0}},  //
    {"GOTO", cmd_id_GOTO, 0, {pcs_token, 0}},
    {"WHILE", cmd_id_WHILE, efl_any_expr, {pcs_expression, '{', 0}},
    {"BREAK", cmd_id_BREAK, 0, {0}},
    {"BYTE", cmd_id_BYTE, efl_inline_const_array, {pcs_token, pcs_IND_optional, '=' | pcs_MOD_group, pcs_expression | pcs_MOD_group, 0}},
    {"CHAR", cmd_id_CHAR, efl_inline_const_string, {pcs_token, pcs_IND_optional, '=' | pcs_MOD_group, pcs_expression | pcs_MOD_group, 0}},
    {"VAR", cmd_id_VAR, efl_inline_const_array | efl_any_expr, {pcs_token, pcs_IND_optional, '=' | pcs_MOD_group, pcs_expression | pcs_MOD_group, 0}},
    {"STOP", cmd_id_STOP, 0, {0}},
    {"BRKPNT", cmd_id_BRKPNT, 0, {0}},
    {"FUNC", cmd_id_PROC, 0, {pcs_token, '(', pcs_IND_optional, pcs_variable | pcs_MOD_group, pcs_token | pcs_MOD_group, pcs_repeated, ')', '{', 0}},
    //{"LOAD", cmd_id_LOAD, {pcs_string, 0}},
    {"", 0, 0, {0}}};

const uint8_t curly_brace_begin_templ[TEMPL_SIZE_MAX] = {0};
const uint8_t curly_brace_end_templ[TEMPL_SIZE_MAX] = {pcs_IND_optional, pcs_else, 0};

// #define CMD_NOP_P 0
#define TABLE_CMD_EXT(cmd_ext_id) table_cmd_ext[cmd_ext_id - cmd_ext_pre_begin - 1]
#define CONV_TYPE(id, expr_flags, ...) \
    { STR(id), cmd_ext_id_##id, expr_flags, __VA_ARGS__ }
const struct commands table_cmd_ext[] = {EXT_OPER_LIST};
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif