#include "vm_funcs.h"
#include "parser_reg.h"
#include "../shared/parser_func_tokens.h"
#include "parse_exp.h"
#include <string.h>
#include "vm_code.h"

int get_func_info_pnt(parse_result_t *result) {
    int p = 0;
    while (table_fun[p].func_id) {
        MSG_DBG(DL_TRC, "vs func:%s", table_fun[p].text);
        if (strncmp(table_fun[p].text, result->token.data, result->token.size) == 0) {
            return p;
        }
        p++;
    }
    return -1;
}

parse_error_t parse_func_params(parse_result_t *result, int *params_cnt, func_param_type_t func_param_type) {
    parse_error_t pe;
    const_array_info_t *const_array_info;
    (*params_cnt) = 0;
    while (1) {
        switch (CUR_SYM()) {
            case 0:
                return pe_syntax_invalid;
            case ',':
                SKIP_SYM();
                break;
            case ')':
                SKIP_SYM();
                return pe_no_error;
                break;
            default:
        }
        switch (func_param_type) {
            case ft_generic:
                pe = ExpParse(result);
                break;
            case ft_pointer:
                pe = Check_as_array(result, NULL, TRUE, &const_array_info);
                if (pe) {  // lookup among vars arrays
                    result->line_str.line_pnt_ro += result->token.size;
                    ctx_var_info_t ctx_var_info;
                    pe = RegisterVar(result, result->token.data, 0, vsm_GET_ARRAY, &ctx_var_info.var_info);
                    if (pe == pe_no_error) {
                        ctx_var_info.array_auto_size = FALSE;
                        ctx_var_info.array_compile_time_idx = -1;
                        ctx_var_info.is_pointer = TRUE;
                        pe = VmOp_ArgVar(result, &ctx_var_info);
                    }
                }
                break;
            // case ft_byte:
            //     break;
            default:
                pe = pe_syntax_invalid;
        }
        MSG_DBG(DL_DBG1, "token:%s   cur_sym:%c", result->token.data, CUR_SYM());
        if (pe) {
            return pe;
        }
        (*params_cnt)++;
    }

    return pe_syntax_invalid;
}

parse_error_t ParseFunc(parse_result_t *result, char need_return) {
    MSG_DBG(DL_DBG1, "token:%s   cur_sym:%c", result->token.data, CUR_SYM());
    if (CUR_SYM() != '(') {
        return pe_syntax_invalid;
    }
    // Let's look at the user functions first
    func_info_t *func_info = NULL;
    int func_params_cnt = 0;
    int func_id = 0;
    uint8_t func_with_return = 1;
    func_param_type_t func_param_type = ft_generic;
    parse_error_t pe = RegisterSub(result, result->token.data, 0, &func_info);
    if (pe) {  // not found
        // internal funcs
        int func_pnt = get_func_info_pnt(result);
        MSG_DBG(DL_DBG1, "func_id:%d", func_pnt);
        if (func_pnt < 0) {
            MSG_ERR("Function %s not found", result->token.data);
            return pe_object_unknown;
        }
        func_params_cnt = table_fun[func_pnt].params_cnt;
        func_id = table_fun[func_pnt].func_id;
        func_param_type = table_fun[func_pnt].params_type;
        func_with_return = func_id & FUNC_WITH_RETURN_FL;
        pe = pe_no_error;
    } else {
        func_params_cnt = func_info->params_cnt;
        func_id = func_info->func_id;
    }

    SKIP_SYM();  // '('

    if (need_return && !func_with_return) {
        MSG_ERR("Function %s does not return a result", result->token.data);
        return pe_parameters_mismatch;
    }

    int params_cnt;
    pe = parse_func_params(result, &params_cnt, func_param_type);

    if (!pe) {
        MSG_DBG(DL_TRC, "func params cnt real:%d vs func:%d", params_cnt, func_params_cnt);
        if (func_params_cnt != params_cnt) {
            MSG_ERR("function parameters mismatch");
            return pe_parameters_mismatch;
        }
        // vm_ct_lib = vmo_CALL_LIB,
        if (func_info) {
            VmOp_CallPrg(result, func_info);
        } else {
            VmOp_Call(result, vm_ct_int, func_id);
        }
        if (!need_return && func_with_return) {
            VmOp_PopFake(result);
        }
    }
    return pe;
}
