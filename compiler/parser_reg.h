#ifndef PARSE_REG_H_
#define PARSE_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parser_pub.h"
#include "parser_priv.h"

parse_error_t RegisterLabel(parse_result_t *result, const char *token_str, bool new_object, label_info_t **label_info);
parse_error_t RegisterVar(parse_result_t *result, const char *token_str, int array_size, var_set_mode_t mode, var_info_t **var_info);
parse_error_t FixArraySize(parse_result_t *result, int array_size, var_info_t *var_info);
parse_error_t UnregisterLastBlockVars(parse_result_t *result);
parse_error_t RegisterSub(parse_result_t *result, const char *token_str, bool new_object, func_info_t **func_info);
parse_error_t RegisterConstArray(parse_result_t *result, const char *token_str, void *data, uint8_t size, var_type_t type, bool new_object, const_array_info_t **const_array_info);
parse_error_t RegisterConstGenVar(parse_result_t *result, const char *token_str, fpt data, bool new_object, const_gen_info_t **const_gen_info);
void EndSubContext(parse_result_t *result);
int GetCodeOffset(parse_result_t *result);

void UpperCaseStr(char *str);
char UpperCaseChar(char ch);
char ToNextSym(parse_result_t *result, char skip_spaces);
char GetCurSymSkipSpaces(parse_result_t *result);
#define GET_CUR_SKIP_SPACES() GetCurSymSkipSpaces(result)
#define SKIP_SYM() ToNextSym(result, 0)
#define CUR_SYM() result->line_str.data[result->line_str.line_pnt_ro]
#define TO_END_LINE() result->line_str.line_pnt_ro = result->line_str.line_len - 1
// #define EOL_FLAG() result->line_str.new_line_fl
#define SYM_ROLLBACK() result->line_str.line_pnt_ro--

#define LINE_PNT_STORE() int line_pnt_back = result->line_str.line_pnt_ro
//    VECTOR_NEW(result->line_pnts_vect);
//    VECTOR_LAST(result->line_pnts_vect) = result->line_str.line_pnt_ro
#define LINE_PNT_RESTORE() result->line_str.line_pnt_ro = line_pnt_back
//    result->line_str.line_pnt_ro = VECTOR_LAST(result->line_pnts_vect);
//   VECTOR_POP(result->line_pnts_vect)
// #define LINE_PNT_POP() VECTOR_POP(result->line_pnts_vect)

// #define CODE_GEN_ENABLE() result->enable_code_gen = 1
// #define CODE_GEN_DISABLE() result->enable_code_gen = 0
#define CODE_GEN_BACKUP_AND_DISABLE()               \
    char enable_code_gen = result->enable_code_gen; \
    result->enable_code_gen = 0;

#define CODE_GEN_RESTORE() result->enable_code_gen = enable_code_gen

// void CopyNameFromToken(parse_result_t *result, char *to);
#define CopyNameFromToken(to) strncpy(to, result->token.data, result->token.size);
void Print_fpt(fpt A);
parse_error_t ParseNum(parse_result_t *result, expr_info_t *expr_cur);
void AutoName(parse_result_t *result, char *str, int size_max, char *suf);

void DefinePartArgType(parse_result_t *result, print_part_type_t part_type);

#ifdef __cplusplus
}
#endif
#endif