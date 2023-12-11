#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

void *VectorNewElement(void **vect_pnt, int *vect_size, int *_elems_alloc, int elem_size) {
    void *pnt;
    if (*vect_pnt == NULL) {
        pnt = malloc(elem_size * VECTOR_STEP_SIZE);
        if (pnt) {
            *vect_pnt = pnt;
            *_elems_alloc = VECTOR_STEP_SIZE;
            *vect_size = 1;
        }
    } else {
        if (*vect_size + 1 > *_elems_alloc) {
            int new_alloc_size = *_elems_alloc + VECTOR_STEP_SIZE;
            pnt = realloc(*vect_pnt, elem_size * new_alloc_size);
            if (!pnt) {
                return pnt;
            }
            *vect_pnt = pnt;
            *_elems_alloc = new_alloc_size;
        }
        pnt = *vect_pnt + elem_size * (*vect_size);
        (*vect_size)++;
    }
    memset(pnt, 0, elem_size);
    return pnt;
}