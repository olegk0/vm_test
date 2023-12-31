#include <stdio.h>
#include <string.h>
#include "parser_gen.h"
#include "parser_cmd_tokens.h"
#include "parse_exp.h"
#include "parser_reg.h"
#include "vm_code.h"
#include "vm_funcs.h"

#define CONV_TYPE(err, str) str
static const char *parse_errorrs_str[] = {ERRORS_LIST};
#undef CONV_TYPE

parse_error_t PrintError(parse_result_t *result, parse_error_t error) {
    fprintf(stderr, "\nError: line:%d, pos:%d, near the (%s) %s(%d)", result->line_str.line_num, result->line_str.line_pnt_ro,
            result->token.data, GetParseErrorStr(error), error);

    return error;
}

/*
token name:
0-9
a-z
A-Z
_
*/
parse_error_t GetToken(parse_result_t *result) {
    result->token.size = 0;
    result->token.data[0] = 0;
    result->token.parsed_cmd = cmd_id_NOP;
    char c = GET_CUR_SKIP_SPACES();
    // MSG_DBG(DL_TRC, "CUR_SYM >%c<", c);
    while (c) {
        MSG_DBG(DL_TRC, "_%c_   pnt %d  result->token.size %d", c, result->line_str.line_pnt_ro, result->token.size);
        if (VAR_IN_EQ(c, '0', '9') || VAR_IN_EQ(c, 'a', 'z') || VAR_IN_EQ(c, 'A', 'Z') || c == '_') {
            if (result->token.size == 0) {
                if (VAR_IN_EQ(c, '0', '9')) {
                    // MSG_DBG(DL_DBG,"token must not start with a number");
                    return pe_token_start_with_num;
                }
            }
            result->token.data[result->token.size] = UpperCaseChar(c);
            result->token.size++;
        } else if (c <= ' ') {
            if (result->token.size > 0) {
                result->token.token_valid = 1;
                result->token.data[result->token.size] = 0;
                //   return pe_no_error;
            }
            // if (EOL_FLAG()) {
            return pe_no_error;
            // }
        } else {
            if (result->token.size > 0) {
                result->token.token_valid = 1;
                result->token.data[result->token.size] = 0;
            } else {
                if (c == '{') {
                    result->token.parsed_cmd = cmd_id_BLOCK_BEGIN;
                } else if (c == '}') {
                    result->token.parsed_cmd = cmd_id_BLOCK_END;
                }
                // result->token.data[0] = c;
            }
            return pe_no_error;
        }
        c = ToNextSym(result, 0);
    };
    // MSG_DBG(DL_DBG,"error *result->token.begin:%d  *result->token.size:%d", *result->token.begin, result->token.size);
    return pe_token_error_parse;
}

const char *GetParseErrorStr(parse_error_t error) {
    return parse_errorrs_str[error];
}

parse_error_t parse_string(parse_result_t *result, const char *token_str, const_array_info_t **const_array_info) {
    MSG_DBG(DL_TRC, "");
    if (GET_CUR_SKIP_SPACES()) {
        MSG_DBG(DL_TRC, "next_sym >%c<", CUR_SYM());
        if (CUR_SYM() == '"') {  // string ?
            char *str = &CUR_SYM() + 1;
            char c;
            uint8_t size = 0;
            while ((c = ToNextSym(result, 0))) {
                // MSG_DBG(DL_TRC, "next_sym >%c<", c);
                if (c == '"') {
                    // CUR_SYM() = 0;
                    // MSG_DBG(DL_DBG, "string: >%s<", str);
                    MSG_DBG(DL_DBG, "string: >%.*s<", size, str);
                    // result->line_str.line_pnt_ro++;  //
                    SKIP_SYM();
                    return RegisterConstArray(result, token_str, str, size, vt_array_of_char, TRUE, const_array_info);
                }
                size++;
                if (size > 254) {
                    return pe_array_or_string_too_long;
                }
            }
        }
    }
    return pe_expression_invalid;
}

parse_error_t parse_array(parse_result_t *result, const char *token_str, const_array_info_t **const_array_info) {
    MSG_DBG(DL_TRC, "");
    parse_error_t pe;
    if (GET_CUR_SKIP_SPACES() == '{') {  // array
        SKIP_SYM();
        MSG_DBG(DL_TRC, "next_sym >%c<", CUR_SYM());
        fpt data[260];
        uint8_t size = 0;
        char c;
        while ((c = GET_CUR_SKIP_SPACES())) {
            MSG_DBG(DL_TRC, "next_sym0 >%c<", c);
            switch (c) {
                case '}': {
                    SKIP_SYM();
                    pe = RegisterConstArray(result, token_str, data, size, vt_array_of_generic, TRUE, const_array_info);
                    return pe;
                } break;
                case ',':
                    SKIP_SYM();
                    break;
                default:
                    expr_info_t expr_cur;
                    pe = ParseNum(result, &expr_cur);
                    if (pe) {
                        return pe;
                    }
                    data[size] = expr_cur.value;
                    size++;
                    if (size > 254) {
                        return pe_array_or_string_too_long;
                    }
            }
        }
    }
    return pe_expression_invalid;
}

