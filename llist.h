#ifndef LLIST_H
#define LLIST_H

#include "nand.h"

typedef struct llist_element llist_element_t;
typedef struct llist llist_t;
// TODO maybe struct for return of list getters

extern llist_t *ll_new(void);
extern void ll_delete(llist_t *list);
extern ssize_t ll_length(llist_t *list);
extern int ll_add_element(llist_t *list, nand_t *gate, unsigned index);
extern void ll_delete_element_with_given_value(llist_t *list, nand_t *gate,
                                               unsigned index);
extern void ll_pop_head(llist_t *list, nand_t **return_gate_ptr,
                        unsigned *return_index_ptr);
extern void ll_get_head(llist_t *list, nand_t **return_gate_ptr,
                        unsigned *return_index_ptr);
extern void ll_get_kth_element(llist_t *list, ssize_t k,
                               nand_t **return_gate_ptr,
                               unsigned *return_index);

// TODO shouldnt the statics maybe be done in the c file itself
// does static have to be repeated in the declarations later
// i guess gcc gave me the answer
static int ll_add_element_to_empty_list(llist_t *list, nand_t *gate,
                                        unsigned index);
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t *gate,
                                           unsigned index);
static void ll_get_element(llist_element_t *elem, nand_t **return_gate_ptr,
                           unsigned *return_index);
static void ll_delete_given_node_from_list(llist_t *list,
                                           llist_element_t *node);

#endif // LLIST_H