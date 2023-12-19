#include "vm_code.h"

#include "parser_priv.h"
#include "parser_reg.h"

#ifdef DEBUG

#define CONV_TYPE(err, size) STR(err)
static const char *vm_ops_str[] = {VM_CMD_LIST};
#undef CONV_TYPE

void put_lst_sym(parse_result_t *result, const char sym) {
    if (result->enable_code_gen) {
        result->put_sym(sym);
    }
}

void put_byte(parse_result_t *result, uint8_t sym) {
    uint8_t *cpnt;
    if (result->subs_body) {
        VECTOR_NEW_RET(result->obj_subs_vect, cpnt);
    } else {
        VECTOR_NEW_RET(result->obj_main_vect, cpnt);
    }
    *cpnt = sym;
}

void put_code_sym(parse_result_t *result, uint8_t sym) {
    if (result->enable_code_gen) {
        put_byte(result, sym);
        // result->vm_code_offset++;
    }
}

void put_code_num(parse_result_t *result, int val, char bytes) {
    if (result->enable_code_gen) {
        while (bytes) {
            put_byte(result, val & 0xff);
            // MSG_DBG(DL_TRC, "%X", val & 0xff);
            // result->vm_code_offset++;
            val >>= 8;
            bytes--;
        }
    }
}

void out_str(parse_result_t *result, const char *str) {
    while (str[0]) {
        put_lst_sym(result, str[0]);
        str++;
    }
    put_lst_sym(result, ' ');
}

void out_num(parse_result_t *result, int num) {
    char buf[100];
    char *str = buf;
    sprintf(buf, "%d ", num);
    while (str[0]) {
        put_lst_sym(result, str[0]);
        str++;
    }
}
#endif

#define CONV_TYPE(cmd, arg_cnt) arg_cnt
static uint8_t vm_ops_argc[] = {VM_CMD_LIST};
#undef CONV_TYPE

parse_error_t out_op(parse_result_t *result, vm_ops_list_t op, char mod) {
    out_str(result, vm_ops_str[op]);
    if (mod) {
        op = op | VM_CMD_MOD;
        out_str(result, "MOD");
    }
    put_code_sym(result, op);
    char buf[10];
    sprintf(buf, "{0x%X}", op);
    out_str(result, buf);
    return pe_no_error;
}
// TODO inc vm_code_offset

parse_error_t to_vm_type(parse_result_t *result, expr_info_t *expr) {
    int val = 1;
    // vm_var_type_t vt = -1;

    switch (expr->type) {
        case pcs_number:
            /* if (fpt_fracpart(expr->value) || fpt2i(expr->value) < MIN_OF(int16_t) || fpt2i(expr->value) > MAX_OF(int16_t)) {
                 // copy value to const block
                 out_str(result, "(CONST)");
                 //      vt = vvt_const_generic;
                 val = 111;  // get const block offs
             } else {
                 out_str(result, "(NUM)");
                 vt = vvt_local;
                 val = fpt2i(expr->value);
             }*/
            out_str(result, "(NUM)");
            // vt = vvt_local;
            val = fpt2i(expr->value);
            // VmOp_ArgNum(result, expr->value);
            break;
        case pcs_variable:
            //   vt = vvt_var_generic;
            val = expr->ctx_var_info.var_info->mem_offs;
            out_str(result, "(VAR:");
            out_str(result, expr->name);
            put_lst_sym(result, ')');
            break;
        case pcs_var_last:
            //   vt = vvt_stack_generic;
            val = 0;
            out_str(result, "(POP_LAST)");
            break;
        case pcs_function:
            val = 0;
            out_str(result, "(FUNC:");
            out_str(result, expr->name);
            put_lst_sym(result, ')');
            break;
        default:
            MSG_ERR("Unknown expr type:%d", expr->type);
            return pe_expression_invalid;
    }
    // out_num(result, vt);
    out_num(result, val);
    return pe_no_error;
}

parse_error_t VmOp_Math(parse_result_t *result, vm_ops_list_t op, expr_info_t *l_expr, expr_info_t *r_expr) {
    MSG_DBG(DL_TRC, "");
    parse_error_t pe = pe_no_error;
    if (r_expr->type <= pcs_none) {
        return pe_expression_invalid;
    }
    out_str(result, "PUSH");
    out_op(result, op, 0);
    int argc = vm_ops_argc[op];
    if (argc > 1) {
        if (l_expr->type <= pcs_none) {
            return pe_expression_invalid;
        }
        pe = to_vm_type(result, l_expr);
        if (!pe) {
            pe = to_vm_type(result, r_expr);
        }
    } else {
        pe = to_vm_type(result, r_expr);
    }

    put_lst_sym(result, '\n');
    // VmOp(result, vmo_PUSH_M);
    return pe;
}