parse_error_t ParseVar(parse_result_t *result, var_set_mode_t mode, ctx_var_info_t *ctx_var_info) {
    parse_error_t pe = pe_no_error;
    MSG_DBG(DL_DBG1, "var:>%s<   mode:%d   cur_sym:%c", result->token.data, mode, CUR_SYM());
    // if (mode < vsm_DECLARE_end && result->declare_scope == 0) {
    //     MSG_ERR("Variables must be declared at the beginning of the program/function");
    //    return pe_syntax_invalid;
    // }
    uint8_t need_pointer = mode & vsm_POINTER_FLAG;
    mode &= vsm_POINTER_FLAG - 1;
    int array_size = -1;
    ctx_var_info->array_compile_time_idx = -1;
    ctx_var_info->is_object = FALSE;
    char token_str_bak[TOKEN_MAX_LEN] = {0};
    bool no_idx = TRUE;
    ctx_var_info->array_auto_size = FALSE;
    // char is_array = 0;
    if (CUR_SYM() == '[') {  // array
        if (mode == vsm_DECLARE_VAR) {
            if (need_pointer) {
                mode = vsm_DECLARE_g_POINTER;
            } else {
                mode = vsm_DECLARE_VAR_ARRAY;
            }
        } else {
            if (need_pointer) {
                mode = vsm_DECLARE_b_POINTER;
            }
        }
        no_idx = FALSE;
        // is_array = 1;
        strncpy(token_str_bak, result->token.data, TOKEN_MAX_LEN);
        SKIP_SYM();  //'['
        MSG_DBG(DL_DBG, "is array, idx >[ ");
        fpt val;
        switch (mode) {
            case vsm_DECLARE_g_POINTER:
            case vsm_DECLARE_b_POINTER:
            case vsm_DECLARE_BYTE_ARRAY:
            case vsm_DECLARE_CHAR_ARRAY:
            case vsm_DECLARE_VAR_ARRAY:
                if (GET_CUR_SKIP_SPACES() == ']') {  // calculate the size of the array later by the size of the right side
                    MSG_DBG(DL_DBG1, "array_auto_size");
                    array_size = 0;
                    ctx_var_info->array_auto_size = TRUE;
                } else {
                    if (need_pointer) {
                        MSG_ERR("What is required here is not an object but a pointer");
                        return pe_syntax_invalid;
                    }
                    pe = ExpParseCompileTime(result, &val);
                    if (pe == pe_no_error) {
                        array_size = fpt2i(val);
                        MSG_DBG(DL_DBG, "value:%d", array_size);
                        if (array_size <= 0) {
                            MSG_ERR("Array size must be positive");
                            return pe_array_index_invalid;
                        }
                    } else {
                        MSG_ERR("Unable to evaluate array index or size at compile time");
                        return pe;
                    }
                }
                break;
            default:  // set/get array
                LINE_PNT_STORE();
                // DISABLE_CODE_GEN();
                pe = ExpParseCompileTime(result, &val);
                // ENABLE_CODE_GEN();
                if (pe == pe_no_error) {
                    ctx_var_info->array_compile_time_idx = fpt2i(val);
                    // LINE_PNT_POP();
                } else {
                    LINE_PNT_RESTORE();
                    if (pe == pe_error_eval_exp) {
                        pe = ExpParse(result);
                    }
                }
                if (pe) {
                    return pe;
                }
                array_size = 0;
                if (mode == vsm_GET_VAR) {
                    mode = vsm_GET_ARRAY;
                } else {
                    mode = vsm_SET_ARRAY;
                }
        }
        GET_CUR_SKIP_SPACES();
        MSG_DBG(DL_DBG, " %c< ", CUR_SYM());
        if (CUR_SYM() == ']') {
            SKIP_SYM();  // ]
            if (ctx_var_info->array_auto_size) {
                if (GET_CUR_SKIP_SPACES() == '=' || result->cur_cmd == cmd_id_PROC) {
                    ctx_var_info->is_object = TRUE;
                } else {
                    MSG_ERR("Unable to evaluate array size at compile time");
                    return pe_array_index_invalid;
                }
            }
        } else {
            return pe_syntax_invalid;
        }
    } else {               // not array
        need_pointer = 0;  // only for object
        // MSG_ERR("byte and char can only be used as arrays");
    }
    MSG_DBG(DL_DBG1, "cur_sym:>%c<", CUR_SYM());

    if (token_str_bak[0]) {
        pe = RegisterVar(result, token_str_bak, array_size, mode, &ctx_var_info->var_info);
    } else {
        pe = RegisterVar(result, result->token.data, array_size, mode, &ctx_var_info->var_info);
    }

    if (pe == pe_no_error) {
        MSG_DBG(DL_DBG, "var type:%d", ctx_var_info->var_info->type);
        if (ctx_var_info->var_info->type & vt_arrays) {
            if (no_idx) {
                ctx_var_info->is_object = TRUE;
            }
        }
    }
    return pe;
}

