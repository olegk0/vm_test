#ifndef PARSE_CMD_H_
#define PARSE_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vm_config.h"

#define FUNC_WITH_RET_LIST CONV_TYPE(IN, {ft_generic, 0}),                  /*Read data from port. Params: port number*/            \
                           CONV_TYPE(RAND, {ft_generic, 0}),                /*Get random number, from 0 to top. Params: top*/       \
                           CONV_TYPE(INKEY, {ft_generic, 0}),               /*Get keypress. Params: wait (sec), (<=0) for no wait*/ \
                           CONV_TYPE(SIZE, {ft_pointer, 0}),                /*Get array (string) size. Params: pointer to array*/   \
                           CONV_TYPE(SCR_W, {ft_none, 0}),                  /*Get screen width.*/                                   \
                           CONV_TYPE(SCR_H, {ft_none, 0}),                  /*Get screen height.*/                                  \
                           CONV_TYPE(SCR_SYM, {ft_generic, ft_generic, 0}), /*Read symbol on screen at position. Params: x, y*/

#define FUNC_NO_RET_LIST CONV_TYPE(PUTC, {ft_generic, 0}),             /*Print char. Params: char*/                      \
                         CONV_TYPE(PUTS, {ft_pointer, 0}),             /*Print . Params: pointer to array*/              \
                         CONV_TYPE(PUTN, {ft_generic, 0}),             /*Print number. Params: number*/                  \
                         CONV_TYPE(PUTI, {ft_generic, 0}),             /*Print integer part of number. Params: number*/  \
                         CONV_TYPE(SLEEP_MS, {ft_generic, 0}),         /*Sleep mS. Params: mS*/                          \
                         CONV_TYPE(BEEP, {ft_generic, ft_generic, 0}), /*Beep. Params: length, frequency*/               \
                         CONV_TYPE(OUT, {ft_generic, ft_generic, 0}),  /*Write data to port. Params: port number, data*/ \
                         CONV_TYPE(CLS, {ft_none, 0}),                 /*Clear of screen*/                               \
                         CONV_TYPE(CURS, {ft_generic, 0}),             /*Set cursor position. Params: x, y*/

#define CONV_TYPE(id, ...) func_##id
typedef enum { func_id_begin,
               FUNC_NO_RET_LIST
                   func_with_ret_begin = FUNC_WITH_RETURN_FL - 1,
               FUNC_WITH_RET_LIST
} func_id_list_t;
#undef CONV_TYPE

#define CMD_SIZE_MAX 8
struct functions {
    char text[CMD_SIZE_MAX];
    uint8_t func_id;
    uint8_t params[FUNC_MAX_PARAMS + 1];
};

#define CONV_TYPE(id, ...) \
    { STR(id), func_##id, __VA_ARGS__ }
const struct functions table_fun[] = {FUNC_NO_RET_LIST FUNC_WITH_RET_LIST{"", 0, {ft_none}}};
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif