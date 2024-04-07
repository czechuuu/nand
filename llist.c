// TODO function declaration parameter names
#include "llist.h"
#include "nand.h"
#include <assert.h> //?do i really need this?
#include <stdlib.h>

static int ll_add_element_to_empty_list(llist_t *list, nand_t const *gate,
                                        unsigned index);
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t const *gate,
                                           unsigned index);
static void ll_get_element(llist_element_t const *elem,
                           nand_t **return_gate_ptr, unsigned *return_index);
static void ll_delete_given_node_from_list(llist_t *list,
                                           llist_element_t *node);

// TODO maybe wrap gate and index in a struct?
struct llist_element {
    nand_t *val_gate;
    unsigned val_index;
    struct llist_element *next, *prev;
};

struct llist {
    ssize_t length;
    struct llist_element *head, *tail;
};

/*
 * creates new empty list
 * @returns a new empty list or null
 */
llist_t *ll_new(void) {
    llist_t *newEmptyList = malloc(sizeof(llist_t));
    if (newEmptyList) {
        newEmptyList->length = 0;
        newEmptyList->head = NULL;
        newEmptyList->tail = NULL;
    }

    return newEmptyList;
}

/*
 * deletes given linked list along with all its elements freeing memory
 * @param list list to be deleted
 */
void ll_delete(llist_t *list) {
    llist_element_t *currentElement = list->head;
    while (currentElement) {
        llist_element_t *nextElement = currentElement->next;
        free(currentElement);
        currentElement = nextElement;
    }

    free(list);
}

/*
 * @returns length of given linked list
 */
ssize_t ll_length(llist_t const *list) { return list->length; }

/*
! check return value for failure
?? repetitive code
 * adds a given nand gate to a given linked list
 * @param list list to which the gate will be added
 * @param gate the gate which will be added
 * @returns 0 on success -1 otherwise
 */
int ll_add_element(llist_t *list, nand_t const *gate, unsigned index) {
    if (!list->length) {
        return ll_add_element_to_empty_list(list, gate, index);
    } else {
        return ll_add_element_to_nonempty_list(list, gate, index);
    }
}

/*
 TODO does static have to be repeated
 * internal function that adds to a list assuming it already has no elements
*/
static int ll_add_element_to_empty_list(llist_t *list, nand_t const *gate,
                                        unsigned index) {
    llist_element_t *newListElement = malloc(sizeof(llist_element_t));
    if (newListElement) {
        newListElement->val_gate = gate;
        newListElement->val_index = index;
        newListElement->prev = NULL;
        newListElement->next = NULL;

        list->head = newListElement;
        list->tail = newListElement;
        list->length = 1;

        return 0;
    } else {
        // TODO handle memerror
        return -1;
    }
}

/*
 * internal function that adds to a list assuming it already has an element
 */
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t const *gate,
                                           unsigned index) {
    llist_element_t *newListElement = malloc(sizeof(llist_element_t));
    if (newListElement) {
        newListElement->val_gate = gate;
        newListElement->val_index = index;
        newListElement->prev = list->tail;
        newListElement->next = NULL;

        list->tail->next = newListElement;
        list->tail = newListElement;
        list->length++;

        return 0;
    } else {
        // TODO handle memerror
        return -1;
    }
}

/*
 * deletes first element with given nothing
 * @param list from which an element should be deleted
 * @param gate gate value of the element that should be deleted
 * @param index index value of the element that should be deleted
 */
void ll_delete_element_with_given_value(llist_t *list, nand_t const *gate,
                                        unsigned index) {
    llist_element_t *currentElement = list->head;
    while (currentElement) {
        if (currentElement->val_gate == gate &&
            currentElement->val_index == index) {
            ll_delete_given_node_from_list(list, currentElement);
            break;
        }
        currentElement = currentElement->next;
    }
}

/*
 * deletes a node from a list
 * the node has to be in fact a part of the list
 */
static void ll_delete_given_node_from_list(llist_t *list,
                                           llist_element_t *node) {
    if (node == list->head && node == list->tail) {
        list->head = NULL;
        list->tail = NULL;
    } else if (node == list->head) {
        list->head = node->next;
        list->head->prev = NULL;
    } else if (node == list->tail) {
        list->tail = node->prev;
        list->tail->next = NULL;
    } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    list->length--;
    free(node);
}

static void ll_get_element(llist_element_t const *elem,
                           nand_t **return_gate_ptr, unsigned *return_index) {
    *return_gate_ptr = elem->val_gate;
    *return_index = elem->val_index;
}

void ll_get_head(llist_t const *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    ll_get_element(list->head, return_gate_ptr, return_index_ptr);
}

/*
 * deletes current list head if nonnull ptrs given also returns value
 * @returns via ptrs if nonnull
 */
void ll_pop_head(llist_t *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    if (return_gate_ptr && return_index_ptr) {
        ll_get_head(list, return_gate_ptr, return_index_ptr);
    }
    ll_delete_given_node_from_list(list, list->head);
}

void ll_get_kth_element(llist_t const *list, ssize_t k,
                        nand_t **return_gate_ptr, unsigned *return_index) {
    assert(k < list->length);
    llist_element_t *current_element = list->head;
    for (ssize_t i = 0; i < k; i++) {
        current_element = current_element->next;
    }
    ll_get_element(current_element, return_gate_ptr, return_index);
}