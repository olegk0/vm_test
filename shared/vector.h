#ifndef VECTOR_H_
#define VECTOR_H_

#define VECTOR_STEP_SIZE 100

#define VECTOR(STRUCT, VECT) \
    typedef struct {         \
        int size_ro;         \
        STRUCT *data;        \
        int _elems_alloc;    \
    } VECT;
// int elem_size;

#define VECTOR_INIT(VECT)      \
    {                          \
        VECT.data = NULL;      \
        VECT.size_ro = 0;      \
        VECT._elems_alloc = 0; \
    }
//        VECT.elem_size = sizeof(VECT);

#define VECTOR_FREE(VECT)  \
    {                      \
        free(VECT.data);   \
        VECTOR_INIT(VECT); \
    }

void *VectorNewElement(void **vect_pnt, int *vect_size, int *_elems_alloc, int elem_size);

// #define VECTOR_NEW(STRUCT, VECT) (STRUCT *)VectorNewElement((void **)&VECT.data, &VECT.size_ro, &VECT._elems_alloc, sizeof(STRUCT))
#define VECTOR_NEW(VECT) \
    VectorNewElement((void **)&VECT.data, &VECT.size_ro, &VECT._elems_alloc, sizeof(*VECT.data))
#define VECTOR_NEW_RET(VECT, toPNT) \
    VECTOR_NEW(VECT);               \
    (toPNT) = &VECT.data[VECT.size_ro - 1]

#define VECTOR_SIZE(VECT) VECT.size_ro
#define VECTOR_LAST_PNT(VECT) (VECT.size_ro > 0) ? &VECT.data[VECT.size_ro - 1] : NULL
#define VECTOR_LAST(VECT) VECT.data[VECT.size_ro - 1]
#define VECTOR_FROM_LAST(VECT, offs) VECT.data[VECT.size_ro - (1 + offs)]
#define VECTOR_FROM_FIRST(VECT, offs) VECT.data[offs]

#define VECTOR_POP(VECT)    \
    if (VECT.size_ro > 0) { \
        VECT.size_ro--;     \
    }

#define VECTOR_PACK(VECT) TODO

#define VECTOR_REMOVE(VECT, IDX) TODO

#endif
