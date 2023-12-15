#ifndef VM_CODE_H_
#define VM_CODE_H_

#include "parser_pub.h"
#include "parser_priv.h"
#include "../shared/vm_cmd_list.h"

parse_error_t VmOp_Math(parse_result_t *result, vm_ops_list_t op, expr_info_t *l_expr, expr_info_t *r_expr);
parse_error_t VmOp_PopVar(parse_result_t *result, ctx_var_info_t *ctx_var_info);
// void VmOp_PopRet(parse_result_t *result);
// void VmOp_PushRet(parse_result_t *result);
void VmOp_PopFake(parse_result_t *result);
parse_error_t VmOp_Jmp(parse_result_t *result, label_info_t *label_info, char use_condition);
parse_error_t VmOp_Return(parse_result_t *result, func_info_t *func_info);
void VmOp_Return_Value(parse_result_t *result);
void VmOp_ArgNum(parse_result_t *result, fpt value);
// parse_error_t VmOp_ArgPnt(parse_result_t *result, int value);
//  parse_error_t VmOp_ArgArrIndex(parse_result_t *result, int value);
parse_error_t VmOp_ArgConstArray(parse_result_t *result, const_array_info_t *const_array_info);
parse_error_t VmOp_ArgVar(parse_result_t *result, ctx_var_info_t *ctx_var_info);
parse_error_t VmOp_VarHeap(parse_result_t *result, var_info_t *var_info, char alloc);
parse_error_t VmOp_Spc_Cmds(parse_result_t *result);
typedef enum {
    vm_ct_proc = vmo_CALL_INT_PROC,
    vm_ct_int = vmo_CALL_INT_FUNC,
    vm_ct_lib = vmo_CALL_LIB_FUNC,
} vm_call_type;
void VmOp_Call(parse_result_t *result, vm_call_type type, uint8_t fun_id);
void VmOp_CallPrg(parse_result_t *result, func_info_t *func_info);

void VmOp_End(parse_result_t *result);
void VmOp_BreakPoint(parse_result_t *result);

void VmOp_DebugNewLine(parse_result_t *result);
void VmOp_Debug(parse_result_t *result, const char *str, const char *str1);
void VmOp_Debug1(parse_result_t *result, const char *str, int num);

#endif