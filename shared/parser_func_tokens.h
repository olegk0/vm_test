#ifndef PARSE_CMD_H_
#define PARSE_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ft_none,
    // ft_byte,
    ft_generic,
    ft_pointer,
} func_param_type_t;

#define FUNC_WITH_RET_LIST CONV_TYPE(IN, 1, ft_generic),      /*Read data from port. Params: port number*/            \
                           CONV_TYPE(RAND, 1, ft_generic),    /*Get random number, from 0 to top. Params: top*/       \
                           CONV_TYPE(INKEY, 1, ft_generic),   /*Get keypress. Params: wait (sec), (<=0) for no wait*/ \
                           CONV_TYPE(SIZE, 1, ft_pointer),    /*Get array (string) size. Params: pointer to array*/   \
                           CONV_TYPE(SCR_W, 0, ft_none),      /*Get screen width.*/                                   \
                           CONV_TYPE(SCR_H, 0, ft_none),      /*Get screen height.*/                                  \
                           CONV_TYPE(SCR_SYM, 2, ft_generic), /*Read symbol on screen at position. Params: x, y*/

#define FUNC_NO_RET_LIST CONV_TYPE(PUTC, 1, ft_generic),     /*Print char. Params: char*/                      \
                         CONV_TYPE(PUTS, 1, ft_pointer),     /*Print . Params: pointer to array*/              \
                         CONV_TYPE(PUTN, 1, ft_generic),     /*Print number. Params: number*/                  \
                         CONV_TYPE(PUTI, 1, ft_generic),     /*Print integer part of number. Params: number*/  \
                         CONV_TYPE(SLEEP_MS, 1, ft_generic), /*Sleep mS. Params: mS*/                          \
                         CONV_TYPE(BEEP, 2, ft_generic),     /*Beep. Params: length, frequency*/               \
                         CONV_TYPE(OUT, 2, ft_generic),      /*Write data to port. Params: port number, data*/ \
                         CONV_TYPE(CLS, 0, ft_none),         /*Clear of screen*/                               \
                         CONV_TYPE(CURS, 2, ft_generic),     /*Set cursor position. Params: x, y*/

#define CONV_TYPE(id, params_cnt, params_type) func_##id
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
    uint8_t params_cnt;
    uint8_t params_type;
};

#define CONV_TYPE(id, params_cnt, params_type) \
    { STR(id), func_##id, params_cnt, params_type }
const struct functions table_fun[] = {FUNC_NO_RET_LIST FUNC_WITH_RET_LIST{"", 0, 0, ft_none}};
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif