#ifndef DEBUG_H_
#define DEBUG_H_

#define STR(s) #s

enum {
    DL_WRN,
    DL_DBG,
    DL_DBG1,
    DL_TRC,
};

#ifdef DEBUG
#define MSG_SUF() fprintf(stderr, " - [%s:%d(%s)]\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define MSG_DBG(lvl, format, ...)                                                                                       \
    if (lvl <= DEBUG_LVL) {                                                                                             \
        fprintf(stderr, "%d: " format " - [%s:%d(%s)]\n", lvl, ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    }

#define MSG_ERR(format, ...) fprintf(stderr, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define MSG_INF(format, ...) fprintf(stderr, format " - [%s:%d(%s)]\n", ##__VA_ARGS__, __FILE__, __LINE__, __PRETTY_FUNCTION__)

#else

#define MSG_DBG(lvl, format, ...)
#define MSG_ERR(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)
#define MSG_INF(format, ...) fprintf(stderr, format "\n", ##__VA_ARGS__)
#endif

#define PRINT_ERROR(err)         \
    {                            \
        PrintError(result, err); \
        MSG_SUF();               \
    }

#endif

#define RETURN_ON_ERROR(err)                          \
    if (err) {                                        \
        parse_error_t __pe = PrintError(result, err); \
        MSG_SUF();                                    \
        return __pe;                                  \
    }
