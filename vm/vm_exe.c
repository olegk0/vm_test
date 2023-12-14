
#include "vm_exe.h"
#include "../shared/vm_cmd_list.h"
#include <unistd.h>
#include <stdlib.h>
#include "../shared/parser_func_tokens.h"

#define CONV_TYPE(cmd, size) size
static uint8_t cmds_size[] = {VM_CMD_LIST};
#undef CONV_TYPE

#define CONV_TYPE(cmd, size) STR(cmd)
static const char *vm_ops_str[] = {VM_CMD_LIST};
#undef CONV_TYPE

#define STACK_SIZE 400
static uint8_t mem_heap[STACK_SIZE + 10];
static int sp;
static int hp;
static int ip;
static int vars_base;
static uint8_t *const_block;

void dump_stack() {
    wclear(stack_window);
    int ll = 0;
    if (sp < 0) {
        sp = 0;
    }
    for (int i = STACK_SIZE - 1; i >= sp; i--) {
        if (ll == 0) {
            wprintw(stack_window, "%.4X:", i);
        }
        wprintw(stack_window, " %.2X", mem_heap[i]);
        ll++;
        if (ll >= 16) {
            wprintw(stack_window, "\n");
            ll = 0;
        }
    }
}

void dump_vars() {
    wclear(vars_window);
    int ll = 0;
    if (hp >= STACK_SIZE) {
        hp = STACK_SIZE - 1;
    }
    for (int i = 0; i < hp; i++) {
        if (ll == 0) {
            wprintw(vars_window, "%.4X:", i);
        }
        wprintw(vars_window, " %.2X", mem_heap[i]);
        ll++;
        if (ll >= 16) {
            wprintw(vars_window, "\n");
            ll = 0;
        }
    }
}

void dump_pnts(int entry_point) {
    wprintw(pnts_window, "%.4X %.4X %.4X %.4X\n", ip, sp, hp, vars_base);
    MSG_DBG(DL_DBG, "ip:0x%X ip_rel:0x%X(%d) sp:0x%X hp:0x%X vars_base:0x%X\n", ip, ip - entry_point, ip - entry_point, sp, hp, vars_base);
}

void dump_code(uint8_t *code_block, int entry_point) {
    // wclear(code_window);
    wprintw(code_window, "%.4X:", ip - entry_point);
#ifdef WRITE_LOG
    fprintf(outLogP, "%.4X:", ip - entry_point);
#endif
    for (int i = 0; i < 8; i++) {
        wprintw(code_window, " %.2X", code_block[i + ip]);
#ifdef WRITE_LOG
        fprintf(outLogP, " %.2X", code_block[i + ip]);
#endif
    }
    wprintw(code_window, "\n");
#ifdef WRITE_LOG
    fprintf(outLogP, "\n");
#endif
}

#if VARS_ADDR_BITS == 16
#define READ_VAR_OFFS(var)                                      \
    int var = (code_block[ip + 1] | (code_block[ip + 2] << 8)); \
    if (cmd_mod) {                                              \
        var += vars_base;                                       \
    }                                                           \
    MSG_INF("offs:0x%X", var);

void push_var_pnt(int addr) {
    // MSG_DBG(DL_TRC, "PUSH pnt:%d", addr);
    mem_heap[sp] = addr & 0xff;
    addr >>= 8;
    sp--;
    mem_heap[sp] = addr & 0xff;
    sp--;
}

int pop_var_pnt() {
    int addr;
    sp++;
    addr = mem_heap[sp];
    addr <<= 8;
    sp++;
    addr |= mem_heap[sp];
    // MSG_DBG(DL_TRC, "POP pnt:%d", addr);
    return addr;
}

void push_var_pnt_to_heap(int addr) {
    // MSG_DBG(DL_TRC, "PUSH pnt:%d", addr);
    mem_heap[hp] = addr & 0xff;
    addr >>= 8;
    hp++;
    mem_heap[hp] = addr & 0xff;
    hp++;
}

