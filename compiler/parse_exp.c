#include "parse_exp.h"
#include "parser_reg.h"
#include "parser_gen.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm_code.h"
#include "vm_funcs.h"

// #define DEBUG 1

#ifdef DEBUG

#define CONV_TYPE(err, arg_cnt) STR(err)
static const char *math_ops_str[] = {VM_CMD_LIST};
#undef CONV_TYPE

#endif

parse_error_t expLevel_0(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expLevel_1(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expLevel_2(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expLevel_3(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expLevel_4(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expLevel_last(parse_result_t *result, expr_info_t *expr_cur);
parse_error_t expPartParse(parse_result_t *result, expr_info_t *expr_cur);

parse_error_t register_expr(parse_result_t *result, vm_ops_list_t ops, expr_info_t *l_expr) {
    if (l_expr->calc_comptime && ops && l_expr->type != pcs_number) {
        return pe_error_eval_exp;
    }
    expr_info_t r_expr;
    r_expr.calc_comptime = l_expr->calc_comptime;
    r_expr.brackets_cnt = l_expr->brackets_cnt;

    parse_error_t pe = expPartParse(result, &r_expr);
    if (pe == pe_no_error) {
        if (ops) {
            if (l_expr->type == pcs_number && r_expr.type == pcs_number) {
                switch (ops) {
                    case vmo_MUL:
                        l_expr->value = fpt_mul(l_expr->value, r_expr.value);
                        break;
                    case vmo_DIV:
                        l_expr->value = fpt_div(l_expr->value, r_expr.value);
                        break;
                    case vmo_PLUS:
                        l_expr->value = fpt_add(l_expr->value, r_expr.value);
                        break;
                    case vmo_MINUS:
                        l_expr->value = fpt_sub(l_expr->value, r_expr.value);
                        break;
                    case vmo_LE:
                        l_expr->value = l_expr->value <= r_expr.value;
                        break;
                    case vmo_GE:
                        l_expr->value = l_expr->value >= r_expr.value;
                        break;
                    case vmo_LT:
                        l_expr->value = l_expr->value < r_expr.value;
                        break;
                    case vmo_GT:
                        l_expr->value = l_expr->value > r_expr.value;
                        break;
                    case vmo_EQ:
                        l_expr->value = l_expr->value == r_expr.value;
                        break;
                    case vmo_NE:
                        l_expr->value = l_expr->value != r_expr.value;
                        break;
                    case vmo_AND:
                        l_expr->value = l_expr->value && r_expr.value;
                        break;
                    case vmo_OR:
                        l_expr->value = l_expr->value || r_expr.value;
                        break;
                        //
                    case vmo_NOT:
                        l_expr->value = !r_expr.value;
                        break;
                    case vmo_INV:
                        l_expr->value = fpt_mul(r_expr.value, FPT_MINUS_ONE);
                        break;
                    default:
                }
            } else {
                if (l_expr->calc_comptime) {
                    return pe_error_eval_exp;
                }
                pe = VmOp_Math(result, ops, l_expr, &r_expr);
#ifdef DEBUG
                MSG_DBG(DL_DBG, "Register EXP: %s (%s(%d):%d, %s(%d):%d) ", math_ops_str[ops], l_expr->name, l_expr->type, fpt2i(l_expr->value), r_expr.name, r_expr.type, fpt2i(r_expr.value));
                strcpy(l_expr->name, "LAST_VAL");
#endif

                l_expr->type = pcs_var_last;
            }
        } else {
            memcpy(l_expr, &r_expr, sizeof(r_expr));
        }
        if (pe == pe_no_error) {
            if (GET_CUR_SKIP_SPACES() != 0) {
                pe = expLevel_last(result, l_expr);
            }
        }
    }
    MSG_DBG(DL_TRC, "<pe:%d   >%c<(%d)", pe, CUR_SYM(), CUR_SYM());
    return pe;
}

/*
test for
* number
* var
* var[]
* function()
*/
parse_type_t parse_exp_token(parse_result_t *result) {
    parse_error_t pe = GetToken(result);
    if (pe == pe_no_error) {
        // MSG_DBG(DL_DBG, "token: %s  token_len:%d\n", result->token.data, result->token.size);
        MSG_DBG(DL_DBG1, "next_sym >%c<  at  line_pnt:%d", CUR_SYM(), result->line_str.line_pnt_ro);
        if (result->token.size > 0) {
            // MSG_DBG(DL_DBG1, "next_sym >%c<  at  line_pnt:%d", CUR_SYM(), result->line_str.line_pnt_ro);
            if (CUR_SYM() == '(') {  // this is probably the func
                return pcs_function;
            }
            // if (next_sym == '[')
            {  // this is probably the array
                return pcs_variable;
            }
        } else if (CUR_SYM() == '\'') {
            return pcs_byte;
        }
    } else if (pe == pe_token_start_with_num) {  // this is probably the number
        return pcs_number;
    }
    return pcs_expression;
}

/* recursive descent parser for arithmetic/logical expressions */
parse_error_t ExpParse(parse_result_t *result) {
    MSG_DBG(DL_DBG1, "enable_code_gen:%d", result->enable_code_gen);
    expr_info_t expr_cur;
    int brackets_cnt = 0;
    expr_cur.brackets_cnt = &brackets_cnt;
    expr_cur.type = 0;
    expr_cur.value = 0;
    expr_cur.calc_comptime = FALSE;
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = register_expr(result, 0, &expr_cur);
    if (pe == pe_no_error) {
        if (expr_cur.type == pcs_number) {
            // MSG_DBG(DL_DBG, " VAL:");
            Print_fpt(expr_cur.value);
        }
    }
    result->calc_comptime = FALSE;
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    TOKEN_INVALIDATE();
    return pe;
}

parse_error_t ExpParseCompileTime(parse_result_t *result, fpt *val) {
    CODE_GEN_BACKUP_AND_DISABLE();
    MSG_DBG(DL_DBG1, "");
    expr_info_t expr_cur;
    int brackets_cnt = 0;
    expr_cur.brackets_cnt = &brackets_cnt;
    expr_cur.type = 0;
    expr_cur.value = 0;
    expr_cur.calc_comptime = TRUE;
    parse_error_t pe = register_expr(result, 0, &expr_cur);
    if (pe == pe_no_error) {
        if (expr_cur.type != pcs_number) {
            MSG_DBG(DL_DBG, "Unable to evaluate expression at compile time");
            pe = pe_error_eval_exp;
        } else {
            result->calc_comptime = TRUE;
            *val = expr_cur.value;
            // MSG_DBG(DL_DBG, " VAL:%d", *val);
            Print_fpt(expr_cur.value);
        }
    }
    CODE_GEN_RESTORE();
    TOKEN_INVALIDATE();
    return pe;
}

parse_error_t expLevel_0(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    SKIP_SYM();
    if (sym == '=' && CUR_SYM() == '=') {  // ==
        SKIP_SYM();
        pe = register_expr(result, vmo_EQ, expr_cur);
    } else if (sym == '!' && CUR_SYM() == '=') {  // !=
        SKIP_SYM();
        pe = register_expr(result, vmo_NE, expr_cur);
    } else {
        SYM_ROLLBACK();
    }
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expLevel_1(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    SKIP_SYM();
    if (sym == '&' && CUR_SYM() == '&') {  // &&
        SKIP_SYM();
        pe = register_expr(result, vmo_AND, expr_cur);
    } else if (sym == '|' && CUR_SYM() == '|') {  // ||
        SKIP_SYM();
        pe = register_expr(result, vmo_OR, expr_cur);
    } else {
        SYM_ROLLBACK();
        pe = expLevel_0(result, expr_cur);
    }
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expLevel_2(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    switch (sym) {
        case '<':
            SKIP_SYM();
            pe = register_expr(result, vmo_LT, expr_cur);
            break;
        case '>':
            SKIP_SYM();
            pe = register_expr(result, vmo_GT, expr_cur);
            break;
        default:
            pe = expLevel_1(result, expr_cur);
    }
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expLevel_3(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    SKIP_SYM();
    if (sym == '<' && CUR_SYM() == '=') {  // <=
        SKIP_SYM();
        pe = register_expr(result, vmo_LE, expr_cur);
    } else if (sym == '>' && CUR_SYM() == '=') {  // >=
        SKIP_SYM();
        pe = register_expr(result, vmo_GE, expr_cur);
    } else {
        SYM_ROLLBACK();
        pe = expLevel_2(result, expr_cur);
    }

    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expLevel_4(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    switch (sym) {
        case '+':
            SKIP_SYM();
            pe = register_expr(result, vmo_PLUS, expr_cur);
            break;
        case '-':
            SKIP_SYM();
            pe = register_expr(result, vmo_MINUS, expr_cur);
            break;
        default:
            pe = expLevel_3(result, expr_cur);
    }
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expLevel_last(parse_result_t *result, expr_info_t *expr_cur) {
    MSG_DBG(DL_TRC, ">line_pnt:%d", result->line_str.line_pnt_ro);
    parse_error_t pe = pe_no_error;
    uint8_t sym = CUR_SYM();
    MSG_DBG(DL_TRC, "=line_pnt:%d  >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);
    switch (sym) {
        case '*':
            SKIP_SYM();
            pe = register_expr(result, vmo_MUL, expr_cur);
            break;
        case '/':
            SKIP_SYM();
            pe = register_expr(result, vmo_DIV, expr_cur);
            break;
        default:
            pe = expLevel_4(result, expr_cur);
    }
    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}

parse_error_t expPartParse(parse_result_t *result, expr_info_t *expr_cur) {
    parse_error_t pe = pe_no_error;

    uint8_t sym = GET_CUR_SKIP_SPACES();
    MSG_DBG(DL_TRC, ">line_pnt:%d   >%c<(%d)", result->line_str.line_pnt_ro, sym, sym);

    switch (sym) {
            //       case 0:
        case ')':
            return pe;
        case '-':
            SKIP_SYM();
            pe = register_expr(result, vmo_INV, expr_cur);
            break;
        case '!':
            SKIP_SYM();
            pe = register_expr(result, vmo_NOT, expr_cur);
            break;
        default:
            parse_type_t exp_type = parse_exp_token(result);
            MSG_DBG(DL_DBG1, "exp_type:%d  calc_comptime:%d  line_pnt:%d  cur_sym:%c token.size:%d", exp_type, expr_cur->calc_comptime, result->line_str.line_pnt_ro, CUR_SYM(), result->token.size);
            expr_cur->type = exp_type;
#ifdef DEBUG
            if (result->token.size) {
                CopyNameFromToken(expr_cur->name);
                expr_cur->name[result->token.size] = 0;
            }
#endif
            switch (exp_type) {
                case pcs_function: {
                    pe = ParseFunc(result, 1);
                } break;
                case pcs_number: {
                    ParseNum(result, expr_cur);
                    if (!expr_cur->calc_comptime) {
                        VmOp_ArgNum(result, expr_cur->value);
                    }
                } break;
                case pcs_byte: {
                    SKIP_SYM();
                    char symb = 0;
                    if (CUR_SYM() == '\\') {
                        SKIP_SYM();
                        switch (CUR_SYM()) {
                            case 'n':
                                symb = '\n';
                                break;
                            case 't':
                                symb = '\t';
                                break;
                            case '\\':
                                symb = '\\';
                                break;
                            default:
                                pe = pe_expression_invalid;
                        }
                    } else {
                        symb = CUR_SYM();
                    }
                    if (pe == pe_no_error) {
                        SKIP_SYM();
                        if (CUR_SYM() == '\'') {
                            SKIP_SYM();
                            expr_cur->value = i2fpt(symb);
                            strncpy(expr_cur->name, "char", TOKEN_MAX_LEN);  // DEBUG
                            expr_cur->type = pcs_number;
                            if (!expr_cur->calc_comptime) {
                                VmOp_ArgNum(result, expr_cur->value);
                            }
                        } else {
                            pe = pe_expression_invalid;
                        }
                    }
                } break;
                case pcs_variable: {
                    // MSG_DBG(DL_TRC, "enable_code_gen:%d", result->enable_code_gen);
                    const_gen_info_t *const_gen_info;
                    pe = RegisterConstGenVar(result, result->token.data, 0, 0, &const_gen_info);
                    if (pe == pe_no_error) {
                        expr_cur->type = pcs_number;
                        expr_cur->value = const_gen_info->data;
                        strncpy(expr_cur->name, const_gen_info->name, TOKEN_MAX_LEN);
                        if (!expr_cur->calc_comptime) {
                            VmOp_ArgNum(result, expr_cur->value);
                        }
                    } else {
                        if (expr_cur->calc_comptime) {
                            pe = pe_error_eval_exp;
                        } else {
                            pe = ParseVar(result, vsm_GET_VAR, &expr_cur->ctx_var_info);
                            // MSG_DBG(DL_TRC, "enable_code_gen:%d", result->enable_code_gen);
                            if (expr_cur->ctx_var_info.is_object) {
                                MSG_ERR("Error in using the object");
                                pe = pe_expression_invalid;
                            } else {
                                if (pe == pe_no_error) {
                                    pe = VmOp_ArgVar(result, &expr_cur->ctx_var_info);
                                }
                            }
                        }
                    }
                } break;
                default: {
                    if (CUR_SYM() == '(') {
                        SKIP_SYM();
                        (*expr_cur->brackets_cnt)++;
                        MSG_DBG(DL_TRC, "Brace++ :%d", *expr_cur->brackets_cnt);
                        pe = register_expr(result, 0, expr_cur);
                        if (GET_CUR_SKIP_SPACES() == ')') {
                            SKIP_SYM();
                            if (*expr_cur->brackets_cnt > 0) {
                                (*expr_cur->brackets_cnt)--;
                                MSG_DBG(DL_TRC, "Brace-- :%d", *expr_cur->brackets_cnt);
                            } else {
                                MSG_ERR("Missing opening bracket?");
                                pe = pe_expression_invalid;
                            }
                        }
                    } else {
                        MSG_DBG(DL_TRC, "Unknown sym:%d", CUR_SYM());
                        pe = pe_expression_invalid;
                    }
                }
            }
    }

    MSG_DBG(DL_TRC, "<pe:%d  line_pnt:%d   >%c<(%d)", pe, result->line_str.line_pnt_ro, CUR_SYM(), CUR_SYM());
    return pe;
}