/*
void VmOp_PushRet(parse_result_t *result) {
    out_op(result, vmo_PUSH_RET, 0);
    put_lst_sym(result, '\n');
}

void VmOp_PopRet(parse_result_t *result) {
    out_op(result, vmo_POP_RET, 0);
    put_lst_sym(result, '\n');
}
*/

void VmOp_PopFake(parse_result_t *result) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_POP_TO_NOWHERE, 0);
    put_lst_sym(result, '\n');
}

parse_error_t VmOp_PopVar(parse_result_t *result, ctx_var_info_t *ctx_var_info) {
    MSG_DBG(DL_TRC, "");
    int mem_offs = ctx_var_info->var_info->mem_offs;
    vm_ops_list_t op_code;
    switch (ctx_var_info->var_info->type) {
        case vt_generic:
            op_code = vmo_POP_VAR;
            break;
        case vt_array_of_byte:
            if (result->ctx_var_info.is_pointer) {
                op_code = vmo_COPY_TO_bARRAY;
            } else {
                if (ctx_var_info->array_compile_time_idx >= 0) {
                    if (ctx_var_info->array_compile_time_idx >= ctx_var_info->var_info->elm_count) {
                        return pe_array_index_overrange;
                    }
                    mem_offs += ctx_var_info->array_compile_time_idx + 1;  // skip size byte
                    op_code = vmo_POP_BYTE;
                } else {
                    op_code = vmo_POP_bARRAY_BY_IDX;
                }
            }
            break;
        case vt_array_of_generic:
            if (result->ctx_var_info.is_pointer) {
                op_code = vmo_COPY_TO_gARRAY;
            } else {
                if (ctx_var_info->array_compile_time_idx >= 0) {
                    if (ctx_var_info->array_compile_time_idx >= ctx_var_info->var_info->elm_count) {
                        return pe_array_index_overrange;
                    }
                    mem_offs += ctx_var_info->array_compile_time_idx * BITS_TO_BYTES(FPT_BITS) + 1;  // skip size byte
                    op_code = vmo_POP_VAR;
                } else {
                    op_code = vmo_POP_gARRAY_BY_IDX;
                }
            }
            break;
        default:
            MSG_DBG(DL_DBG, "Wrong var type:%d", ctx_var_info->var_info->type);
            return pe_var_error_parse;
    }

    out_op(result, op_code, ctx_var_info->var_info->subs_body);
    out_str(result, "offs:");
    out_num(result, mem_offs);
    put_code_num(result, mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
    out_str(result, "(");
    out_str(result, ctx_var_info->var_info->name);
    put_lst_sym(result, ')');
    // out_num(result, vvt_var_generic);

    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_ArrayCopy(parse_result_t *result, ctx_var_info_t *to_ctx_var_info) {
    MSG_DBG(DL_TRC, "");
    int mem_offs = to_ctx_var_info->var_info->mem_offs;
    switch (to_ctx_var_info->var_info->type) {
        case vt_array_of_byte:
            out_op(result, vmo_COPY_TO_bARRAY, to_ctx_var_info->var_info->subs_body);
            break;
        case vt_array_of_generic:
            out_op(result, vmo_COPY_TO_gARRAY, to_ctx_var_info->var_info->subs_body);
            break;
        default:
            MSG_DBG(DL_DBG, "Wrong var type:%d", to_ctx_var_info->var_info->type);
            return pe_var_error_parse;
    }

    out_str(result, "offs:");
    out_num(result, mem_offs);
    put_code_num(result, mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
    out_str(result, "(");
    out_str(result, to_ctx_var_info->var_info->name);
    put_lst_sym(result, ')');
    // out_num(result, vvt_var_generic);

    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_Jmp(parse_result_t *result, label_info_t *label_info, char use_condition) {
    MSG_DBG(DL_TRC, "JMP to label:%d  use_condition:%d", label_info->id, use_condition);
    parse_error_t pe = pe_no_error;
    if (use_condition) {
        out_op(result, vmo_JMP_POP_CMPZ, 0);
    } else {
        out_op(result, vmo_JMP, 0);
    }
    out_str(result, "Label:");
    out_num(result, label_info->id);
    label_info->subs_body = result->subs_body;
    if (label_info->jmp_code_offs_cnt >= LINKS_BY_LABEL_MAX) {
        MSG_ERR("You need to increase the value of the variable 'LINKS_BY_LABEL_MAX'");
        return pe_label_invalid;
    }
    label_info->jmp_code_offs_arr[label_info->jmp_code_offs_cnt] = GetCodeOffset(result);
    label_info->jmp_code_offs_cnt++;

    put_code_num(result, label_info->id, BITS_TO_BYTES(PRG_ADDR_BITS));
    out_str(result, "(line");
    out_num(result, label_info->line_num);
    put_lst_sym(result, ')');
    put_lst_sym(result, '\n');
    return pe;
}

void VmOp_Call(parse_result_t *result, vm_call_type type, uint8_t fun_id) {
    MSG_DBG(DL_TRC, "");
    out_op(result, type, 0);
    out_num(result, fun_id);
    put_code_num(result, fun_id, 1);
    put_lst_sym(result, '\n');
}

void VmOp_CallPrg(parse_result_t *result, func_info_t *func_info) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_CALL_PRG_FUNC, 0);
    out_str(result, "offset:");
    out_num(result, func_info->vm_code_offset);
    put_code_num(result, func_info->vm_code_offset, BITS_TO_BYTES(PRG_ADDR_BITS));
    put_lst_sym(result, '\n');
}

parse_error_t VmOp_Return(parse_result_t *result, func_info_t *func_info) {
    MSG_DBG(DL_TRC, "");
    parse_error_t pe = pe_no_error;

    /*if (result->func_info->have_return == 0) {  // ret var in stack
        out_op(result, vmo_PUSH_NUM, 0);
        out_num(result, 0);
        put_code_num(result, 0, BITS_TO_BYTES(FPT_BITS));
        put_lst_sym(result, '\n');
    }*/
    out_op(result, vmo_RET, 0);
    put_lst_sym(result, '\n');
    return pe;
}

void VmOp_Return_Value(parse_result_t *result) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_STORE_RETVAL, 0);
    put_lst_sym(result, '\n');
}

