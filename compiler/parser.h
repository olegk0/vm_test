#ifndef PARSE_H_
#define PARSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "parser_pub.h"

int ParseInit(parse_result_t *ppr);
void ParseFree();

parse_error_t ParseLine(parse_result_t *ppr, int line_num, char *line, int line_len);

#ifdef __cplusplus
}
#endif
#endif