int get_cmd_info_pnt(char *token) {
    int p = 0;
    while (table_cmd[p].text[0]) {
        if (strncmp(table_cmd[p].text, token, CMD_SIZE_MAX) == 0) {
            return p;
        }
        p++;
    }
    return -1;
}

parse_error_t check_sym(parse_result_t *result, uint8_t step) {
    MSG_DBG(DL_DBG1, "CASE sym check   cur_cmd:%d", result->cur_cmd);
    parse_error_t pe = pe_no_error;  // GetToken(result);
                                     // RETURN_ON_ERROR(pe);

    MSG_DBG(DL_DBG1, "compare, expect >%c< VS available >%c<", step, CUR_SYM());
    if (GET_CUR_SKIP_SPACES() != step) {
        if (!result->optional_mod) {
            MSG_DBG(DL_DBG, "symbol '%c' is missing", step);
        }
        pe = pe_expression_invalid;
    } else {
        SKIP_SYM();
    }
    return pe;
}

parse_error_t open_block(parse_result_t *result, block_type_t type, char early) {
    parse_error_t pe = pe_no_error;
    result->early_opened_block = early;
    // result->cur_block++;
    result->block_id++;
    VECTOR_NEW(result->block_vect);
    VECTOR_LAST(result->block_vect).block_type = type;
    MSG_DBG(DL_DBG, "Open Block:%d  type:%d  early:%d  blocks opened:%d", VECTOR_SIZE(result->block_vect), type, early, VECTOR_SIZE(result->block_vect));
    switch (type) {
            //  case bt_else:
        case bt_if:
        case bt_loop:
            // case bt_condition:
            pe = RegisterLabel(result, NULL, 0, &VECTOR_LAST(result->block_vect).label_end);
            break;
        default:
    }

    return pe;
}

void begin_block(parse_result_t *result, uint8_t past_cmd) {
    MSG_DBG(DL_TRC, "past_cmd: %d", past_cmd);
    result->cmd_templ_pnt = curly_brace_begin_templ;
    char condition_jmp = 0;
    block_type_t btype = bt_none;
    switch (past_cmd) {
        case cmd_id_IF:
            btype = bt_if;
            condition_jmp = 1;
            break;
        case cmd_ext_id_ELSE:
            btype = bt_else;
            break;
        case cmd_id_WHILE:
            btype = bt_loop;
            condition_jmp = 1;
            break;
        default:
    }
    if (result->early_opened_block) {
        result->early_opened_block = 0;
        MSG_DBG(DL_DBG, "Skip Open Block:%d", VECTOR_SIZE(result->block_vect));
    } else {
        open_block(result, btype, 0);
    }
    if (condition_jmp) {
        if (result->calc_comptime) {  // infinite loop
        } else {
            VmOp_Jmp(result, VECTOR_LAST(result->block_vect).label_end, 1);
        }
    }
}

void const_action(parse_result_t *result, const_array_info_t *const_array_info) {
    switch (result->cur_cmd) {
        case cmd_id_CONST:
            // skip
            break;
        default:
            VmOp_ArgConstArray(result, const_array_info);
    }
}

parse_error_t Check_as_array(parse_result_t *result, uint8_t expr_flags, const_array_info_t **const_array_info) {
    parse_error_t pe = pe_error_eval_exp;
    // 1. check string
    // char str[250];
    // params_type
    char *token_str = NULL;
    if (TOKEN_VALID()) {
        token_str = result->token.data;
        TOKEN_INVALIDATE();
    }
    MSG_DBG(DL_DBG1, "token_str: %s", token_str);
    if (expr_flags & efl_inline_const_string) {
        MSG_DBG(DL_DBG1, "test inline as const string");
        pe = parse_string(result, token_str, const_array_info);
        if (pe == pe_no_error) {  // const
            if (const_array_info) {
                const_action(result, *const_array_info);
            }
            MSG_DBG(DL_TRC, "string Ok  CUR_SYM():%d", CUR_SYM());
            return pe;
        }
    }
    if (expr_flags & efl_inline_const_array) {
        MSG_DBG(DL_DBG1, "test as inline const array");
        pe = parse_array(result, token_str, const_array_info);
        if (pe == pe_no_error) {
            const_action(result, *const_array_info);
            return pe;
        }
    }
    if (expr_flags & (efl_const_array | efl_var_array)) {
        LINE_PNT_STORE();
        pe = GetToken(result);
        if (pe == pe_no_error) {
            TOKEN_INVALIDATE();
            if (expr_flags & efl_const_array) {  //  look in the list of const arrays
                MSG_DBG(DL_DBG1, "test as const array");
                // MSG_DBG(DL_TRC, "token_str1:%s", result->token.data);
                pe = RegisterConstArray(result, result->token.data, NULL, 0, vt_none, FALSE, const_array_info);
                if (pe == pe_no_error) {
                    const_action(result, *const_array_info);
                    return pe;
                }
            }
            if (expr_flags & efl_var_array) {
                MSG_DBG(DL_DBG1, "test as var array");
                ctx_var_info_t ctx_var_info;
                pe = ParseVar(result, vsm_GET_ARRAY, &ctx_var_info);
                if (pe == pe_no_error) {
                    MSG_DBG(DL_DBG1, "is_object:%d   type:%d", ctx_var_info.is_object, ctx_var_info.var_info->type);
                    if (ctx_var_info.is_object) {
                        VmOp_ArgVar(result, &ctx_var_info);
                        return pe;
                    } else {
                        pe = pe_error_eval_exp;
                    }
                }
            }
        }
        LINE_PNT_RESTORE();
    }
    return pe;
}