int pop_var_pnt_from_heap() {
    int addr;
    hp--;
    addr = mem_heap[hp];
    addr <<= 8;
    hp--;
    addr |= mem_heap[hp];
    // MSG_DBG(DL_TRC, "POP pnt:%d", addr);
    return addr;
}
#else
TODO
#endif

#if PRG_ADDR_BITS == 16
#define CODE_OFFS() \
    (code_block[ip + 1] | (code_block[ip + 2] << 8));

void push_code_addr(int addr) {
    // MSG_DBG(DL_TRC, "PUSH code pnt:%d", addr);
    mem_heap[sp] = addr & 0xff;
    addr >>= 8;
    sp--;
    mem_heap[sp] = addr & 0xff;
    sp--;
}

int pop_code_addr(/*int offset_no_change_sp*/) {
    int addr;
    /*if (offset_no_change_sp) {
        addr = (mem_heap[sp + offset_no_change_sp + 1] << 8) | mem_heap[sp + offset_no_change_sp + 2];
    } else {*/
    sp++;
    addr = mem_heap[sp];
    addr <<= 8;
    sp++;
    addr |= mem_heap[sp];
    //}
    // MSG_DBG(DL_TRC, "POP pnt:%d", addr);
    return addr;
}

void push_code_addr_to_heap(int addr) {
    // MSG_DBG(DL_TRC, "PUSH pnt:%d", addr);
    mem_heap[hp] = addr & 0xff;
    addr >>= 8;
    hp++;
    mem_heap[hp] = addr & 0xff;
    hp++;
}

int pop_code_addr_from_heap() {
    int addr;
    hp--;
    addr = mem_heap[hp];
    addr <<= 8;
    hp--;
    addr |= mem_heap[hp];
    // MSG_DBG(DL_TRC, "POP pnt:%d", addr);
    return addr;
}
#else
    TODO
#endif

fpt pop_num() {
    int val = 0;
    for (int i = 0; i < BITS_TO_BYTES(FPT_BITS); i++) {
        val <<= 8;
        sp++;
        val |= mem_heap[sp];
        MSG_DBG(DL_TRC, "Bpop:%d", mem_heap[sp]);
    }
    MSG_DBG(DL_TRC, "val:%d", fpt2i(val));
    return val;
}

void push_num(fpt value) {
    MSG_DBG(DL_DBG1, "val:%d", fpt2i(value));
    for (int i = 0; i < BITS_TO_BYTES(FPT_BITS); i++) {
        MSG_DBG(DL_DBG1, "sp:%d value:%d", sp, value);
        mem_heap[sp] = value & 0xff;
        MSG_DBG(DL_TRC, "Bpush:%d", mem_heap[sp]);
        value >>= 8;
        sp--;
    }
}

#define MATH_POP()          \
    fpt value2 = pop_num(); \
    fpt value1 = pop_num();
