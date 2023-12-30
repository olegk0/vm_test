
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

#define STACK_SIZE_MIN 200
static uint8_t *mem_heap;
static int stack_size;
static int sp;
static int hp;
static int ip;
static int vars_base;
static uint8_t *const_block;
static int line_num = -1;

void dump_stack() {
    wclear(stack_window);
    int ll = 0;
    if (sp < 0) {
        sp = 0;
    }
    for (int i = stack_size - 1; i >= sp; i--) {
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
    if (hp >= stack_size) {
        hp = stack_size - 1;
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
    MSG_DBG(DL_DBG, "line:%d ip:0x%X ip_rel:0x%X(%d) sp:0x%X hp:0x%X vars_base:0x%X", line_num, ip, ip - entry_point, ip - entry_point, sp, hp, vars_base);
}

void dump_code(uint8_t *code_block, int entry_point) {
    // wclear(code_window);
    int l_ip = ip - entry_point;
    if (l_ip < 0) {
        l_ip = ip;
    }
    wprintw(code_window, "(%d)%.4X:", line_num, l_ip);
#ifdef WRITE_LOG
    fprintf(outLogP, "%.4X:", l_ip);
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

void var_zerro(int addr, int size) {
    MSG_DBG(DL_TRC, "addr:%d  size:%d", addr, size);
    // if (hp < addr + size) {
    //}
    for (int i = 0; i < size; i++) {
        mem_heap[addr + i] = 0;
    }
}

#if VARS_ADDR_BITS == 16
int read_var_addr(uint8_t *pnt) {  // 0-low, 1-hi
    int var = (pnt[0] | (pnt[1] << 8));
    return var;
}

void write_var_addr(uint8_t *pnt, int addr) {  // 0-low, 1-hi
    pnt[0] = addr;
    pnt[1] = addr >> 8;
}

void write_var_addr_reverse(uint8_t *pnt, int addr) {  // 1-hi, 0-low
    pnt[0] = addr >> 8;
    pnt--;
    pnt[0] = addr;
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

int read_var_offset(uint8_t *code_block, uint8_t cmd_mod) {
    int var = read_var_addr(code_block + ip + 1);
    if (cmd_mod) {
        var += vars_base;
    }
    MSG_DBG(DL_DBG1, "offs:0x%X", var);
    return var;
}

void push_var_addr(int addr) {
    // MSG_DBG(DL_TRC, "PUSH pnt:%d", addr);
    write_var_addr_reverse(&mem_heap[sp], addr);
    sp -= BITS_TO_BYTES(VARS_ADDR_BITS);
}

int pop_var_addr() {
    int addr = read_var_addr(&mem_heap[sp + 1]);
    sp += BITS_TO_BYTES(VARS_ADDR_BITS);
    // MSG_DBG(DL_TRC, "POP pnt:%d", addr);
    return addr;
}

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

void push_var(int var_offs) {
    // int val = 0;
    for (int i = 0; i < BITS_TO_BYTES(FPT_BITS); i++) {
        mem_heap[sp] = mem_heap[var_offs + i];
        //  MSG_INF("PUSH_VAR:%d  %d", val, mem_heap[sp]);
        sp--;  // stack down
    }
}

void pop_var(int var_offs) {
    for (int i = BITS_TO_BYTES(FPT_BITS) - 1; i >= 0; i--) {
        sp++;  // stack up
        mem_heap[var_offs + i] = mem_heap[sp];
        // MSG_DBG(DL_TRC, "POP_VAR:%d", mem_heap[sp]);
    }
}

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

void print_gen_num(fpt value) {
    char str[32];
    fpt_str(value, str, sizeof(str) - 1);
    wprintw(main_window, "%s", str);
}

void print_str(uint8_t *pnt, vm_var_type_t pnt_type, int size) {
    switch (pnt_type) {
        case vvt_var_generic_array_pnt:
        case vvt_const_generic_array_pnt: {
            waddch(main_window, '{');
            fpt value;
            int p = 0;
            for (int i = 0; i < size; i++) {
                value = 0;
                for (int n = 0; n < BITS_TO_BYTES(FPT_BITS); n++) {
                    value |= pnt[p] << (n * 8);
                    p++;
                }
                print_gen_num(value);
                waddch(main_window, ',');
            }
            waddch(main_window, '}');
        } break;
        case vvt_var_char_array_pnt:
        case vvt_const_string_pnt: {
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
        } break;
        // case vvt_var_byte_array_pnt:
        // case vvt_const_byte_array_pnt:
        default: {  // byte array
            waddch(main_window, '{');
            for (int i = 0; i < size; i++) {
                wprintw(main_window, "%d", pnt[i]);
                waddch(main_window, ',');
            }
            waddch(main_window, '}');
        }
    }
}

uint8_t *pop_var_pnt(int *size, vm_var_type_t *pnt_type) {
    uint8_t *pnt = NULL;
    sp++;
    *pnt_type = mem_heap[sp] & 0xf0;
    mem_heap[sp] = mem_heap[sp] & 0xf;
    int addr = mem_heap[sp] << VARS_ADDR_BITS;
    MSG_DBG(DL_DBG1, "pnt_type:%d", *pnt_type);
    switch (*pnt_type) {
        case vvt_const_string_pnt:
        case vvt_const_generic_array_pnt:
        case vvt_const_byte_array_pnt: {
            addr |= pop_var_addr();
            MSG_DBG(DL_DBG1, "const addr:0x%X", addr);
            /*const_block[vm_header->const_block_size];*/
            *size = const_block[addr];
            pnt = const_block + addr + 1;
        } break;
        case vvt_var_generic_array_pnt:
        case vvt_var_byte_array_pnt:
        case vvt_var_char_array_pnt: {
            addr |= pop_var_addr();
            MSG_DBG(DL_DBG1, "var addr:0x%X", addr);
            /*const_block[vm_header->const_block_size];*/
            *size = mem_heap[addr];
            pnt = mem_heap + addr + 1;
        } break;
        case vvt_inline: {
            /*const_block[vm_header->const_block_size];*/
            *size = BITS_TO_BYTES(VARS_ADDR_BITS);
            pnt = mem_heap + sp;
            // MSG_DBG(DL_DBG1, "inline pnt:0x%X", pnt);
            uint8_t t = pnt[1];  // TODO
            pnt[1] = pnt[2];
            pnt[2] = t;
            sp += *size;
            (*size)++;
        } break;
        default:
            *size = 0;
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
                case func_SIZE: {
                    int size;
                    vm_var_type_t pnt_type;
                    pop_var_pnt(&size, &pnt_type);
                    // MSG_DBG(DL_TRC, "addr:0x%X", addr);
                    //(*ret_value) = i2fpt(const_block[addr]);
                    (*ret_value) = i2fpt(size);
                } break;
                case func_SCR_W:
                    (*ret_value) = i2fpt(getmaxx(main_window));
                    break;
                case func_SCR_H:
                    (*ret_value) = i2fpt(getmaxy(main_window));
                    break;
                case func_SCR_SYM: {
                    int c_y = fpt2i(pop_num());
                    int c_x = fpt2i(pop_num());
                    // MSG_DBG(DL_DBG, "set cursor to x:%d y:%d", c_x, c_y);
                    wmove(main_window, c_y, c_x);
                    (*ret_value) = i2fpt(winch(main_window) & A_CHARTEXT);
                } break;
                    // without ret
                case func_PUTC: {
                    int ch = fpt2i(pop_num());
                    waddch(main_window, ch);
                } break;
                case func_PUTS: {
                    int size;
                    vm_var_type_t pnt_type;
                    uint8_t *pnt = pop_var_pnt(&size, &pnt_type);
                    print_str(pnt, pnt_type, size);
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
                    // MSG_DBG(DL_DBG, "set cursor to x:%d y:%d", c_x, c_y);
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
        case print_ss_pointer: {
            MSG_DBG(DL_DBG1, "print_ss_pointer");
            int size;
            vm_var_type_t pnt_type;
            uint8_t *pnt = pop_var_pnt(&size, &pnt_type);
            MSG_DBG(DL_DBG1, "pnt_type:%d  size:%d", pnt_type, size);
            print_str(pnt, pnt_type, size);
        } break;
        case print_ss_number:
            MSG_DBG(DL_DBG1, "print_ss_expr");
            print_gen_num(pop_num());
            break;
        default:
    }
}

err_code_t Run(uint8_t *code_block, uint8_t *_const_block, vm_header_t *vm_header, char print_stat) {
    err_code_t ret = ec_ok;
    // int ret_addr = 0;
    int recurs_cnt = 0;
    stack_size = vm_header->heap_min_size * 2;
    if (stack_size < STACK_SIZE_MIN) {
        stack_size = STACK_SIZE_MIN;
    }
    uint8_t heap[stack_size + BITS_TO_BYTES(FPT_BITS) * 3];
    mem_heap = heap + BITS_TO_BYTES(FPT_BITS) * 2;
    for (int i = 0; i < BITS_TO_BYTES(FPT_BITS) * 2; i++) {
        heap[i] = 0;
    }
    const_block = _const_block;
    ip = vm_header->entry_point;
    hp = 0;
    vars_base = 0;
    sp = stack_size - 1;
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
        MSG_DBG(DL_DBG1, "Decode cmd:");
        // dump_pnts(vm_header->entry_point);
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
                ret = ec_end;
                break;
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
            case vmo_INIT_gVAR:
            case vmo_INIT_bVAR: {
                /*
                1. GET var_offset
                2. GET size(byte)
                3.
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                uint8_t size = code_block[ip + BITS_TO_BYTES(VARS_ADDR_BITS) + 1];
                int tsize = size;
                int tvar_offs = var_offs;
                if (tsize > 0) {  // array
                    tvar_offs++;  // skip size byte
                } else {          // gen var
                    tsize = 1;
                }
                if (cmd == vmo_INIT_gVAR) {
                    hp = tvar_offs + tsize * BITS_TO_BYTES(FPT_BITS);
                    // var_zerro(tvar_offs, tsize * BITS_TO_BYTES(FPT_BITS));
                } else {
                    hp = tvar_offs + tsize;
                    // var_zerro(tvar_offs, tsize);
                }
                if (size > 0) {  // array. init
                    mem_heap[var_offs] = size;
                }
                MSG_INF("size:%d", size);
            } break;
            case vmo_LOAD_BYTE: {
                /*
                1. GET var_offset
                2. GET value(byte)
                3. vars [var_offset]  =  value
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                uint8_t byte = code_block[ip + BITS_TO_BYTES(VARS_ADDR_BITS) + 1];
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
                int var_offs = read_var_offset(code_block, cmd_mod);
                mem_heap[var_offs] = mem_heap[sp + BITS_TO_BYTES(FPT_WBITS)];
                sp += BITS_TO_BYTES(FPT_BITS);
            } break;
            case vmo_POP_VAR: {
                /*
                1. GET var_offset
                2. POP value(gen_num)
                3. vars[var_offset]  =  value
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                pop_var(var_offs);
            } break;
            case vmo_POP_PNT: {
                /*
                1. GET var_offset
                2. POP to var
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                // stack pointer
                // type,var absolute offset
                sp++;
                uint8_t pnt_type = mem_heap[sp];
                int pnt_offs = pop_var_addr();
                mem_heap[var_offs] = pnt_type;
                write_var_addr(&mem_heap[var_offs + 1], pnt_offs);
                //  MSG_DBG(DL_DBG, "offs:0x%X", var_offs);
            } break;
            case vmo_POP_bARRAY_BY_IDX: {
                /*
                1. GET array_offset
                2. POP value(gen_num)
                3. POP array_idx(gen_num)
                4. arrays [array_offset + array_idx] =  value
                */
                {
                    int var_offs = read_var_offset(code_block, cmd_mod);
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
                    MSG_DBG(DL_TRC, "num:%d", mem_heap[sp]);
                    sp--;  // stack down
                }
                break;
            case vmo_PUSH_PNT: {
                /*
                1. GET pnt_var offset
                2. PUSH const_offset(gen_num)
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                // var pointer
                // type,h-byte, l-byte
                int pnt_offs = read_var_addr(&mem_heap[var_offs + 1]);
                push_var_addr(pnt_offs);
                mem_heap[sp] = mem_heap[var_offs];
                sp--;
                // MSG_DBG(DL_DBG1, "offs:%d  type:%d", (code_block[ip + 2] << 8) | code_block[ip + 1], code_block[ip + 3]);
            } break;
            case vmo_PUSH_OBJ_PNT: {
                /*
                1. GET obj_offset
                2. GET obj_type
                3. PUSH obj_offset,obj_type
                */
                int var_offs = read_var_offset(code_block, cmd_mod);  // pointer is always an absolute address
                uint8_t type = code_block[ip + BITS_TO_BYTES(VARS_ADDR_BITS) + 1];
                push_var_addr(var_offs);
                mem_heap[sp] = type;
                sp--;
                MSG_DBG(DL_DBG1, "offs:%d  type:%d", (code_block[ip + 2] << 8) | code_block[ip + 1], code_block[ip + 3]);

            } break;
            case vmo_PUSH_VAR: {
                /*
                1. GET var_offset
                2. PUSH vars[var_offset](gen_num)
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                push_var(var_offs);
            } break;
            case vmo_PUSH_BYTE: {
                /*
                1. GET byte_offset
                2. PUSH vars[byte_offset] (as gen_num)
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                push_num(i2fpt(mem_heap[var_offs]));
            } break;
            case vmo_PUSH_bARRAY_BY_IDX: {
                /*
                1. GET array_offset
                2. POP array_idx(gen_num)
                3. PUSH arrays[array_offset + array_idx]
                */
                int var_offs = read_var_offset(code_block, cmd_mod);
                int arr_idx = mem_heap[sp + BITS_TO_BYTES(FPT_WBITS)];  // ARRAY_INDEX_BITS == 8
                sp += BITS_TO_BYTES(FPT_BITS);                          // pop arr_idx
                uint8_t val = mem_heap[var_offs + arr_idx + 1];
                MSG_DBG(DL_TRC, "arr  offs:%d   idx:%d  val:%d", var_offs, arr_idx, val);
                push_num(i2fpt(val));
            } break;
            case vmo_COPY_TO_bARRAY: {
                /*
                1. GET dest_array_offset
                2. POP src_array_pnt(pnt with type)
                3. Copy
                */
                int dest_array_offset = read_var_offset(code_block, cmd_mod);
                int src_array_size;
                vm_var_type_t pnt_type;
                uint8_t *src_array_pnt = pop_var_pnt(&src_array_size, &pnt_type);
                int dest_array_size = mem_heap[dest_array_offset];
                dest_array_offset++;
                switch (pnt_type) {
                    case vvt_var_generic_array_pnt:
                    case vvt_const_generic_array_pnt: {
                        int g_p = BITS_TO_BYTES(FPT_WBITS) - 2;
                        for (int i = 0; i < dest_array_size && i < src_array_size; i++) {
                            mem_heap[dest_array_offset + i] = src_array_pnt[g_p];
                            g_p += BITS_TO_BYTES(FPT_BITS);
                        }
                    } break;
                    default: {
                        for (int i = 0; i < dest_array_size && i < src_array_size; i++) {
                            mem_heap[dest_array_offset + i] = src_array_pnt[i];
                        }
                    }
                }
            } break;
            case vmo_COPY_TO_gARRAY: {
                /*
                1. GET dest_array_offset
                2. POP src_array_pnt(pnt with type)
                3. Copy
                */
                int dest_array_offset = read_var_offset(code_block, cmd_mod);
                int src_array_size;
                vm_var_type_t pnt_type;
                uint8_t *src_array_pnt = pop_var_pnt(&src_array_size, &pnt_type);
                int dest_array_size_bytes = mem_heap[dest_array_offset] * BITS_TO_BYTES(FPT_BITS);
                dest_array_offset++;
                switch (pnt_type) {
                    case vvt_var_generic_array_pnt:
                    case vvt_const_generic_array_pnt: {
                        // MSG_DBG(DL_DBG, "dst_array_pnt:%p  src_array_pnt:%p", dst_array_pnt, src_array_pnt);
                        int src_array_size_bytes = src_array_size * BITS_TO_BYTES(FPT_BITS);
                        for (int i = 0; i < dest_array_size_bytes && i < src_array_size_bytes; i++) {
                            mem_heap[dest_array_offset + i] = src_array_pnt[i];
                        }
                    } break;
                    default: {  // from byte array
                        int src_idx = 0;
                        int i = 1;
                        for (int p = 0; p < dest_array_size_bytes && src_idx < src_array_size; p++) {
                            switch (i) {
                                case BITS_TO_BYTES(FPT_WBITS) - 1:
                                    mem_heap[dest_array_offset + p] = src_array_pnt[src_idx];
                                    src_idx++;
                                    break;
                                case BITS_TO_BYTES(FPT_BITS):
                                    i = 0;
                                default:
                                    mem_heap[dest_array_offset + p] = 0;
                            }

                            i++;
                        }
                    }
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
                        vm_var_type_t pnt_type;
                        MSG_DBG(DL_TRC, "sp:%d", sp);
                        uint8_t *pnt = pop_var_pnt(&size, &pnt_type);
                        MSG_DBG(DL_DBG1, "PRINT sp:%d  size:%d  cont:%X %X %X", sp, size, pnt[0], pnt[1], pnt[2]);
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
                                MSG_DBG(DL_DBG1, "i:%d  part_type:%d stack_size:%d  stack_offset:%d", i, print_params[i].part_type, print_params[i].stack_size, print_params[i].stack_offset);
                                stack_offs += print_params[i].stack_size;
                            }
                        }
                        // dump_stack();
                        // return ec_call_unknown_func;
                        int sp_back = sp;
                        for (int i = 0; i < size * 2; i++) {
                            sp = sp_back + print_params[i].stack_offset;
                            print_from_stack(&print_params[i]);
                        }
                        sp = sp_back + stack_offs;
                        MSG_DBG(DL_TRC, "-sp:%d", sp);
                        if (func_id == cmd_id_PRINT_LN) {
                            waddch(main_window, '\n');
                        }
                    } break;
                    default:
                        ret = ec_call_unknown_func;
                }
                // ret = ec_call_unknown_func;
            } break;
            case vmo_RETVAL:
                pop_var(vars_base - BITS_TO_BYTES(FPT_BITS));  // store return value (to special place in heap) from stack
                // break;
            case vmo_RET:
                if (recurs_cnt > 0) {
                    hp = vars_base - BITS_TO_BYTES(FPT_BITS);
                    push_var(hp);                         // move return value from special place in heap to stack
                    ip = pop_code_addr_from_heap();       // restore return address from heap
                    vars_base = pop_var_pnt_from_heap();  // retore vars_base from heap
                    cmd_size = 0;
                    recurs_cnt--;
                } else {  // exit from programm
                    ret = ec_exit;
                }
                break;
            case vmo_CALL_PRG_FUNC: {
                /* internal procedures
                1. GET func_offs
                2. Call func_offs
                */
                int16_t func_offs = CODE_OFFS();
                MSG_DBG(DL_DBG, "func_offs:%d", func_offs);
                // ret_addr = ip + cmd_size;
                push_var_pnt_to_heap(vars_base);        // store vars_base to heap
                push_code_addr_to_heap(ip + cmd_size);  // store return address to heap
                hp += BITS_TO_BYTES(FPT_BITS);          // reserve for return value
                ip = func_offs;
                vars_base = hp;
                cmd_size = 0;
                recurs_cnt++;
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
        if (hp >= sp || sp >= stack_size) {
            MSG_ERR("Stack overflow");
            ret = ec_stack_corrupted;
        }
        if (print_stat || ret) {
            dump_pnts(vm_header->entry_point);
            dump_vars();
            dump_stack();
            wrefresh(stack_window);
            wrefresh(vars_window);
            wrefresh(log_window);
            wrefresh(code_window);
            wrefresh(pnts_window);
        }
        wrefresh(main_window);

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
