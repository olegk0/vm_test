// #include <stdio.h>
//  #include <string.h>
#include "parser_reg.h"
#include <string.h>
#include "parser_priv.h"
#include "vm_code.h"

#define FIND_OBJECT(OBJ)                                                                   \
    OBJ##_info_t *found_obj = NULL;                                                        \
    int cur_id = 0;                                                                        \
    if (token_str[0] == 0) {                                                               \
        return pe_syntax_invalid;                                                          \
    }                                                                                      \
    for (; cur_id < VECTOR_SIZE(result->OBJ##s_vect); cur_id++) {                          \
        MSG_DBG(DL_TRC, "'%s' VS '%s'", result->OBJ##s_vect.data[cur_id].name, token_str); \
        int p = 0;                                                                         \
        while (1) {                                                                        \
            if (result->OBJ##s_vect.data[cur_id].name[p] != token_str[p]) {                \
                p = -1;                                                                    \
                break;                                                                     \
            }                                                                              \
            if (result->OBJ##s_vect.data[cur_id].name[p] == 0 || token_str[p] == 0) {      \
                break;                                                                     \
            }                                                                              \
            p++;                                                                           \
        }                                                                                  \
        if (p > 0) { /*EQ*/                                                                \
            found_obj = &result->OBJ##s_vect.data[cur_id];                                 \
            break;                                                                         \
        }                                                                                  \
    }

#define FOUND_OBJECT(OBJ)                             \
    *OBJ##_info = found_obj;                          \
    if (new_object) {                                 \
        MSG_ERR(#OBJ " %s already exist", token_str); \
        return pe_object_exist;                       \
    } /*get object*/                                  \
    return pe_no_error;

#define NOT_FOUND_OBJECT(OBJ)                                   \
    if (new_object) {                                           \
        cur_id = VECTOR_SIZE(result->OBJ##s_vect);              \
        VECTOR_NEW_RET(result->OBJ##s_vect, *OBJ##_info);       \
        strncpy((*OBJ##_info)->name, token_str, TOKEN_MAX_LEN); \
    } else { /*get object */                                    \
        MSG_DBG(DL_DBG, #OBJ " %s not exists", token_str);      \
        return pe_object_unknown;                               \
    }

//////////////////
parse_error_t RegisterLabel(parse_result_t *result, const char *token_str, char create_object, label_info_t **label_info) {
    char buf[32];
    if (token_str == NULL) {
        token_str = buf;
        AutoName(result, buf, sizeof(buf), "label");
    }

    MSG_DBG(DL_TRC, "label:%s for line:%d  create_object:%d", token_str, result->line_str.line_num, create_object);
    FIND_OBJECT(label);

    if (found_obj) { /*Found*/
        MSG_DBG(DL_TRC, "Found");
        // FOUND_OBJECT(label);
        *label_info = found_obj;
        if (create_object) {
            if ((*label_info)->code_offs >= 0) {
                MSG_ERR("label %s is already defined", token_str);
                return pe_object_exist;
            }
            if ((*label_info)->func_info != result->func_info) {
                MSG_ERR("label %s is already defined in another scope", token_str);
                return pe_object_exist;
            }
            (*label_info)->line_num = result->line_str.line_num;
            (*label_info)->code_offs = GetCodeOffset(result);
            MSG_DBG(DL_DBG, "Fix label:%d(%s) for line:%d", (*label_info)->id, token_str, (*label_info)->line_num);
            VmOp_Debug1(result, "Label:", (*label_info)->id);
        }
        return pe_no_error;
    } else { /*Not Found */
        char new_object = 1;
        NOT_FOUND_OBJECT(label);
        (*label_info)->id = VECTOR_SIZE(result->labels_vect);
        (*label_info)->func_info = result->func_info;

        if (create_object) {
            (*label_info)->line_num = result->line_str.line_num;
            (*label_info)->code_offs = GetCodeOffset(result);
            VmOp_Debug1(result, "Label:", (*label_info)->id);
        } else {
            (*label_info)->line_num = -1;
            (*label_info)->code_offs = -1;
        }
        MSG_DBG(DL_DBG, "Register label:%d(%s) for line:%d", (*label_info)->id, token_str, (*label_info)->line_num);
    }

    return pe_no_error;
}

/*

*/
parse_error_t RegisterVar(parse_result_t *result, const char *token_str, int array_size, var_set_mode_t mode, var_info_t **var_info) {
    MSG_DBG(DL_TRC, "line:%d  var:%s  array_size:%d  mode:%d", result->line_str.line_num, token_str, array_size, mode);
    /*
        if (array_size < 0) {
            ttype = vt_generic;
            tsize = BITS_TO_BYTES(FPT_BITS );  //
        } else {                     // byte array
            ttype = vt_array_of_byte;
            tsize = array_size + BITS_TO_BYTES(ARRAY_INDEX_BITS);  //+size byte
        }
    */
    FIND_OBJECT(var);
    char new_object = 0;
    var_type_t ttype = vt_generic;
    int tsize;
    switch (mode) {
        case vsm_GET_VAR:  // get generic or array element
            break;
        case vsm_SET_VAR:  // set generic var
            break;
        case vsm_DECLARE_ARRAY:
            ttype = vt_array_of_byte;
            tsize = array_size + 1;  // ARRAY_INDEX_BITS / 8;
            new_object = 1;
            break;
        case vsm_DECLARE_VAR:
            tsize = BITS_TO_BYTES(FPT_BITS);
            new_object = 1;
            break;
        case vsm_SET_ARRAY:
            ttype = vt_array_of_byte;
            break;
        case vsm_GET_ARRAY:
            ttype = vt_array_of_byte;
            break;
        default:
            return pe_syntax_invalid;
    }

    parse_error_t ret = pe_no_error;
    if (found_obj) { /*Found*/
        if (new_object) {
            *var_info = found_obj;
            return pe_object_exist;
        } else {
            *var_info = found_obj;
            if (found_obj->type != ttype) {
                // MSG_ERR("Var: %s already declared", token_str);
                if (new_object) {
                    return pe_var_redeclare;
                }
                return pe_var_incompatible;
            }
            if (array_size > 0 && array_size + 1 >= (*var_info)->size) {
                return pe_array_index_overrange;
            }
        }
        // return pe_no_error;
    } else {  // Not Found
        NOT_FOUND_OBJECT(var);
        (*var_info)->type = ttype;
        (*var_info)->size = tsize;
        (*var_info)->in_block = VECTOR_SIZE(result->block_vect);
        (*var_info)->subs_body = result->subs_body;
        ret = VmOp_VarHeap(result, (*var_info), 1);
        /*(*var_info)->mem_offs = result->vars_memory;
        result->vars_memory += (*var_info)->size;
        if (result->vars_memory > result->vars_memory_max) {
            result->vars_memory_max = result->vars_memory;
        }*/
        MSG_DBG(DL_DBG, "Register var:%s, type:%s, size:%d, subs_body:%d", token_str, array_size < 0 ? "generic" : "array", tsize, (*var_info)->subs_body);
    }

    return ret;
}

parse_error_t UnregisterLastBlockVars(parse_result_t *result) {
    parse_error_t pe = pe_no_error;

    int in_block = VECTOR_SIZE(result->block_vect);
    while (VECTOR_SIZE(result->vars_vect) > 0) {
        var_info_t *last_obj = VECTOR_LAST_PNT(result->vars_vect);
        if (last_obj->in_block == in_block) {
            MSG_DBG(DL_DBG, "UnRegister var:%s, type:%d, size:%d  in_block:%d", last_obj->name, last_obj->type, last_obj->size, in_block);
            pe = VmOp_VarHeap(result, last_obj, 0);
            if (pe) {
                break;
            }
            VECTOR_POP(result->vars_vect);
        } else {
            break;
        }
    }

    return pe;
}

parse_error_t RegisterSub(parse_result_t *result, const char *token_str, char new_object, func_info_t **func_info) {
    FIND_OBJECT(func);

    if (found_obj) { /*Found*/
        FOUND_OBJECT(func);
    } else { /*Not Found */
        NOT_FOUND_OBJECT(func);
        (*func_info)->params_cnt = 0;
        (*func_info)->in_block = VECTOR_SIZE(result->block_vect);
        (*func_info)->func_id = VECTOR_SIZE(result->funcs_vect);

        result->heap_memory_max_back = result->heap_memory_max;
        result->heap_memory_max = 0;
        result->heap_pnt_back = result->heap_pnt;
        result->heap_pnt = 0;
        result->subs_body = 1;
        VmOp_Debug(result, "Begin sub:", token_str);
        (*func_info)->vm_code_offset = GetCodeOffset(result);
        MSG_DBG(DL_DBG, "Register func:%s at line:%d", token_str, result->line_str.line_num);
    }
    return pe_no_error;
}

parse_error_t RegisterConstArray(parse_result_t *result, const char *token_str, uint8_t *data, uint8_t size, char new_object, const_array_info_t **const_array_info) {
    MSG_DBG(DL_TRC, "token_str: %s  new_object:%d  size:%d", token_str, new_object, size);
    char buf[32];
    if (token_str == NULL) {
        if (!new_object) {
            MSG_ERR("Empty token for get object");
            return pe_object_unknown;
        }
        token_str = buf;
        AutoName(result, buf, sizeof(buf), "arr");
    }
    FIND_OBJECT(const_array);

    if (found_obj) { /*Found*/
        FOUND_OBJECT(const_array);
    } else { /*Not Found */
        NOT_FOUND_OBJECT(const_array);
        (*const_array_info)->size = size;
        memcpy((*const_array_info)->data, data, size);
        (*const_array_info)->data[size] = 0;
        (*const_array_info)->mem_offs = result->const_memory;
        result->const_memory += size + 1;
        MSG_DBG(DL_DBG, "Register ConstArray:%s at line:%d", token_str, result->line_str.line_num);
    }
    return pe_no_error;
}

parse_error_t RegisterConstGenVar(parse_result_t *result, const char *token_str, fpt data, char new_object, const_gen_info_t **const_gen_info) {
    MSG_DBG(DL_TRC, "token_str: %s  new_object:%d", token_str, new_object);
    char buf[32];
    if (token_str == NULL) {
        if (!new_object) {
            MSG_ERR("Empty token for get object");
            return pe_object_unknown;
        }
        token_str = buf;
        AutoName(result, buf, sizeof(buf), "var");
    }
    FIND_OBJECT(const_gen);

    if (found_obj) { /*Found*/
        FOUND_OBJECT(const_gen);
    } else { /*Not Found */
        NOT_FOUND_OBJECT(const_gen);
        //(*const_gen_info)->mem_offs = result->const_memory;
        // result->const_memory += BITS_TO_BYTES(FPT_BITS);
        (*const_gen_info)->data = data;
        MSG_DBG(DL_DBG, "Register ConstGenVar:%s at line:%d", token_str, result->line_str.line_num);
    }
    return pe_no_error;
}

// Utils
char ToNextSym(parse_result_t *result, char skip_spaces) {
    // MSG_DBG(DL_TRC, "1 sym_rollback:%d  past_sym:%d  cur_sym:%d", result->line_str.sym_rollback, result->line_str.past_sym, result->line_str.cur_sym);
    if (result->line_str.data[result->line_str.line_pnt_ro] == 0) {
        return 0;
    }
    int r;
    do {
        result->line_str.line_pnt_ro++;
        r = result->line_str.data[result->line_str.line_pnt_ro];
        // MSG_DBG(DL_TRC, "read sym:%d  new_line_fl:%d", r, result->line_str.new_line_fl);
        if (r == 0) {
            break;
        }
        if (r <= ' ') {
            r = ' ';
            if (!skip_spaces) {
                break;
            }
        }

    } while (r <= ' ');
    // MSG_DBG(DL_TRC, "2 sym_rollback:%d  past_sym:%d  cur_sym:%d", result->line_str.sym_rollback, result->line_str.past_sym, result->line_str.cur_sym);
    return r;
}

char GetCurSymSkipSpaces(parse_result_t *result) {
    if (result->line_str.data[result->line_str.line_pnt_ro] > ' ') {
        return result->line_str.data[result->line_str.line_pnt_ro];
    }
    return ToNextSym(result, 1);
}

char UpperCaseChar(char ch) {
    if (VAR_IN_EQ(ch, 'a', 'z')) {
        return ch - 32;
    }
    return ch;
}

void UpperCaseStr(char *str) {
    int pnt = 0;
    while (str[pnt]) {
        str[pnt] = UpperCaseChar(str[pnt]);
        pnt++;
    }
}

void Print_fpt(fpt A) {
    char num[20];

    fpt_str(A, num, -2);
    MSG_DBG(DL_DBG, "NUM value:%s", num);
}

parse_error_t ParseNum(parse_result_t *result, expr_info_t *expr_cur) {
    if (!VAR_IN_EQ(GET_CUR_SKIP_SPACES(), '0', '9')) {
        return pe_number_error_parse;
    }
    expr_cur->value = 0;
#ifdef DEBUG
    int p = 0;
#endif
    int expo = 0;
    int val = 0;
    char is_fract = 0;
    while (CUR_SYM()) {
        if (VAR_IN_EQ(CUR_SYM(), '0', '9')) {
            val *= 10;
            val += CUR_SYM() - '0';
            expo++;
        } else if (CUR_SYM() == '.') {
            is_fract = 1;
            expo = 0;
            // MSG_DBG(DL_DBG1, " NUM (int part):%d", val);
            expr_cur->value = i2fpt(val);
            val = 0;
        } else {
            break;
        }
#ifdef DEBUG
        expr_cur->name[p++] = CUR_SYM();
#endif
        SKIP_SYM();
    }
#ifdef DEBUG
    expr_cur->name[p] = 0;
#endif
    if (is_fract > 0) {
        expr_cur->value += fpt_div(i2fpt(val), i2fpt(_pow(10, expo)));
    } else {
        expr_cur->value = i2fpt(val);
    }
    Print_fpt(expr_cur->value);

    return pe_no_error;
}

// void CopyNameFromToken(parse_result_t *result, char *to) {
//     strncpy(to, from->data, from->size);
//     to[from->size] = 0;
// }

void AutoName(parse_result_t *result, char *str, int size_max, char *suf) {
    snprintf(str, size_max, "#%d_%s%d", result->line_str.line_num, suf, result->line_str.line_pnt_ro);
}

void EndSubContext(parse_result_t *result) {
    result->heap_memory_max_funcs += result->heap_memory_max;
    result->heap_memory_max = result->heap_memory_max_back;
    result->heap_pnt = result->heap_pnt_back;
    result->subs_body = 0;
    VmOp_Debug(result, "End sub:", result->func_info->name);
    result->func_info = NULL;
}

int GetCodeOffset(parse_result_t *result) {
    // if (result->func_info && result->func_info->main_func == 0) {
    if (result->subs_body) {
        return VECTOR_SIZE(result->obj_subs_vect);
    }
    return VECTOR_SIZE(result->obj_main_vect);
}