void VmOp_ArgNum(parse_result_t *result, fpt value) {
    MSG_DBG(DL_TRC, "");
    // parse_error_t pe = pe_no_error;
    out_op(result, vmo_PUSH_NUM, 0);
    out_num(result, fpt2i(value));
    put_code_num(result, value, BITS_TO_BYTES(FPT_BITS));
    put_lst_sym(result, '\n');
    // return pe;
}

/*parse_error_t VmOp_ArgPnt(parse_result_t *result, int value) {
    parse_error_t pe = pe_no_error;
    out_op(result, vmo_PUSH_PNT,0);
    out_num(result, value);
    put_code_num(result, value, BITS_TO_BYTES(VARS_ADDR_BITS ));
    put_lst_sym(result, '\n');
    return pe;
}

parse_error_t VmOp_ArgArrIndex(parse_result_t *result, int value) {
    parse_error_t pe = pe_no_error;
    out_op(result, vmo_PUSH_IDX,0);
    out_num(result, value);
    put_code_num(result, value, BITS_TO_BYTES(ARRAY_INDEX_BITS ));
    put_lst_sym(result, '\n');
    return pe;
}*/

parse_error_t VmOp_ArgConstArray(parse_result_t *result, const_array_info_t *const_array_info) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_PUSH_PNT, 0);
    out_str(result, "offs:");
    out_num(result, const_array_info->mem_offs);
    put_code_num(result, const_array_info->mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
    out_str(result, "type:");
    vm_var_type_t type;
    switch (const_array_info->type) {
        case vt_array_of_byte:
            type = vvt_const_array_pnt;
            break;
        case vt_array_of_generic:
            type = vvt_const_generic_pnt;
            break;
        default:
            return pe_syntax_invalid;
    }
    out_num(result, type);
    put_code_num(result, type, 1);
    out_str(result, "(");
    out_str(result, const_array_info->name);
    put_lst_sym(result, ')');
    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_ArgConstInline(parse_result_t *result, unsigned int value) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_PUSH_PNT, 0);
    out_str(result, "inline:");
    out_num(result, value);
    put_code_num(result, value, BITS_TO_BYTES(VARS_ADDR_BITS));
    out_str(result, "type:");
    out_num(result, vvt_inline);
    put_code_num(result, vvt_inline, 1);
    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_ArgVar(parse_result_t *result, ctx_var_info_t *ctx_var_info) {
    MSG_DBG(DL_TRC, "");
    int mem_offs = ctx_var_info->var_info->mem_offs;
    vm_ops_list_t op_code;
    vm_var_type_t type = 0;
    switch (ctx_var_info->var_info->type) {
        case vt_generic:
            op_code = vmo_PUSH_VAR;
            break;
        case vt_array_of_byte:
            type = vvt_var_byte_array_pnt;
            if (ctx_var_info->array_compile_time_idx >= 0) {
                if (ctx_var_info->array_compile_time_idx >= ctx_var_info->var_info->elm_count) {
                    return pe_array_index_overrange;
                }
                mem_offs += ctx_var_info->array_compile_time_idx + 1;  // skip size byte
                op_code = vmo_PUSH_BYTE;
            } else {
                op_code = vmo_PUSH_bARRAY_BY_IDX;
            }
            break;
        case vt_array_of_generic:
            type = vvt_var_generic_array_pnt;
            if (ctx_var_info->array_compile_time_idx >= 0) {
                if (ctx_var_info->array_compile_time_idx >= ctx_var_info->var_info->elm_count) {
                    return pe_array_index_overrange;
                }
                mem_offs += ctx_var_info->array_compile_time_idx * BITS_TO_BYTES(FPT_BITS) + 1;  // skip size byte
                op_code = vmo_PUSH_VAR;
            } else {
                op_code = vmo_PUSH_gARRAY_BY_IDX;
            }
            break;
        default:
            MSG_DBG(DL_DBG, "Wrong var type:%d", ctx_var_info->var_info->type);
            return pe_var_error_parse;
    }
    if (ctx_var_info->is_pointer) {
        op_code = vmo_PUSH_PNT;
    } else {
        type = 0;
    }
    out_op(result, op_code, ctx_var_info->var_info->subs_body);
    out_str(result, "offs:");
    out_num(result, mem_offs);
    put_code_num(result, mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
    out_str(result, "(");
    out_str(result, ctx_var_info->var_info->name);
    put_lst_sym(result, ')');
    if (type) {
        out_str(result, " type:");
        out_num(result, type);
        put_code_num(result, type, 1);
    }
    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_LoadByte(parse_result_t *result, var_info_t *var_info, uint8_t byte) {
    MSG_DBG(DL_TRC, "");
    out_op(result, vmo_LOAD_BYTE, var_info->subs_body);
    out_str(result, "offs:");
    out_num(result, var_info->mem_offs);
    put_code_num(result, var_info->mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
    put_lst_sym(result, '(');
    out_str(result, var_info->name);
    out_str(result, "), byte:");
    out_num(result, byte);
    put_code_num(result, byte, 1);
    put_lst_sym(result, '\n');
    return pe_no_error;
}

parse_error_t VmOp_VarHeap(parse_result_t *result, var_info_t *var_info, char alloc) {
    MSG_DBG(DL_TRC, "alloc:%d", alloc);
    if (alloc) {
        out_op(result, vmo_ALLOC, var_info->subs_body);
        var_info->mem_offs = result->heap_pnt;
        result->heap_pnt += var_info->mem_size;
        if (result->heap_pnt > result->heap_memory_max) {
            result->heap_memory_max = result->heap_pnt;
        }
    } else {
        out_op(result, vmo_FREE, var_info->subs_body);
        var_info->mem_offs = result->heap_pnt;
        result->heap_pnt -= var_info->mem_size;
        if (result->heap_pnt < 0) {
            return pe_stack_corrupted;
        }
    }
    put_code_num(result, var_info->mem_size, 1);
    out_str(result, "offs:");
    out_num(result, var_info->mem_offs);
    out_str(result, "size:");
    out_num(result, var_info->mem_size);
    // put_code_num(result, var_info->mem_offs, BITS_TO_BYTES(VARS_ADDR_BITS ));
    out_str(result, "(");
    out_str(result, var_info->name);
    put_lst_sym(result, ')');
    put_lst_sym(result, '\n');
    if (alloc) {
        switch (var_info->type) {
            case vt_generic:
                break;
            case vt_array_of_byte:
            case vt_array_of_generic:
                VmOp_LoadByte(result, var_info, var_info->elm_count);  // add array size
                break;
            default:
                MSG_DBG(DL_DBG, "Wrong var type:%d", var_info->type);
                return pe_var_error_parse;
        }
    }
    return pe_no_error;
}

void VmOp_BreakPoint(parse_result_t *result) {
    out_op(result, vmo_BREAK, 0);
    put_lst_sym(result, '#');
    out_num(result, result->line_str.line_num);
    put_code_num(result, result->line_str.line_num, BITS_TO_BYTES(PRG_ADDR_BITS));
    put_lst_sym(result, '\n');
}

void VmOp_DebugNewLine(parse_result_t *result) {
    out_op(result, vmo_DBG, 0);
    put_lst_sym(result, '#');
    out_num(result, result->line_str.line_num);
    put_code_num(result, result->line_str.line_num, BITS_TO_BYTES(PRG_ADDR_BITS));
    out_str(result, "NewLine, code offs:");
    out_num(result, GetCodeOffset(result));
    put_lst_sym(result, '\n');
}

void VmOp_Debug(parse_result_t *result, const char *str, const char *str1) {
    out_str(result, vm_ops_str[vmo_DBG]);
    put_lst_sym(result, '#');
    out_num(result, result->line_str.line_num);
    if (str) out_str(result, str);
    if (str1) out_str(result, str1);
    out_str(result, ", code offs:");
    out_num(result, GetCodeOffset(result));
    put_lst_sym(result, '\n');
}

void VmOp_Debug1(parse_result_t *result, const char *str, int num) {
    out_str(result, vm_ops_str[vmo_DBG]);
    put_lst_sym(result, '#');
    out_num(result, result->line_str.line_num);
    if (str) out_str(result, str);
    out_num(result, num);
    out_str(result, ", code offs:");
    out_num(result, GetCodeOffset(result));
    put_lst_sym(result, '\n');
}

parse_error_t VmOp_Spc_Cmds(parse_result_t *result) {
    MSG_DBG(DL_TRC, "");
    parse_error_t pe = pe_no_error;

    uint8_t params_cnt = result->params_str.params_cnt;
    switch (result->cur_cmd) {
        case cmd_id_PRINT_LN:
        case cmd_id_PRINT:
            MSG_DBG(DL_DBG1, "params_cnt:%d\n", params_cnt);
            if (DL_DBG1 <= DEBUG_LVL) {
                for (int i = 0; i < params_cnt; i++) {
                    printf("%d ", result->params_str.params_type[i]);
                }
                printf("\n");
            }
            if (params_cnt <= BITS_TO_BYTES(VARS_ADDR_BITS) * 2) {
                unsigned int value = 0;
                for (int i = 0; i < params_cnt; i++) {
                    value <<= 4;
                    value |= result->params_str.params_type[i];
                }
                if (params_cnt & 1) {
                    value <<= 4;
                }
                MSG_DBG(DL_TRC, "value:0x%X", value);
                VmOp_ArgConstInline(result, value);
            } else {
                uint8_t params_type[FUNC_MAX_PARAMS];
                int p = 0;
                for (int i = 0; i < params_cnt; i++) {
                    if (i & 1) {
                        params_type[p] = (params_type[p] << 4) | result->params_str.params_type[i];
                        p++;
                    } else {
                        params_type[p] = result->params_str.params_type[i];
                    }
                }
                if (params_cnt & 1) {
                    params_type[p] <<= 4;
                }
                p = (params_cnt + 1) / 2;
                MSG_DBG(DL_TRC, "p:%d", p);
                // params_type[p] = 0;
                const_array_info_t *const_array_info;
                pe = RegisterConstArray(result, NULL, params_type, p, TRUE, vt_array_of_byte, &const_array_info);
                if (pe) return pe;
                VmOp_ArgConstArray(result, const_array_info);
            }
            break;
        /*case cmd_id_CLS:
            break;
        case cmd_id_SET_CURS:
            break;
        case cmd_id_BEEP:
            break;
        case cmd_id_OUT:
            break;*/
        default:
            return pe_no_error;
    }
    VmOp_Call(result, vm_ct_proc, result->cur_cmd);
    return pe;
}

void VmOp_End(parse_result_t *result) {
    out_op(result, vmo_END, 0);
}