#ifndef LLIST_H
#define LLIST_H

#include "nand.h"

typedef struct llist_element llist_element_t;
typedef struct llist llist_t;

extern llist_t *ll_new(void);
extern llist_t *delete(llist_t *);
extern int ll_add_element(llist_t *, nand_t *);
extern void ll_delete_element_with_given_value(llist_t *list, nand_t *gate);
extern void ll_delete_given_node_from_list(llist_t *, llist_element_t *);

static int ll_add_element_to_empty_list(llist_t *, nand_t *);
static int ll_add_element_to_nonempty_list(llist_t *, nand_t *);

#endif // LLIST_H