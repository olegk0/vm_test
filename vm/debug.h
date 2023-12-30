#ifndef DEBUG_H_
#define DEBUG_H_

#include <curses.h>

extern WINDOW *main_window;
extern WINDOW *stack_window;
extern WINDOW *vars_window;
extern WINDOW *code_window;
extern WINDOW *log_window;
extern WINDOW *pnts_window;

#ifdef WRITE_LOG
extern FILE *outLogP;
#endif

#define FULL_WIDTH 150
#define FULL_HEIGHT 35

#define STACK_WIN_WIDTH 60
#define PNT_WIN_WIDTH 22
#define MAIN_WIN_WIDTH (FULL_WIDTH - STACK_WIN_WIDTH - PNT_WIN_WIDTH)
#define MAIN_WIN_HEIGHT 20
#if 1
#define STACK_WIN_HEIGHT (FULL_HEIGHT / 2)
#define VARS_WIN_HEIGHT (FULL_HEIGHT - STACK_WIN_HEIGHT)
#define LOG_WIN_HEIGHT (FULL_HEIGHT - MAIN_WIN_HEIGHT)
#define LOG_WIN_WIDTH (FULL_WIDTH - STACK_WIN_WIDTH)
#define LOG_CODE_WIN_WIDTH 36
#else
#define STACK_WIN_HEIGHT (MAIN_WIN_HEIGHT / 2)
#define VARS_WIN_HEIGHT (MAIN_WIN_HEIGHT - STACK_WIN_HEIGHT)
#define LOG_WIN_HEIGHT (FULL_HEIGHT - MAIN_WIN_HEIGHT)
#define LOG_WIN_WIDTH FULL_WIDTH
#define LOG_CODE_WIN_WIDTH 20
#endif
#define STR(s) #s

enum {
    DL_WRN,
    DL_DBG,
    DL_DBG1,
    DL_TRC,
};

#ifdef DEBUG
#ifdef WRITE_LOG

#define MSG_DBG(lvl, format, ...)                                                                                           \
    if (lvl <= DEBUG_LVL) {                                                                                                 \
        wprintw(log_window, "%d: " format " - [%s:%d(%s)]\n", lvl, ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
        fprintf(outLogP, "%d: " format " - [%s:%d(%s)]\n", lvl, ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__);    \
    }

#define MSG_ERR(format, ...)                                                                                \
    wprintw(log_window, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    fprintf(outLogP, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define MSG_INF(format, ...)                                                                                \
    wprintw(log_window, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    fprintf(outLogP, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)

#else

#define MSG_DBG(lvl, format, ...)                                                                                           \
    if (lvl <= DEBUG_LVL) {                                                                                                 \
        wprintw(log_window, "%d: " format " - [%s:%d(%s)]\n", lvl, ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    }

#define MSG_ERR(format, ...) \
    wprintw(log_window, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__);

#define MSG_INF(format, ...) \
    wprintw(log_window, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__);

#endif

#else

#ifdef WRITE_LOG

#define MSG_DBG(lvl, format, ...)
#define MSG_ERR(format, ...)                                   \
    wprintw(log_window, "Error: " format "\n", ##__VA_ARGS__); \
    fprintf(outLogP, "Error: " format "\n", ##__VA_ARGS__)
#define MSG_INF(format, ...)                                  \
    wprintw(log_window, "Info: " format "\n", ##__VA_ARGS__); \
    fprintf(outLogP, "Info: " format "\n", ##__VA_ARGS__)

#else
#define MSG_DBG(lvl, format, ...)
#define MSG_ERR(format, ...) \
    wprintw(log_window, "Error: " format "\n", ##__VA_ARGS__);

#define MSG_INF(format, ...) \
    wprintw(log_window, "Info: " format "\n", ##__VA_ARGS__);

#endif
#endif

#endif
