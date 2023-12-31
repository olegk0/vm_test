#ifndef PARSE_GEN_H_
#define PARSE_GEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parser_pub.h"
#include "parser_priv.h"

parse_error_t PrintError(parse_result_t *result, parse_error_t error);

parse_error_t GetToken(parse_result_t *result);
#define TOKEN_INVALIDATE() result->token.token_valid = 0
#define TOKEN_VALID() result->token.token_valid

const char *GetParseErrorStr(parse_error_t error);

// parse_error_t Parse_string(parse_result_t *result, const char *token_str, const_array_info_t **const_array_info);
// parse_error_t Parse_array(parse_result_t *result, const char *token_str, const_array_info_t **const_array_info);
parse_error_t Check_as_array(parse_result_t *result, uint8_t expr_flags, const_array_info_t **const_array_info);

parse_error_t ParseStep(parse_result_t *result, uint8_t step);

parse_error_t ParseVar(parse_result_t *result, var_set_mode_t mode, ctx_var_info_t *ctx_var_info);

#ifdef __cplusplus
}
#endif
#endif