#include "llist.h"
#include "nand.h"
#include <assert.h>
#include <stdlib.h>

// no point in doing it in the header since its only used internally
typedef struct llist_element llist_element_t;

// some of then are used backwards so declarations are needed
static int ll_add_element_to_empty_list(llist_t *list, nand_t const *gate,
                                        unsigned index);
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t const *gate,
                                           unsigned index);
static void ll_get_element(llist_element_t const *elem,
                           nand_t **return_gate_ptr,
                           unsigned *return_index_ptr);
static void ll_delete_given_node_from_list(llist_t *list,
                                           llist_element_t *node);

struct llist_element {
    nand_t const *val_gate;
    unsigned val_index;
    struct llist_element *next, *prev;
};

struct llist {
    ssize_t length;
    struct llist_element *head, *tail;
};

/*
 * creates new empty list
 * can fail and return null
 * @returns a new empty list or null
 */
llist_t *ll_new(void) {
    llist_t *const newEmptyList = malloc(sizeof(llist_t));
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
        llist_element_t *const nextElement = currentElement->next;
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
 * adds a given nand gate and index to a given linked list
 * can fail
 * @param list list to which the elements will be added
 * @param index index which will be added
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
 * internal function that adds to a list assuming it already has no elements
 * can fail
 * @param list list to which the elements will be added
 * @param index index which will be added
 * @param gate the gate which will be added
 * @returns 0 on success -1 otherwise
 */
static int ll_add_element_to_empty_list(llist_t *list, nand_t const *gate,
                                        unsigned index) {
    llist_element_t *const newListElement = malloc(sizeof(llist_element_t));
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
        return -1;
    }
}

/*
 * internal function that adds to a list assuming it already has an element
 * can fail
 * @param list list to which the elements will be added
 * @param index index which will be added
 * @param gate the gate which will be added
 * @returns 0 on success -1 otherwise
 */
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t const *gate,
                                           unsigned index) {
    llist_element_t *const newListElement = malloc(sizeof(llist_element_t));
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
        return -1;
    }
}

/*
 * deletes first element with given gate index value pair
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
 * internal function that deletes a node from a list
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

/*
 * internal function that returns the gate index value pair
 * of a given list element via pointers
 * @param elem list element whose value should be unpacked
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
static void ll_get_element(llist_element_t const *elem,
                           nand_t **return_gate_ptr,
                           unsigned *return_index_ptr) {
    *return_gate_ptr = (nand_t *)elem->val_gate;
    *return_index_ptr = elem->val_index;
}

/*
 * returns the gate index value pair of the head of given list
 * the list should be nonempty
 * @param list
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_get_head(llist_t const *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    ll_get_element(list->head, return_gate_ptr, return_index_ptr);
}

/*
 * deletes current list head
 * if nonnull ptrs given also returns its value via ptrs
 * @param list
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_pop_head(llist_t *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    if (return_gate_ptr && return_index_ptr) {
        ll_get_head(list, return_gate_ptr, return_index_ptr);
    }
    ll_delete_given_node_from_list(list, list->head);
}

/*
 * gets the value of the kth element of the list
 * @param list
 * @param k which element
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_get_kth_element(llist_t const *list, ssize_t k,
                        nand_t **return_gate_ptr, unsigned *return_index_ptr) {
    assert(k < list->length);
    llist_element_t const *current_element = list->head;
    for (ssize_t i = 0; i < k; i++) {
        current_element = current_element->next;
    }
    ll_get_element(current_element, return_gate_ptr, return_index_ptr);
}