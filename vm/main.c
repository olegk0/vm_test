#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "vm_exe.h"

WINDOW *main_window;
WINDOW *pnts_window;
WINDOW *stack_window;
WINDOW *vars_window;
WINDOW *log_window;
WINDOW *code_window;
#ifdef WRITE_LOG
FILE *outLogP = NULL;
#endif

void init_screen() {
    initscr();
    // with height = 15 and width = 10
    // also with start x axis 10 and start y axis = 20
    WINDOW *main_window_out = newwin(MAIN_WIN_HEIGHT, MAIN_WIN_WIDTH, 0, 0);
    box(main_window_out, 0, 0);
    mvwprintw(main_window_out, 0, 1, "Main output");
    main_window = newwin(MAIN_WIN_HEIGHT - 2, MAIN_WIN_WIDTH - 2, 0 + 1, 0 + 1);
    scrollok(main_window, TRUE);
    //
    WINDOW *pnts_window_out = newwin(MAIN_WIN_HEIGHT, PNT_WIN_WIDTH, 0, MAIN_WIN_WIDTH);
    box(pnts_window_out, 0, 0);
    mvwprintw(pnts_window_out, 0, 1, " IP   SP   HP   VB ");
    pnts_window = newwin(MAIN_WIN_HEIGHT - 2, PNT_WIN_WIDTH - 2, 0 + 1, MAIN_WIN_WIDTH + 1);
    scrollok(pnts_window, TRUE);
    //
    WINDOW *stack_window_out = newwin(STACK_WIN_HEIGHT, STACK_WIN_WIDTH, 0, MAIN_WIN_WIDTH + PNT_WIN_WIDTH);
    box(stack_window_out, 0, 0);
    mvwprintw(stack_window_out, 0, 1, "Stack");
    stack_window = newwin(STACK_WIN_HEIGHT - 2, STACK_WIN_WIDTH - 2, 0 + 1, MAIN_WIN_WIDTH + PNT_WIN_WIDTH + 1);
    scrollok(stack_window, TRUE);
    //
    WINDOW *vars_window_out = newwin(VARS_WIN_HEIGHT, STACK_WIN_WIDTH, STACK_WIN_HEIGHT, MAIN_WIN_WIDTH + PNT_WIN_WIDTH);
    box(vars_window_out, 0, 0);
    mvwprintw(vars_window_out, 0, 1, "Heap");
    vars_window = newwin(VARS_WIN_HEIGHT - 2, STACK_WIN_WIDTH - 2, STACK_WIN_HEIGHT + 1, MAIN_WIN_WIDTH + PNT_WIN_WIDTH + 1);
    scrollok(vars_window, TRUE);
    //
    WINDOW *log_window_out = newwin(LOG_WIN_HEIGHT, LOG_WIN_WIDTH, MAIN_WIN_HEIGHT, 0);
    box(log_window_out, 0, 0);
    mvwprintw(log_window_out, 0, 1, "Logs");
    log_window = newwin(LOG_WIN_HEIGHT - 2, (LOG_WIN_WIDTH - 2) - LOG_CODE_WIN_WIDTH, MAIN_WIN_HEIGHT + 1, 0 + 1);
    scrollok(log_window, TRUE);
    code_window = newwin(LOG_WIN_HEIGHT - 2, LOG_CODE_WIN_WIDTH, MAIN_WIN_HEIGHT + 1, (LOG_WIN_WIDTH - 2) - LOG_CODE_WIN_WIDTH + 1);
    scrollok(code_window, TRUE);
    //
    refresh();

    // refreshing the window
    wrefresh(main_window_out);
    wrefresh(pnts_window_out);
    wrefresh(stack_window_out);
    wrefresh(vars_window_out);
    wrefresh(log_window_out);
    // move(5, 5);
    // addstr("Hello");

    // timeout(5000);
    //  int c = getch();
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("no file\n");
        return 1;
    }

    FILE *filePointer = fopen(argv[1], "r");
    if (filePointer == NULL) {
        printf("wrong file:%s\n", argv[1]);
        return 1;
    }

#ifdef WRITE_LOG
    char *file_name;
    (file_name = strrchr(argv[1], '/')) ? file_name++ : (file_name = argv[1]);
    char *file_ext = strrchr(file_name, '.');
    if (file_ext) {
        *file_ext = 0;
    }

    char namebuf[strlen(file_name) + 10];
    snprintf(namebuf, sizeof(namebuf), "%s.log", file_name);
    outLogP = fopen(namebuf, "w");
    if (outLogP == NULL) {
        printf("error create file:%s\n", namebuf);
        return 1;
    }
#endif

    // fseek(filePointer, 0L, SEEK_END);
    // int file_size = ftell(filePointer);

    // rewind(filePointer);
    vm_header_t vm_header;

    if (fread(&vm_header, 1, sizeof(vm_header_t), filePointer) == sizeof(vm_header_t)) {
        // printf("\nHeap(vars) memory min size: %d bytes\n", vm_header.heap_min_size);
        // printf("\tMain programm size: %d bytes\n", vm_header.prg_size - vm_header.entry_point);
        // printf("\tSubroutines size: %d bytes\n", vm_header.entry_point);
        // printf("\tConst block size: %d bytes\n", vm_header.const_block_size);
        // printf("\t___________\n\t%d bytes\n", vm_header.prg_size + vm_header.const_block_size);

        uint8_t const_block[vm_header.const_block_size];
        if (fread(const_block, 1, vm_header.const_block_size, filePointer) == vm_header.const_block_size) {
            uint8_t code_block[vm_header.prg_size];
            if (fread(code_block, 1, vm_header.prg_size, filePointer) == vm_header.prg_size) {
                init_screen();
                wprintw(log_window, "\nHeap(vars) memory min size: %d bytes\n", vm_header.heap_min_size);
                wprintw(log_window, "\tMain programm size: %d bytes\n", vm_header.prg_size - vm_header.entry_point);
                wprintw(log_window, "\tSubroutines size: %d bytes\n", vm_header.entry_point);
                wprintw(log_window, "\tConst block size: %d bytes\n", vm_header.const_block_size);
                wprintw(log_window, "\t___________\n\t%d bytes\n", vm_header.prg_size + vm_header.const_block_size);
                err_code_t ret = Run(code_block, const_block, &vm_header, 1);
                wprintw(log_window, "\nret: %d\n", ret);
                wrefresh(main_window);
                wrefresh(stack_window);
                wrefresh(vars_window);
                wrefresh(log_window);
                fclose(filePointer);
#ifdef WRITE_LOG
                fclose(outLogP);
#endif
                getch();
                endwin();
                return ret;
            }
        }
    }
    fprintf(stderr, "Error read file %s\n", argv[1]);
    fclose(filePointer);
#ifdef WRITE_LOG
    fclose(outLogP);
#endif
    return -1;
}