#define MATH_OPT(act, opt)         \
    fpt value = value1 act value2; \
    push_num(value);               \
    MSG_DBG(DL_TRC, #opt ":%d", fpt2i(value));

void print_str(char *pnt, int size) {
    char ss = 0;
    for (int i = 0; i < size; i++) {
        char c = pnt[i];
        if (c == '\\') {
            ss = 1;
            continue;
        }
        if (ss) {
            ss = 0;
            switch (c) {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                default:
                    c = ' ';
            }
        }
        waddch(main_window, c);
    }
}

void print_gen_num(fpt value) {
    char str[32];
    fpt_str(value, str, sizeof(str) - 1);
    wprintw(main_window, "%s", str);
}

uint8_t *get_var_pnt(int *size) {
    uint8_t *pnt;
    sp++;
    uint8_t pnt_type = mem_heap[sp];
    MSG_DBG(DL_TRC, "pnt_type:%d", pnt_type);
    switch (pnt_type) {
        case vvt_const_array_pnt: {
            int addr = pop_var_pnt();
            MSG_DBG(DL_TRC, "addr:0x%X", addr);
            /*const_block[vm_header->const_block_size];*/
            *size = const_block[addr];
            pnt = const_block + addr + 1;
        } break;
        case vvt_inline:
            /*const_block[vm_header->const_block_size];*/
            *size = BITS_TO_BYTES(VARS_ADDR_BITS);
            pnt = mem_heap + sp + 1;
            sp += *size;
            break;
    }
    return pnt;
}

err_code_t call_funcs(uint8_t func_type, uint8_t func_id, fpt *ret_value) {
    err_code_t ret = ec_ok;
    switch (func_type) {
        case vmo_CALL_INT_FUNC:
            switch (func_id) {
                case func_IN: {
                    /*Params:
                    port num*/
                    int port_num = fpt2i(pop_num());
                    MSG_INF("read port:%d", port_num);
                    (*ret_value) = i2fpt(-1);
                } break;
                case func_RAND: {
                    int rtop = fpt2i(pop_num());
                    if (rtop < 0)
                        rtop = 0;
                    (*ret_value) = i2fpt(rand() % (rtop + 1));
                } break;
                case func_INKEY: {
                    int delay_sec = fpt2i(pop_num());
                    if (delay_sec > 0) {
                    }
                    timeout(delay_sec * 1000);
                    //  int c = getch();
                    (*ret_value) = i2fpt(getch());
                } break;
                case func_LEN: {
                    int addr = pop_var_pnt();
                    // MSG_DBG(DL_TRC, "addr:0x%X", addr);
                    (*ret_value) = i2fpt(const_block[addr]);
                } break;
                case func_SCR_W:
                    (*ret_value) = i2fpt(getmaxx(main_window));
                    break;
                case func_SCR_H:
                    (*ret_value) = i2fpt(getmaxy(main_window));
                    break;
                    // without ret
                case func_PUTC: {
                    int ch = fpt2i(pop_num());
                    waddch(main_window, ch);
                } break;
                case func_PUTS: {
                    int size;
                    uint8_t *pnt = get_var_pnt(&size);
                    print_str((char *)pnt, size);
                } break;
                case func_PUTN:
                    print_gen_num(pop_num());
                    break;
                case func_PUTI:
                    print_gen_num(fpt2i(pop_num()));
                    break;
                case func_BEEP: {
                    int freq = fpt2i(pop_num());
                    int len = fpt2i(pop_num());
                    freq = freq;
                    len = len;
                    beep();
                } break;
                case func_SLEEP_MS: {
                    int uS = fpt2i(pop_num());
                    usleep(uS * 1000);
                } break;
                case func_OUT: {
                    int data = fpt2i(pop_num());
                    int port_num = fpt2i(pop_num());
                    MSG_INF("write %d to port:%d", data, port_num);
                } break;
                case func_CLS:
                    wclear(main_window);
                    break;
                case func_CURS: {
                    int c_y = fpt2i(pop_num());
                    int c_x = fpt2i(pop_num());
                    MSG_DBG(DL_DBG, "set cursor to x:%d y:%d", c_x, c_y);
                    wmove(main_window, c_y, c_x);
                } break;
                default:
                    ret = ec_call_unknown_func;
                    MSG_ERR("Unknown int func:%d", func_id);
            }
            break;
        case vmo_CALL_LIB_FUNC:
            switch (func_id) {
                default:
                    ret = ec_call_unknown_func;
                    MSG_ERR("Unknown lib func:%d", func_id);
            }
            break;
        default:
            MSG_ERR("Unknown func type: %d", func_type);
            return ec_call_unknown_func;
    }

    return ret;
}

typedef struct {
    print_part_type_t part_type;
    uint8_t stack_size;
    int stack_offset;
} print_params_t;

void print_from_stack(print_params_t *print_param) {
    MSG_DBG(DL_TRC, "value_type:%d", print_param->part_type);
    switch (print_param->part_type) {
        case print_ss_const: {
            int size;
            uint8_t *pnt = get_var_pnt(&size);
            print_str((char *)pnt, size);
        } break;
        case print_ss_expr:
            print_gen_num(pop_num());
            break;
        default:
    }
}

err_code_t Run(uint8_t *code_block, uint8_t *_const_block, vm_header_t *vm_header, char print_stat) {
    err_code_t ret = ec_ok;
    // int ret_addr = 0;
    int line_num = -1;
    const_block = _const_block;
    ip = vm_header->entry_point;
    hp = 0;
    vars_base = 0;
    sp = STACK_SIZE - 1;
    char by_step = 1;
    while (ip < vm_header->prg_size) {
        if (by_step) {
            mvaddstr(FULL_HEIGHT, 0, "Press 'Enter' to start or 's' to step.");
            // wrefresh(log_window);
            timeout(-1);
            if (getch() == 's') {
                by_step = 1;
            } else {
                by_step = 0;
            }
            clrtobot();
        }
        MSG_DBG(DL_DBG, "Decode cmd:");
        dump_pnts(vm_header->entry_point);
        uint8_t raw_cmd = code_block[ip];
        char cmd_mod = raw_cmd & VM_CMD_MOD;
        uint8_t cmd = raw_cmd & (~VM_CMD_MOD);
        int cmd_size = cmds_size[cmd];
        MSG_INF("%s(0x%X) %s  cmd_size:%d", vm_ops_str[cmd], raw_cmd, cmd_mod ? "MOD" : "", cmd_size);
        if (print_stat) {
            dump_code(code_block, vm_header->entry_point);
        }
        switch (cmd) {
            case vmo_END:
                return ec_ok;
            case vmo_BREAK:
                line_num = CODE_OFFS();
                MSG_INF("Break point at line:%d", line_num);
                // ret = ec_break_point;
                by_step = 1;
                break;
            case vmo_DBG:
                line_num = CODE_OFFS();
                MSG_INF("Line:%d", line_num);
                break;
            case vmo_none:
                MSG_ERR("Unknown instruction:0x%X", cmd);
                return ec_inval_code;
                break;
            case vmo_ALLOC: {
                int size = code_block[ip + 1];
                hp += size;
                MSG_INF("size:%d", size);
            } break;
            case vmo_FREE: {
                int size = code_block[ip + 1];
                hp -= size;
                MSG_INF("size:%d", size);
                if (hp < 0) {
                    MSG_ERR("Heap corrupted");
                    ret = ec_stack_corrupted;
                }
            } break;
            case vmo_LOAD_BYTE: {
                /*
                1. GET var_offset
                2. GET value(byte)
                3. vars [var_offset]  =  value
                */
                READ_VAR_OFFS(var_offs);
                uint8_t byte = code_block[ip + 3];
                // MSG_DBG(DL_TRC, "LOAD_BYTE:%d to %d", byte, var_offs);
                mem_heap[var_offs] = byte;
            } break;
            case vmo_POP_TO_NOWHERE: {
                /*
                1. POP value(gen_num)
                */
                pop_num();
            } break;
            case vmo_POP_BYTE: {
                /*
                1. GET byte_offset
                2. POP value(gen_num)
                3. vars[byte_offset]  =  value( byte)
                */
                READ_VAR_OFFS(var_offs);
                mem_heap[var_offs] = mem_heap[sp + BITS_TO_BYTES(FPT_WBITS)];
                sp += BITS_TO_BYTES(FPT_BITS);
            } break;
            case vmo_POP_VAR: {
                /*
                1. GET var_offset
                2. POP value(gen_num)
                3. vars[var_offset]  =  value
                */
                READ_VAR_OFFS(var_offs);
                for (int i = BITS_TO_BYTES(FPT_BITS) - 1; i >= 0; i--) {
                    sp++;  // stack up
                    mem_heap[var_offs + i] = mem_heap[sp];
                    // MSG_DBG(DL_TRC, "POP_VAR:%d", mem_heap[sp]);
                }
            } break;
            case vmo_POP_bARRAY_BY_IDX: {
                /*
                1. GET array_offset
                2. POP value(gen_num)
                3. POP array_idx(gen_num)
                4. arrays [array_offset + array_idx] =  value
                */
                {
                    READ_VAR_OFFS(var_offs);
                    int arr_idx = mem_heap[sp + BITS_TO_BYTES(FPT_BITS) + BITS_TO_BYTES(FPT_WBITS)];  // ARRAY_INDEX_BITS == 8
                    uint8_t val = mem_heap[sp + BITS_TO_BYTES(FPT_WBITS)];
                    mem_heap[var_offs + arr_idx + 1] = val;
                    // printf("var_offs: %d, arr_idx: %d, value: %d\n", var_offs, arr_idx, val);
                    sp += BITS_TO_BYTES(FPT_BITS) * 2;  // arr_idx and value
                }
            } break;
            case vmo_PUSH_NUM:
                /*
                1. GET value(gen_num)
                2. PUSH value
                */
                for (int i = 1; i <= BITS_TO_BYTES(FPT_BITS); i++) {
                    mem_heap[sp] = code_block[ip + i];
                    MSG_DBG(DL_TRC, "PUSH_NUM:%d", mem_heap[sp]);
                    sp--;  // stack down
                }
                break;
            case vmo_PUSH_PNT: {
                /*
                1. GET const_offset
                2. GET const_type
                3. PUSH const_offset(gen_num)
                4. PUSH const_type(byte)
                */
                mem_heap[sp] = code_block[ip + 1];  // offs1
                sp--;
                mem_heap[sp] = code_block[ip + 2];  // offs2
                sp--;
                mem_heap[sp] = code_block[ip + 3];  // type
                // MSG_DBG(DL_TRC, "type:%d", mem_heap[sp]);
                MSG_INF("offs:%d  type:%d", (code_block[ip + 2] << 8) | code_block[ip + 1], code_block[ip + 3]);
                sp--;
            } break;
            case vmo_PUSH_VAR: {
                /*
                1. GET var_offset
                2. PUSH vars[var_offset](gen_num)
                */
                READ_VAR_OFFS(var_offs);
                int val = 0;
                for (int i = 0; i < BITS_TO_BYTES(FPT_BITS); i++) {
                    mem_heap[sp] = mem_heap[var_offs + i];
                    val |= mem_heap[sp] << (8 * i);
                    // MSG_INF("PUSH_VAR:%d  %d", val, mem_heap[sp]);
                    sp--;  // stack down
                }
                MSG_INF("value:%d", val >> 8);
            } break;
            case vmo_PUSH_BYTE: {
                /*
                1. GET byte_offset
                2. PUSH vars[byte_offset] (as gen_num)
                */
                READ_VAR_OFFS(var_offs);
                push_num(i2fpt(mem_heap[var_offs]));
            } break;
            case vmo_PUSH_bARRAY_BY_IDX: {
                /*
                1. GET array_offset
                2. POP array_idx(gen_num)
                3. PUSH arrays[array_offset + array_idx]
                */
                {
                    READ_VAR_OFFS(var_offs);
                    int arr_idx = mem_heap[sp + BITS_TO_BYTES(FPT_WBITS)];  // ARRAY_INDEX_BITS == 8
                    sp += BITS_TO_BYTES(FPT_BITS);                          // pop arr_idx
                    uint8_t val = mem_heap[var_offs + arr_idx + 1];
                    MSG_DBG(DL_TRC, "arr  offs:%d   idx:%d  val:%d", var_offs, arr_idx, val);
                    push_num(i2fpt(val));
                }
            } break;
            case vmo_JMP: {
                /* internal procedures
                1. GET offs(VARS_ADDR_BITS)
                2. Jmp offs
                */
                int16_t var_offs = CODE_OFFS();
                MSG_INF("jmp2offs:%d", var_offs);
                ip += var_offs;
            } break;
            case vmo_JMP_POP_CMPZ: {
                /* internal procedures
                1. GET offs(VARS_ADDR_BITS)
                2. POP var(gen_num)
                3. If var == 0: Jmp offs
                */
                int16_t var_offs = CODE_OFFS();
                fpt value = pop_num();
                MSG_INF("value:%d  jmp2offs:%d", value, var_offs);
                if (!value) {
                    ip += var_offs;
                }
            } break;
            case vmo_CALL_INT_PROC: {
                /* internal procedures
                1. GET func_id(byte)
                2. Call func
                */
                uint8_t func_id = code_block[ip + 1];
                MSG_INF("func_id:%d", func_id);
                switch (func_id) {
                    case cmd_id_PRINT:
                    case cmd_id_PRINT_LN: {
                        /*
                        1. POP pnt_to_args_list
                        2. POP argN
                        3. print argN
                        ...
                        */
                        int size;
                        uint8_t *pnt = get_var_pnt(&size);
                        MSG_DBG(DL_TRC, "PRINT size:%d  cont:%X %X %X", size, pnt[0], pnt[1], pnt[2]);
                        print_params_t print_params[size * 2];
                        int pt = size - 1;
                        int stack_offs = 0;
                        for (int i = size * 2 - 1; i >= 0; i--) {
                            if (i & 1) {
                                print_params[i].part_type = pnt[pt] & 0xf;
                            } else {
                                print_params[i].part_type = pnt[pt] >> 4;
                                pt--;
                            }
                            if (print_params[i].part_type) {
                                if (print_params[i].part_type & print_ss_POINTERS) {
                                    print_params[i].stack_size = BITS_TO_BYTES(VARS_ADDR_BITS) + 1;
                                } else {
                                    print_params[i].stack_size = BITS_TO_BYTES(FPT_BITS);
                                }
                                print_params[i].stack_offset = stack_offs;
                                MSG_DBG(DL_TRC, "i:%d  part_type:%d stack_size:%d  stack_offset:%d", i, print_params[i].part_type, print_params[i].stack_size, print_params[i].stack_offset);
                                stack_offs += print_params[i].stack_size;
                            }
                        }
                        dump_stack();
                        int sp_back = sp;
                        for (int i = 0; i < size * 2; i++) {
                            sp = sp_back + print_params[i].stack_offset;
                            print_from_stack(&print_params[i]);
                        }
                        sp = sp_back + stack_offs;
                        if (func_id == cmd_id_PRINT_LN) {
                            waddch(main_window, '\n');
                        }
                    } break;
                    default:
                        ret = ec_call_unknown_func;
                }
                // ret = ec_call_unknown_func;
            } break;
            case vmo_RET:
                // ip = ret_addr;
                //  ip = pop_code_addr(BITS_TO_BYTES(FPT_BITS));
                ip = pop_code_addr_from_heap();  // return address
                vars_base = pop_var_pnt_from_heap();
                cmd_size = 0;
                break;
            /*case vmo_POP_RET:
                ret_addr = pop_code_addr();
                break;
            case vmo_PUSH_RET:
                push_code_addr(ret_addr);
                break;*/
            case vmo_CALL_PRG_FUNC: {
                /* internal procedures
                1. GET func_offs
                2. Call func_offs
                */
                int16_t func_offs = CODE_OFFS();
                MSG_DBG(DL_DBG, "func_offs:%d", func_offs);
                // ret_addr = ip + cmd_size;
                push_var_pnt_to_heap(vars_base);
                push_code_addr_to_heap(ip + cmd_size);  // return address
                ip = func_offs;
                vars_base = hp;
                cmd_size = 0;
            } break;
            case vmo_CALL_INT_FUNC:
            case vmo_CALL_LIB_FUNC: {
                /* internal procedures
                1. GET func_id(byte)
                2. Call func
                3. if have ret PUSH result
                */
                uint8_t func_id = code_block[ip + 1];
                MSG_INF("func_id:%d", func_id);
                fpt ret_value;
                ret = call_funcs(cmd, func_id, &ret_value);
                if (func_id & FUNC_WITH_RETURN_FL) {
                    // MSG_INF("push ret:%d", fpt2i(ret_value));
                    push_num(ret_value);
                }
            } break;
            case vmo_MINUS: {
                /*
                1. POP value2(gen_num)
                2. POP value1(gen_num)
                3. PUSH (value1 - value2)
                */
                MATH_POP();
                MATH_OPT(-, MINUS);
            } break;
            case vmo_PLUS: {
                MATH_POP();
                MATH_OPT(+, PLUS);
            } break;
            case vmo_MUL: {
                MATH_POP();
                fpt value = fpt_xmul(value1, value2);
                push_num(value);
                MSG_DBG(DL_TRC, " MUL:%d", fpt2i(value));
            } break;
            case vmo_DIV: {
                MATH_POP();
                if (value2) {
                    fpt value = fpt_xdiv(value1, value2);
                    push_num(value);
                    MSG_DBG(DL_TRC, "DIV:%d", fpt2i(value));
                } else {
                    ret = ec_div_except;
                }
            } break;
            case vmo_LE: {
                MATH_POP();
                MATH_OPT(<=, LE);
            } break;
            case vmo_GE: {
                MATH_POP();
                MATH_OPT(>=, GE);
            } break;
            case vmo_LT: {
                MATH_POP();
                MATH_OPT(<, LT);
            } break;
            case vmo_GT: {
                MATH_POP();
                MATH_OPT(>, GT);
            } break;
            case vmo_AND: {
                MATH_POP();
                MATH_OPT(&&, AND);
            } break;
            case vmo_OR: {
                MATH_POP();
                MATH_OPT(||, OR);
            } break;
            case vmo_NOT: {
                /*
                1. POP value
                3. PUSH !value (gen_num)
                */
                fpt value = pop_num();
                value = !value;
                push_num(value);
                MSG_DBG(DL_TRC, "INV:%d", fpt2i(value));
            } break;
            case vmo_INV: {
                /*
                1. POP value
                3. PUSH -value (gen_num)
                */
                fpt value = pop_num();
                value = -value;
                push_num(value);
                MSG_DBG(DL_TRC, "INV:%d", fpt2i(value));
            } break;
            case vmo_EQ: {
                MATH_POP();
                MATH_OPT(==, EQ);
            } break;
            case vmo_NE: {
                MATH_POP();
                MATH_OPT(!=, NE);
            } break;
            default:
                MSG_ERR("Unknown instruction:0x%X", cmd);
                ret = ec_inval_code;
        }
        MSG_DBG(DL_TRC, "=====");
        if (print_stat) {
            // dump_pnts(vm_header->entry_point);
            dump_vars();
            dump_stack();
            wrefresh(stack_window);
            wrefresh(vars_window);
            wrefresh(log_window);
            wrefresh(code_window);
            wrefresh(pnts_window);
        }
        wrefresh(main_window);
        if (hp >= sp || sp >= STACK_SIZE) {
            MSG_ERR("Stack overflow");
            ret = ec_stack_corrupted;
            break;
        }
        if (ret) {
            if (ret == ec_break_point) {
                MSG_INF("Break point at line: %d", line_num);
            }
            break;
        }
        ip += cmd_size;
        //  sleep(1);
    }
    //  const_block[vm_header->const_block_size];
    //  code_block[vm_header->prg_size];

    return ret;
}
