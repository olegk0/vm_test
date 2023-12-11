
#ifndef VM_CMD_LIST_H_
#define VM_CMD_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    vvt_inline,
    // vvt_var_generic_pnt,
    // vvt_const_generic_pnt,
    vvt_const_array_pnt,
    //  vvt_const_string_pnt,

} vm_var_type_t;

#define VM_CMD_MOD 128

#define VM_CMD_LIST CONV_TYPE(none, 1),                                               \
                    CONV_TYPE(DBG, BITS_TO_BYTES(PRG_ADDR_BITS) + 1), /**/            \
                    CONV_TYPE(BREAK, BITS_TO_BYTES(PRG_ADDR_BITS) + 1),               \
                    CONV_TYPE(END, 1),                                                \
                    CONV_TYPE(MUL, 1), /*math and logic ops*/                         \
                    CONV_TYPE(DIV, 1),                                                \
                    CONV_TYPE(PLUS, 1),                                               \
                    CONV_TYPE(MINUS, 1),                                              \
                    CONV_TYPE(LE, 1),                                                 \
                    CONV_TYPE(GE, 1),                                                 \
                    CONV_TYPE(LT, 1),                                                 \
                    CONV_TYPE(GT, 1),                                                 \
                    CONV_TYPE(AND, 1),                                                \
                    CONV_TYPE(OR, 1),                                                 \
                    CONV_TYPE(NOT, 1),                                                \
                    CONV_TYPE(INV, 1),                                                \
                    CONV_TYPE(EQ, 1),                                                 \
                    CONV_TYPE(NE, 1),                                                 \
                    CONV_TYPE(ALLOC, 2),                                       /**/   \
                    CONV_TYPE(FREE, 2),                                        /**/   \
                    CONV_TYPE(JMP, BITS_TO_BYTES(PRG_ADDR_BITS) + 1),          /**/   \
                    CONV_TYPE(JMP_POP_CMPZ, BITS_TO_BYTES(PRG_ADDR_BITS) + 1), /**/   \
                    CONV_TYPE(PUSH_NUM, BITS_TO_BYTES(FPT_BITS) + 1),          /**/   \
                    CONV_TYPE(PUSH_PNT, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1),       \
                    CONV_TYPE(PUSH_VAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),  /**/     \
                    CONV_TYPE(PUSH_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), /**/     \
                    CONV_TYPE(POP_TO_NOWHERE, 1),                            /**/     \
                    CONV_TYPE(POP_VAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),            \
                    CONV_TYPE(POP_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), /**/      \
                    CONV_TYPE(POP_bARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),  \
                    CONV_TYPE(PUSH_bARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), \
                    CONV_TYPE(LOAD_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1),      \
                    CONV_TYPE(CALL_INT_PROC, 1 + 1),                                  \
                    CONV_TYPE(CALL_INT_FUNC, 1 + 1),                            /**/  \
                    CONV_TYPE(CALL_PRG_FUNC, BITS_TO_BYTES(PRG_ADDR_BITS) + 1), /**/  \
                    CONV_TYPE(CALL_LIB_FUNC, 1 + 1),                            /**/  \
                    CONV_TYPE(RET, 1),                                          /*[SP+1]->t, [SP]->[SP+1], POP, t->CodePointer*/

#define CONV_TYPE(cmd, size) vmo_##cmd
typedef enum { VM_CMD_LIST
                   vmo_ENDLIST } vm_ops_list_t;
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif