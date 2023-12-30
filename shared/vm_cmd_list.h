
#ifndef VM_CMD_LIST_H_
#define VM_CMD_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    vvt_inline = 0,
    vvt_var_generic_array_pnt = 1 << 4,
    vvt_var_byte_array_pnt = 2 << 4,
    vvt_var_char_array_pnt = 3 << 4,
    vvt_const_generic_array_pnt = 4 << 4,
    vvt_const_byte_array_pnt = 5 << 4,
    vvt_const_string_pnt = 6 << 4,

} vm_var_type_t;

#define VM_CMD_MOD 128

#define VM_CMD_LIST CONV_TYPE(none, 1),                                                                              \
                    CONV_TYPE(DBG, BITS_TO_BYTES(PRG_ADDR_BITS) + 1), /**/                                           \
                    CONV_TYPE(BREAK, BITS_TO_BYTES(PRG_ADDR_BITS) + 1),                                              \
                    CONV_TYPE(END, 1),                                                                               \
                    CONV_TYPE(MUL, 1), /*math and logic ops*/                                                        \
                    CONV_TYPE(DIV, 1),                                                                               \
                    CONV_TYPE(PLUS, 1),                                                                              \
                    CONV_TYPE(MINUS, 1),                                                                             \
                    CONV_TYPE(LE, 1),                                                                                \
                    CONV_TYPE(GE, 1),                                                                                \
                    CONV_TYPE(LT, 1),                                                                                \
                    CONV_TYPE(GT, 1),                                                                                \
                    CONV_TYPE(AND, 1),                                                                               \
                    CONV_TYPE(OR, 1),                                                                                \
                    CONV_TYPE(NOT, 1),                                                                               \
                    CONV_TYPE(INV, 1),                                                                               \
                    CONV_TYPE(EQ, 1),                                                                                \
                    CONV_TYPE(NE, 1),                                                                                \
                    CONV_TYPE(INIT_bVAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1),    /*OP,var addr, size*/            \
                    CONV_TYPE(INIT_gVAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1),    /*OP,var addr, size*/            \
                    CONV_TYPE(JMP, BITS_TO_BYTES(PRG_ADDR_BITS) + 1),               /**/                             \
                    CONV_TYPE(JMP_POP_CMPZ, BITS_TO_BYTES(PRG_ADDR_BITS) + 1),      /**/                             \
                    CONV_TYPE(PUSH_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),        /*Push(offs) byte */             \
                    CONV_TYPE(PUSH_VAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),         /*OP,var addr*/                  \
                    CONV_TYPE(PUSH_NUM, BITS_TO_BYTES(FPT_BITS) + 1),               /*OP, gen num*/                  \
                    CONV_TYPE(PUSH_OBJ_PNT, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1), /*OP, pointer, type of pointer*/ \
                    CONV_TYPE(PUSH_PNT, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),         /*OP, pointer, type of pointer*/ \
                    CONV_TYPE(PUSH_bARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),                                \
                    CONV_TYPE(PUSH_gARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),                                \
                    CONV_TYPE(POP_TO_NOWHERE, 1),                           /**/                                     \
                    CONV_TYPE(POP_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), /**/                                     \
                    CONV_TYPE(POP_VAR, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),                                           \
                    CONV_TYPE(POP_PNT, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),           /*OP, pnt_var addr*/            \
                    CONV_TYPE(POP_bARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), /*OP,var addr*/                 \
                    CONV_TYPE(POP_gARRAY_BY_IDX, BITS_TO_BYTES(VARS_ADDR_BITS) + 1), /*OP,var addr*/                 \
                    CONV_TYPE(LOAD_BYTE, BITS_TO_BYTES(VARS_ADDR_BITS) + 1 + 1),     /*OP,var addr, byte*/           \
                    CONV_TYPE(COPY_TO_bARRAY, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),    /*OP,dest arr addr*/            \
                    CONV_TYPE(COPY_TO_gARRAY, BITS_TO_BYTES(VARS_ADDR_BITS) + 1),    /*OP,dest arr addr*/            \
                    CONV_TYPE(CALL_INT_PROC, 1 + 1),                                                                 \
                    CONV_TYPE(CALL_INT_FUNC, 1 + 1),                            /**/                                 \
                    CONV_TYPE(CALL_PRG_FUNC, BITS_TO_BYTES(PRG_ADDR_BITS) + 1), /**/                                 \
                    CONV_TYPE(CALL_LIB_FUNC, 1 + 1),                            /**/                                 \
                    CONV_TYPE(RETVAL, 1),                                       /**/                                 \
                    CONV_TYPE(RET, 1),                                          /**/

#define CONV_TYPE(cmd, size) vmo_##cmd
typedef enum { VM_CMD_LIST
                   vmo_ENDLIST } vm_ops_list_t;
#undef CONV_TYPE

#ifdef __cplusplus
}
#endif
#endif