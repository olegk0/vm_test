#ifndef PARSE_CMD_H_
#define PARSE_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CMD_SIZE_MAX 8
#define TEMPL_SIZE_MAX 8
struct commands {
    char text[CMD_SIZE_MAX];
    uint8_t cmd_id;
    uint8_t templ[TEMPL_SIZE_MAX];
};

const struct commands table_cmd[] = {
    {"PRINT", cmd_id_PRINT, {'(', pcs_multi_type, pcs_repeated, ')', 0}},
    {"PRINT_LN", cmd_id_PRINT_LN, {'(', pcs_multi_type, pcs_repeated, ')', 0}},
    //{"CURS", cmd_id_SET_CURS, {'(', pcs_expression, ',', pcs_expression, ')', 0}},
    //{"BEEP", cmd_id_BEEP, {'(', pcs_expression, ',', pcs_expression, ')', 0}},
    //{"OUT", cmd_id_OUT, {'(', pcs_expression, ',', pcs_expression, ')', 0}},
    //{"CLS", cmd_id_CLS, {'(', ')', 0}},
    {"IF", cmd_id_IF, {pcs_expression, '{', 0}},  //
    {"GOTO", cmd_id_GOTO, {pcs_token, 0}},
    {"WHILE", cmd_id_WHILE, {pcs_expression, '{', 0}},
    {"BREAK", cmd_id_BREAK, {0}},
    {"RETURN", cmd_id_RETURN, {pcs_expression, 0}},
    {"BYTE", cmd_id_BYTE, {pcs_variable, 0}},
    {"VAR", cmd_id_VAR, {pcs_variable, '=' | pcs_MOD_optional, pcs_expression | pcs_MOD_optional, 0}},
    {"STOP", cmd_id_STOP, {0}},
    {"BRKPNT", cmd_id_BRKPNT, {0}},
    {"FUNC", cmd_id_PROC, {pcs_token, '(', pcs_variable | pcs_MOD_optional, pcs_repeated, ')', '{', 0}},
    //{"LOAD", cmd_id_LOAD, {pcs_string, 0}},
    {"CONST", cmd_id_CONST, {pcs_token, pcs_multi_type, 0}},
    {"", 0, {0}}};

const uint8_t curly_brace_begin_templ[TEMPL_SIZE_MAX] = {0};
const uint8_t curly_brace_end_templ[TEMPL_SIZE_MAX] = {pcs_else | pcs_MOD_optional, 0};

// #define CMD_NOP_P 0
#define TABLE_CMD_EXT(cmd_ext_id) table_cmd_ext[cmd_ext_id - cmd_ext_pre_begin - 1]
#define CONV_TYPE(id, ...) \
    { STR(id), cmd_ext_id_##id, __VA_ARGS__ }
const struct commands table_cmd_ext[] = {EXT_OPER_LIST};
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif