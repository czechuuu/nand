// TODO variable naming convention
#include "llist.h"
#include "nand.h"
#include <stdlib.h>

struct llist_element {
    nand_t *val;
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
TODO think about memerror handling
?? repetitive code
 * adds a given nand gate to a given linked list
 * @param list list to which the gate will be added
 * @param gate the gate which will be added
 * @returns 0 on success error code otherwise
 */
int ll_add_element(llist_t *list, nand_t *gate) {
    if (!list->length) {
        return ll_add_element_to_empty_list(list, gate);
    } else {
        return ll_add_element_to_nonempty_list(list, gate);
    }
}

/*
 TODO does static have to be repeated
 * internal function that adds to a list assuming it already has no elements
*/
static int ll_add_element_to_empty_list(llist_t *list, nand_t *gate) {
    llist_element_t *newListElement = malloc(sizeof(llist_element_t));
    if (newListElement) {
        newListElement->val = gate;
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
static int ll_add_element_to_nonempty_list(llist_t *list, nand_t *gate) {
    llist_element_t *newListElement = malloc(sizeof(llist_element_t));
    if (newListElement) {
        newListElement->val = gate;
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
 * if no such element is present does nothing
 * @param list from which an element should be deleted
 * @param gate value of the element that should be deleted
 */
void ll_delete_element_with_given_value(llist_t *list, nand_t *gate) {
    llist_element_t *currentElement = list->head;
    while (currentElement) {
        if (currentElement->val == gate) {
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
void ll_delete_given_node_from_list(llist_t *list, llist_element_t *node) {
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