void nextStep(parse_result_t *result, uint8_t *step) {
    uint8_t lstep;
    result->optional_mod = FALSE;
    while (1) {
        lstep = result->cmd_templ_pnt[0];
        result->cmd_templ_pnt++;
        if (lstep == 0 || lstep != pcs_IND_optional) {
            break;
        }
        result->optional_mod = TRUE;
    }
    result->group_mod = lstep & pcs_MOD_group;
    (*step) = lstep & 0x7f;
}

void definePartArgType(parse_result_t *result, print_part_type_t part_type) {
    // if (result->enable_code_gen) {
    VmOp_Debug1(result, "Part", part_type);
    if (result->params_str.params_cnt < FUNC_MAX_PARAMS) {
        result->params_str.params_type[result->params_str.params_cnt] = part_type;
        result->params_str.params_cnt++;
    }
    // }
}

parse_error_t ParseStep(parse_result_t *result, uint8_t step) {
    parse_error_t pe = pe_no_error;

    line_str_t *line = &result->line_str;

    MSG_DBG(DL_DBG1, "BEGIN step:%d  optional:%d  cmd_templ:%d  line->pnt:%d   params_lvl:%d  params_cnt:%d  >%d<   token_valid:%d  token:%s", step, result->optional_mod, result->cmd_templ_pnt ? result->cmd_templ_pnt[0] : 0, line->line_pnt_ro, result->params_lvl, result->params_str.params_cnt, CUR_SYM(), result->token.token_valid, result->token.data);
    switch (step) {
        case pcs_none:  // cmd no longer has any arguments
            MSG_DBG(DL_DBG1, "CASE end");
            break;
        case pcs_command: {  // command or label
            MSG_DBG(DL_DBG1, "CASE pcs_command");
            // parse_error_t pe = pe_no_error;
            if (!TOKEN_VALID()) {
                pe = GetToken(result);
                RETURN_ON_ERROR(pe);
                MSG_DBG(DL_TRC, "token.size:%d", result->token.size);
            }

            if (result->token.size == 0) {
                if (result->token.parsed_cmd) {  // { or }
                    SKIP_SYM();
                } else {
                    return pe_command_invalid;
                }
            }

            MSG_DBG(DL_DBG, "cmd >%s<   parsed_cmd:%d  cur_sym:>%c<", result->token.data, result->token.parsed_cmd, CUR_SYM());

            uint8_t past_cmd = result->cur_cmd;
            result->expr_flags = 0;
            if (result->token.parsed_cmd) {  // { or }
                result->cur_cmd = result->token.parsed_cmd;
                // result->cmd_templ_pnt = curly_brace_templ;
            } else {
                int cmd_table_pnt = get_cmd_info_pnt(result->token.data);
                if (cmd_table_pnt < 0) {
                    char cl = GET_CUR_SKIP_SPACES();
                    if (cl == '=' || cl == '[') {  // it`s var
                        MSG_DBG(DL_TRC, "variable assignment detect");
                        result->cur_cmd = cmd_ext_id_LET;
                        result->cmd_templ_pnt = TABLE_CMD_EXT(cmd_ext_id_LET).templ;
                        result->expr_flags = TABLE_CMD_EXT(cmd_ext_id_LET).expr_flags;
                        goto do_next;
                        break;
                    } else if (cl == '(') {  // it`s func
                        MSG_DBG(DL_TRC, "func call detect");
                        result->cur_cmd = cmd_ext_id_CALL;
                        result->cmd_templ_pnt = TABLE_CMD_EXT(cmd_ext_id_CALL).templ;
                        result->expr_flags = TABLE_CMD_EXT(cmd_ext_id_LET).expr_flags;
                        goto do_next;
                        break;
                    } else {
                        MSG_ERR("Cmd %s not found!", result->token.data);
                        return pe_command_invalid;
                    }
                } else {
                    result->cur_cmd = table_cmd[cmd_table_pnt].cmd_id;
                    result->expr_flags = table_cmd[cmd_table_pnt].expr_flags;
                    result->cmd_templ_pnt = table_cmd[cmd_table_pnt].templ;
                }
            }
            TOKEN_INVALIDATE();
            VmOp_Debug(result, "cmd", result->token.data);
            MSG_DBG(DL_TRC, "cur_cmd:%d", result->cur_cmd);
            // char declare_var = 0;
            switch (result->cur_cmd) {
                case cmd_id_BRKPNT:
                    // declare_var = 1;
                    VmOp_BreakPoint(result);
                    break;
                case cmd_id_STOP:
                    return (pe_stop);
                    break;
                case cmd_id_BLOCK_BEGIN:
                    begin_block(result, past_cmd);
                    break;
                case cmd_id_BLOCK_END:
                    if (VECTOR_SIZE(result->block_vect) > 0) {
                        result->cmd_templ_pnt = curly_brace_end_templ;
                        char double_pop = 0;
                        pe = UnregisterLastBlockVars(result);
                        MSG_DBG(DL_TRC, "Block type:%d  GET_CUR_SKIP_SPACES:%d", VECTOR_LAST(result->block_vect).block_type, GET_CUR_SKIP_SPACES());
                        switch (VECTOR_LAST(result->block_vect).block_type) {
                            case bt_loop:
                                pe = VmOp_Jmp(result, VECTOR_LAST(result->block_vect).label_begin, 0);
                                break;
                            case bt_if:
                            case bt_else:
                                MSG_DBG(DL_DBG1, "Cond Block:%d  type-1:%d  label_pnt-1:%p", VECTOR_SIZE(result->block_vect), VECTOR_FROM_LAST(result->block_vect, 1).block_type, VECTOR_FROM_LAST(result->block_vect, 1).label_end);
                                if (GET_CUR_SKIP_SPACES()) {  // there is something behind the bracket. we are waiting "else"
                                    if (VECTOR_FROM_LAST(result->block_vect, 1).label_end == NULL) {
                                        pe = RegisterLabel(result, NULL, 0, &VECTOR_FROM_LAST(result->block_vect, 1).label_end);  // offset to cond block
                                    }
                                    MSG_DBG(DL_DBG1, "Jump to label:%d", VECTOR_FROM_LAST(result->block_vect, 1).label_end->id);
                                    pe = VmOp_Jmp(result, VECTOR_FROM_LAST(result->block_vect, 1).label_end, 0);
                                } else {
                                    MSG_DBG(DL_DBG1, "no else");
                                    if (VECTOR_FROM_LAST(result->block_vect, 1).block_type != bt_condition) {
                                        RETURN_ON_ERROR(pe_syntax_invalid);
                                    }
                                    if (VECTOR_FROM_LAST(result->block_vect, 1).label_end) {
                                        pe = RegisterLabel(result, VECTOR_FROM_LAST(result->block_vect, 1).label_end->name, 1, &VECTOR_FROM_LAST(result->block_vect, 1).label_end);
                                        //  double_pop = 1;  // pop condition block
                                        // MSG_DBG(DL_TRC, "Cond Block end");
                                    }
                                    double_pop = 1;  // pop condition block
                                    MSG_DBG(DL_TRC, "Cond Block end");
                                }
                                break;
                            default: {
                            }
                        }

                        if (VECTOR_LAST(result->block_vect).label_end) {
                            pe = RegisterLabel(result, VECTOR_LAST(result->block_vect).label_end->name, 1, &VECTOR_LAST(result->block_vect).label_end);
                        }

                        MSG_DBG(DL_DBG, "Close Block:%d  type:%d  double_pop:%d", VECTOR_SIZE(result->block_vect), VECTOR_LAST(result->block_vect).block_type, double_pop);
                        // pe = UnregisterLastBlockVars(result);
                        VECTOR_POP(result->block_vect);

                        if (VECTOR_SIZE(result->block_vect) == 0) {  // top level
                            if (!double_pop) {
                                if (result->func_info) {
                                    MSG_DBG(DL_DBG, "End proc:%s", result->func_info->name);
                                    // if (!result->func_info->have_ret) {
                                    //  VmOp_PopRet(result);
                                    //   VmOp_ArgNum(result, FPT_ZERO); move to VmOp_Return
                                    //}
                                    pe = VmOp_Return(result, result->func_info);
                                    EndSubContext(result);
                                    // result->declare_scope = 1;
                                    // declare_var = 1;
                                }
                                break;
                            }
                        } else {
                            if (double_pop) {
                                VECTOR_POP(result->block_vect);
                            }
                            break;
                        }
                    }
                    MSG_ERR("Redundant curly brace?");
                    pe = pe_syntax_invalid;
                    break;
                case cmd_id_WHILE:
                    open_block(result, bt_loop, 1);
                    // register label
                    char buf[32];
                    AutoName(result, buf, sizeof(buf), "loop_bg");
                    pe = RegisterLabel(result, buf, 1, &VECTOR_LAST(result->block_vect).label_begin);
                    break;
                case cmd_id_IF:
                    if (VECTOR_LAST(result->block_vect).block_type != bt_condition) {
                        open_block(result, bt_condition, 0);
                    }
                    break;
                case cmd_id_RETURN:
                    //  VmOp_PopRet(result);
                    break;
                case cmd_id_BREAK:
                    for (int i = VECTOR_SIZE(result->block_vect) - 1; i >= 0; i--) {
                        if (result->block_vect.data[i].block_type == bt_loop) {
                            pe = VmOp_Jmp(result, VECTOR_FROM_FIRST(result->block_vect, i).label_end, 0);
                            break;
                        }
                    }
                    break;
                case cmd_id_PROC:
                    // result->declare_scope = 1;
                //   break;
                case cmd_id_VAR:
                case cmd_id_CONST:
                case cmd_id_BYTE:
                case cmd_id_CHAR:
                    // declare_var = 1;
                    break;
                case cmd_id_PRINT:
                case cmd_id_PRINT_LN:
                    break;
                default: {
                    MSG_DBG(DL_WRN, "TODO check cmd:%s", result->token.data);
                }
            }
            // if (declare_var == 0) {
            //     result->declare_scope = 0;
            // }
        } break;
        case pcs_else: {
            MSG_DBG(DL_DBG1, "CASE pcs_else");
            VmOp_Debug(result, "cmd", "Else");
            pe = GetToken(result);
            TOKEN_INVALIDATE();
            RETURN_ON_ERROR(pe);
            MSG_DBG(DL_TRC, "token.size:%d", result->token.size);
            pe = pe_syntax_invalid;
            if (result->token.size > 0) {
                if (strncmp(TABLE_CMD_EXT(cmd_ext_id_ELSE).text, result->token.data, CMD_SIZE_MAX) == 0) {
                    result->cur_cmd = cmd_ext_id_ELSE;
                    result->cmd_templ_pnt = TABLE_CMD_EXT(cmd_ext_id_ELSE).templ;
                    if (GET_CUR_SKIP_SPACES() == '{') {  // next { block
                    }
                    pe = pe_no_error;
                }
            } else {  // without else
            }
        } break;
        case pcs_token: {
            MSG_DBG(DL_DBG1, "CASE pcs_token   cur_cmd:%d", result->cur_cmd);
            if (!TOKEN_VALID()) {
                pe = GetToken(result);
                RETURN_ON_ERROR(pe);
                MSG_DBG(DL_TRC, "token.size:%d", result->token.size);
            }
            if (result->token.size > 0) {
                switch (result->cur_cmd) {
                    case cmd_ext_id_CALL:
                        MSG_DBG(DL_DBG, "call label: >%s<", result->token.data);
                        pe = ParseFunc(result, 0);
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_id_PROC:
                        if (result->func_info) {
                            ctx_var_info_t ctx_var_info;
                            pe = ParseVar(result, result->func_info->var_set_mode | vsm_POINTER_FLAG, &ctx_var_info);
                            result->func_info->params_info[result->func_info->params_cnt] = ctx_var_info;
                            if (ctx_var_info.var_info->type == vt_generic) {
                                result->func_info->params[result->func_info->params_cnt] = ft_generic;
                            } else {
                                result->func_info->params[result->func_info->params_cnt] = ft_pointer;
                            }
                            result->func_info->params_cnt++;
                            if (result->func_info->params_cnt >= FUNC_MAX_PARAMS) {
                                MSG_ERR("Number of function parameters exceeded");
                                RETURN_ON_ERROR(pe_object_max_number_exceeded);
                            }
                        } else {
                            MSG_DBG(DL_DBG, "proc name: >%s<", result->token.data);
                            open_block(result, bt_proc, 1);
                            VmOp_Debug(result, "name", result->token.data);
                            pe = RegisterSub(result, result->token.data, 1, &result->func_info);
                        }
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_id_CONST:
                        strncpy(result->token_back, result->token.data, TOKEN_MAX_LEN);
                        break;
                    case cmd_id_GOTO:
                        label_info_t *llabel;
                        pe = RegisterLabel(result, result->token.data, 0, &llabel);
                        pe = VmOp_Jmp(result, llabel, 0);
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_id_BYTE:  // only array
                        pe = ParseVar(result, vsm_DECLARE_BYTE_ARRAY, &result->ctx_var_info);
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_id_CHAR:  // only array
                        pe = ParseVar(result, vsm_DECLARE_CHAR_ARRAY, &result->ctx_var_info);
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_id_VAR:
                        pe = ParseVar(result, vsm_DECLARE_VAR, &result->ctx_var_info);
                        TOKEN_INVALIDATE();
                        break;
                    case cmd_ext_id_LET:
                        pe = ParseVar(result, vsm_SET_VAR, &result->ctx_var_info);
                        MSG_DBG(DL_DBG, "is_object: %d", result->ctx_var_info.is_object);
                        if (result->ctx_var_info.is_object) {
                            result->expr_flags = LET_pnt_expr_flags;
                        }
                        TOKEN_INVALIDATE();
                        break;
                    default: {
                        MSG_DBG(DL_DBG, "TODO use token: >%s< for cmd:%d", result->token.data, result->cur_cmd);
                        RETURN_ON_ERROR(pe_syntax_invalid);
                    }
                }
            } else {
                pe = pe_syntax_invalid;
            }
        } break;
        case pcs_variable: {
            MSG_DBG(DL_DBG1, "CASE pcs_variable, cur_cmd:%d", result->cur_cmd);
            if (!TOKEN_VALID()) {
                pe = GetToken(result);
                RETURN_ON_ERROR(pe);
            }
            TOKEN_INVALIDATE();
            // MSG_DBG(DL_DBG, "token: %s  result->token.size:%d",result->token.data,result->token.size);
            if (result->token.size > 0) {
                switch (result->cur_cmd) {
                    case cmd_id_PROC: {  // args
                        int cmd_table_pnt = get_cmd_info_pnt(result->token.data);
                        if (cmd_table_pnt < 0) {
                            return pe_syntax_invalid;
                        } else {
                            uint8_t cmd_id = table_cmd[cmd_table_pnt].cmd_id;
                            MSG_DBG(DL_DBG, "cmd_id:%d", cmd_id);
                            switch (cmd_id) {
                                case cmd_id_VAR:
                                    result->func_info->var_set_mode = vsm_DECLARE_VAR;
                                    break;
                                case cmd_id_BYTE:
                                    result->func_info->var_set_mode = vsm_DECLARE_BYTE_ARRAY;
                                    break;
                                case cmd_id_CHAR:
                                    result->func_info->var_set_mode = vsm_DECLARE_CHAR_ARRAY;
                                    break;
                                default:
                                    return pe_syntax_invalid;
                            }
                            // result->expr_flags = table_cmd[cmd_table_pnt].expr_flags;
                            // result->cmd_templ_pnt = table_cmd[cmd_table_pnt].templ;
                        }
                    } break;
                    default: {
                        MSG_DBG(DL_DBG, "TODO use var: >%s< for cmd:%d", result->token.data, result->cur_cmd);
                        RETURN_ON_ERROR(pe_syntax_invalid);
                    }
                }

            } else {
                pe = pe_var_error_parse;
            }
        } break;
        case ')':
            switch (result->cur_cmd) {
                case cmd_id_PROC:
                    MSG_DBG(DL_TRC, "func params cnt: %d", result->func_info->params_cnt);
                    for (int i = result->func_info->params_cnt - 1; i >= 0; i--) {  // reverse
                        result->func_info->params[result->func_info->params_cnt] = 0;
                        // if (result->func_info->params_info[i].is_object) {
                        //} else {
                        pe = VmOp_PopVar(result, &result->func_info->params_info[i]);
                        //}
                    }
                    // VmOp_PushRet(result);
                    break;
                default:
            }
            if (pe == pe_no_error) {
                result->params_lvl--;
                pe = check_sym(result, step);
            }
            break;
        case '=':
        case '(':
            result->params_lvl++;
            result->params_str.params_cnt = 0;
            pe = check_sym(result, step);
            break;
        case '{':
            pe = check_sym(result, step);
            if (!pe) {
                begin_block(result, result->cur_cmd);
            }
            break;
        case ',':
            pe = check_sym(result, step);
            break;
        case pcs_repeated: {
            MSG_DBG(DL_DBG1, "CASE pcs_repetition");
            if (CUR_SYM() == ',') {
                // inc_param_cnt(result);
                SKIP_SYM();
                result->cmd_templ_pnt -= 2;                             // -1 - pcs_repeated; -1 - past action
                while (*(result->cmd_templ_pnt - 1) & pcs_MOD_group) {  // rewind the entire group
                    result->cmd_templ_pnt--;
                }
                MSG_DBG(DL_DBG1, "cmd_templ_pnt:%d", result->cmd_templ_pnt[0]);
                // goto do_next;
            }
        } break;
        case pcs_expression: {
            MSG_DBG(DL_DBG1, "CASE pcs_expression  cmd:%d  expr_flags:0x%X", result->cur_cmd, result->expr_flags);
            result->calc_comptime = 0;
            fpt calc_val = 0;
            bool arg_is_object = FALSE;
            const_array_info_t *const_array_info;
            pe = Check_as_array(result, result->expr_flags, &const_array_info);
            if (pe == pe_no_error) {  // expression is array
                arg_is_object = TRUE;
                if (result->ctx_var_info.var_info) {  // variable assignment
                    if (result->ctx_var_info.array_auto_size) {
                        if (const_array_info->type == result->ctx_var_info.var_info->type) {
                            pe = FixArraySize(result, const_array_info->elm_count, result->ctx_var_info.var_info);
                        } else {
                            MSG_DBG(DL_DBG1, "types: %d vs %d", result->ctx_var_info.var_info->type, const_array_info->type);
                            MSG_ERR("The right and left sides of the expression are incompatible");
                            pe = pe_syntax_invalid;
                            break;
                        }
                    } else {
                        if (result->ctx_var_info.is_object) {
                            // array_copy = TRUE;
                        } else {
                            MSG_ERR("You can't copy an array to an array element");
                            pe = pe_syntax_invalid;
                            break;
                        }
                    }
                }
                // break;
            } else {
                if (result->expr_flags & efl_compile_calc_expr) {
                    MSG_DBG(DL_DBG1, "===ExpParseCompileTime===");
                    LINE_PNT_STORE();
                    pe = ExpParseCompileTime(result, &calc_val);
                    if (pe == pe_no_error) {
                        // LINE_PNT_POP();
                        MSG_DBG(DL_DBG1, "___ExpParseCompileTime___");
                        if (result->cur_cmd == cmd_id_CONST) {
                            const_gen_info_t *const_gen_info;
                            pe = RegisterConstGenVar(result, result->token_back, calc_val, 1, &const_gen_info);
                            break;
                        }
                    } else {
                        if (result->cur_cmd == cmd_id_CONST) {
                            // LINE_PNT_POP();
                            break;
                        }
                        LINE_PNT_RESTORE();
                    }
                }
                if (pe && result->expr_flags & efl_expr) {
                    MSG_DBG(DL_DBG1, "===ExpParseRunTime===");
                    pe = ExpParse(result);
                    MSG_DBG(DL_DBG1, "___ExpParseRunTime___");
                }
            }
            if (pe == pe_no_error) {
                switch (result->cur_cmd) {
                    case cmd_id_CHAR:
                    case cmd_id_BYTE:
                    case cmd_id_VAR:
                        // break;
                    case cmd_ext_id_LET:
                        // assign a variable
                        if (result->ctx_var_info.var_info) {
                            MSG_DBG(DL_DBG1, "calc_comptime:%d    is_object:%d", result->calc_comptime, result->ctx_var_info.is_object);
                            if (result->calc_comptime) {
                                VmOp_ArgNum(result, calc_val);
                            }
                            pe = VmOp_PopVar(result, &result->ctx_var_info);
                        }
                        break;
                    case cmd_id_CONST:
                        break;
                    case cmd_id_WHILE:
                        // result->calc_comptime =  infinite loop
                        break;
                    case cmd_id_IF:
                        if (result->calc_comptime) {
                            MSG_ERR("Conditional expression with constant doesn't make sense");
                            return pe_syntax_invalid;
                        }
                        break;
                    case cmd_id_PRINT:
                    case cmd_id_PRINT_LN:
                        if (arg_is_object) {
                            definePartArgType(result, print_ss_pointer);
                        } else {  // print ?
                            definePartArgType(result, print_ss_number);
                        }
                        if (result->calc_comptime) {
                            VmOp_ArgNum(result, calc_val);
                        }
                        break;
                    // case cmd_ext_id_CALL:
                    case cmd_id_RETURN:
                        if (result->func_info) {
                            if (result->calc_comptime) {
                                VmOp_ArgNum(result, calc_val);
                            }
                            // result->func_info->have_return = 1;
                            VmOp_Return_Value(result);
                        } else {
                            MSG_ERR("Unexpected return");
                            return pe_syntax_invalid;
                        }
                        break;
                    default: {
                        MSG_DBG(DL_DBG, "TODO use Exp for cmd:%d", result->cur_cmd);
                        RETURN_ON_ERROR(pe_syntax_invalid);
                    }
                }
            }
        } break;
        default: {
            MSG_ERR("CASE default - syntax invalid");
            return pe_syntax_invalid;
        }
    }

    MSG_DBG(DL_DBG1, "End. Cur step:%d, next templ step:%d,  pe:%d,  optional_mod:%d,  cur_sym:>%d", step,
            result->cmd_templ_pnt ? result->cmd_templ_pnt[0] : 0, pe, result->optional_mod, CUR_SYM());

    switch (pe) {
        case pe_no_error:
            if (GET_CUR_SKIP_SPACES() == '#') {
                TO_END_LINE();
            }

            break;
        case pe_object_exist:
        case pe_object_unknown:  // TODO add errors
            PRINT_ERROR(pe);
            return pe;
            break;
        default:
            if (result->optional_mod) {
                if (result->cmd_templ_pnt[0] == pcs_repeated) {
                    result->cmd_templ_pnt++;  // skip
                    MSG_DBG(DL_TRC, "skip 'repeated' templ step");
                } else if (result->group_mod) {  // skip group
                    while (result->cmd_templ_pnt[0] && result->cmd_templ_pnt[0] & pcs_MOD_group) {
                        result->cmd_templ_pnt++;
                    }
                }
            } else {
                PRINT_ERROR(pe);
                return pe;
            }
    }

do_next:
    if (step != 0) {
        nextStep(result, &step);

        MSG_DBG(DL_TRC, "End. Next step:%d,  cur_sym:%d  optional_mod:%d  group_mod:%d", step, GET_CUR_SKIP_SPACES(), result->optional_mod, result->group_mod);
        if (GET_CUR_SKIP_SPACES()) {
            // TOKEN_INVALIDATE();
            pe = ParseStep(result, step);
            MSG_DBG(DL_DBG1, "End ret:%d", pe);
        } else if (step) {
            if (!result->optional_mod) {
                MSG_DBG(DL_DBG, "Error templ not ended, step:%d", step);
                pe = pe_syntax_invalid;
            }
            /*while (step) {
                if (!result->optional_mod && !result->group_mod) {
                    MSG_DBG(DL_DBG, "Error templ not ended, step:%d", step);
                    pe = pe_syntax_invalid;
                    break;
                }
                nextStep(result, &step);
            }*/
        }
    }

    return pe;
}
