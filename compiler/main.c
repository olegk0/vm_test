#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "vm_code.h"

static FILE *outLstP;
static void put_sym(uint8_t sym) {
    fputc(sym, outLstP);
}

void fix_code_num(parse_result_t *result, char subs_body, int code_offset, int val, char bytes) {
    uint8_t *cpnt;
    if (subs_body) {
        cpnt = &result->obj_subs_vect.data[code_offset];
    } else {
        cpnt = &result->obj_main_vect.data[code_offset];
    }
    val = val - code_offset - BITS_TO_BYTES(PRG_ADDR_BITS);
    while (bytes) {
        *cpnt = val & 0xff;
        // result->vm_code_offset++;
        val >>= 8;
        bytes--;
        cpnt++;
    }
}

int main(int argc, char **argv) {
    int ret = 0;
    if (argc < 2) {
        printf("no file\n");
        return 1;
    }

    FILE *filePointer = fopen(argv[1], "r");
    if (filePointer == NULL) {
        printf("wrong file:%s\n", argv[1]);
        return 1;
    }

    char *file_name;
    (file_name = strrchr(argv[1], '/')) ? file_name++ : (file_name = argv[1]);
    char *file_ext = strrchr(file_name, '.');
    if (file_ext) {
        *file_ext = 0;
    }

    char namebuf[strlen(file_name) + 10];
    snprintf(namebuf, sizeof(namebuf), "%s.lst", file_name);
    outLstP = fopen(namebuf, "w");
    if (outLstP == NULL) {
        printf("error create file:%s\n", namebuf);
        return 1;
    }

    int bufferLength = 255;
    char buffer[bufferLength];

    parse_result_t ppr;
    ParseInit(&ppr);
    ppr.put_sym = put_sym;

    parse_error_t pe = pe_no_error;
    int line = 1;
    while (fgets(buffer, bufferLength, filePointer)) {
        int len = strlen(buffer);
        printf("\nLINE: %d  len:%d   %s\n", line, len, buffer);
        pe = ParseLine(&ppr, line, buffer, len);
        if (pe) {
            ret = pe;
            break;
        }
        line++;
    }

    VmOp_End(&ppr);
    fclose(filePointer);
    fclose(outLstP);

    if (pe == pe_no_error) {
        printf("\n");
        for (int i = 0; i < VECTOR_SIZE(ppr.labels_vect); i++) {
            if (VECTOR_FROM_FIRST(ppr.labels_vect, i).jmp_code_offs_cnt == 0) {
                printf("Warning. Label '%s' is not used.\n", VECTOR_FROM_FIRST(ppr.labels_vect, i).name);
            } else {
                for (int n = 0; n < VECTOR_FROM_FIRST(ppr.labels_vect, i).jmp_code_offs_cnt; n++) {
                    // printf("id:%d(%s)  jmp_code_offs:%d  code_offs:%d\n", i, VECTOR_FROM_FIRST(ppr.labels_vect, i).name, VECTOR_FROM_FIRST(ppr.labels_vect, i).jmp_code_offs_arr[n], VECTOR_FROM_FIRST(ppr.labels_vect, i).code_offs);
                    fix_code_num(&ppr, VECTOR_FROM_FIRST(ppr.labels_vect, i).subs_body, VECTOR_FROM_FIRST(ppr.labels_vect, i).jmp_code_offs_arr[n], VECTOR_FROM_FIRST(ppr.labels_vect, i).code_offs, BITS_TO_BYTES(VARS_ADDR_BITS));
                }
            }
        }

        vm_header_t vm_header;
        vm_header.heap_min_size = ppr.heap_memory_max + ppr.heap_memory_max_funcs;
        vm_header.const_block_size = ppr.const_memory;
        vm_header.entry_point = VECTOR_SIZE(ppr.obj_subs_vect);
        vm_header.prg_size = VECTOR_SIZE(ppr.obj_subs_vect) + VECTOR_SIZE(ppr.obj_main_vect);
        //

        printf("\nHeap(vars) memory min size: %d bytes\n", vm_header.heap_min_size);
        printf("\tMain programm size: %d bytes\n", VECTOR_SIZE(ppr.obj_main_vect));
        printf("\tSubroutines size: %d bytes\n", vm_header.entry_point);
        printf("\tConst block size: %d bytes\n", vm_header.const_block_size);
        printf("\t___________\n\t%d bytes\n", vm_header.prg_size + vm_header.const_block_size);
        //  fclose(outLstP);

        snprintf(namebuf, sizeof(namebuf), "%s.sb", file_name);
        FILE *outExeP = fopen(namebuf, "w");
        if (outExeP == NULL) {
            printf("error create file:%s\n", namebuf);
        } else {
            fwrite(&vm_header, 1, sizeof(vm_header), outExeP);
            for (int i = 0; i < VECTOR_SIZE(ppr.const_arrays_vect); i++) {
                // #if ARRAY_INDEX_BITS != 8
                fwrite(&VECTOR_FROM_FIRST(ppr.const_arrays_vect, i).size, 1, 1, outExeP);
                fwrite(VECTOR_FROM_FIRST(ppr.const_arrays_vect, i).data, 1, VECTOR_FROM_FIRST(ppr.const_arrays_vect, i).size, outExeP);
            }

            if (VECTOR_SIZE(ppr.obj_subs_vect) > 0) {
                fwrite(ppr.obj_subs_vect.data, 1, VECTOR_SIZE(ppr.obj_subs_vect), outExeP);
            }

            if (VECTOR_SIZE(ppr.obj_main_vect) > 0) {
                fwrite(ppr.obj_main_vect.data, 1, VECTOR_SIZE(ppr.obj_main_vect), outExeP);
            }

            fclose(outExeP);
        }
    }

    ParseFree(&ppr);
    return ret;
}