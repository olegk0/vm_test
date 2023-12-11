#ifndef PARSE_EXP_H_
#define PARSE_EXP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parser_gen.h"

parse_error_t ExpParse(parse_result_t *result);
parse_error_t ExpParseCompileTime(parse_result_t *result, fpt *val);

#ifdef __cplusplus
}
#endif
#endif