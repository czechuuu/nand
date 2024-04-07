#ifndef LLIST_H
#define LLIST_H

#include "nand.h"

typedef struct llist_element llist_element_t;
typedef struct llist llist_t;
// TODO maybe struct for return of list getters

extern llist_t *ll_new(void);
extern void ll_delete(llist_t *list);
extern ssize_t ll_length(llist_t const *list);
extern int ll_add_element(llist_t *list, nand_t const *gate, unsigned index);
extern void ll_delete_element_with_given_value(llist_t *list,
                                               nand_t const *gate,
                                               unsigned index);
extern void ll_pop_head(llist_t *list, nand_t **return_gate_ptr,
                        unsigned *return_index_ptr);
extern void ll_get_head(llist_t const *list, nand_t **return_gate_ptr,
                        unsigned *return_index_ptr);
extern void ll_get_kth_element(llist_t const *list, ssize_t k,
                               nand_t **return_gate_ptr,
                               unsigned *return_index);

#endif // LLIST_H