#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "parser_gen.h"
#include "parser_reg.h"
#include <stdlib.h>
#include "vm_code.h"

int ParseInit(parse_result_t *result) {
    memset(result, 0, sizeof(parse_result_t));
    // result->labels_max = labels_max;
    // result->vars_max = vars_max;
    //  VECTOR_INIT(result->labels_vect);
    //  VECTOR_INIT(result->vars_vect);
    //   VECTOR_NEW(block_info_t, result->block_vect);
    return 0;
}

void ParseFree(parse_result_t *result) {
    VECTOR_FREE(result->labels_vect);
    VECTOR_FREE(result->vars_vect);
    // VECTOR_FREE(result->funcs_vect.data->vars_vect);
    VECTOR_FREE(result->funcs_vect);
    VECTOR_FREE(result->const_gens_vect);
    VECTOR_FREE(result->const_arrays_vect);
    VECTOR_FREE(result->block_vect);

    // VECTOR_FREE(result->line_pnts_vect);
    VECTOR_FREE(result->obj_main_vect);
    VECTOR_FREE(result->obj_subs_vect);
}

parse_error_t ParseLine(parse_result_t *result, int line_num, char *line, int line_len) {
    result->line_str.data = line;
    result->line_str.line_num = line_num;
    result->line_str.line_pnt_ro = 0;
    result->line_str.line_len = line_len;
    result->early_opened_block = FALSE;
    result->cmd_templ_pnt = NULL;
    result->cur_cmd = cmd_id_NOP;
    // result->past_cmd = cmd_ext_id_NOP;
    result->params_lvl = 0;
    result->optional_mod = 0;
    result->group_mod = 0;
    result->enable_code_gen = TRUE;
    // result->enable_code_gen_back = 1;
    //    result->var_info = NULL;
    //  result->func_info = NULL;
    result->ctx_var_info.array_compile_time_idx = -1;
    result->ctx_var_info.var_info = NULL;
    result->params_str.params_cnt = 0;

    if (GET_CUR_SKIP_SPACES() <= ' ') {
        return pe_no_error;
    }
    // MSG_DBG(DL_TRC, " >%c(%d))", CUR_SYM(), CUR_SYM());
    if (CUR_SYM() == '#') {  // remark
        return pe_no_error;
    }

    parse_error_t pe = GetToken(result);
    RETURN_ON_ERROR(pe);
    MSG_DBG(DL_DBG1, "token.size:%d", result->token.size);
    if (result->token.size) {
        if (CUR_SYM() == ':') {  // label
            label_info_t *label_info;
            pe = RegisterLabel(result, result->token.data, 1, &label_info);
            SKIP_SYM();
            if (GET_CUR_SKIP_SPACES() == 0) {
                return pe_no_error;
            }
            TOKEN_INVALIDATE();
        }
    }

    VmOp_DebugNewLine(result);

    pe = ParseStep(result, pcs_command);
    if (pe) {
        PRINT_ERROR(pe);
    } else {
        pe = VmOp_Spc_Cmds(result);
    }
    return pe;
}
