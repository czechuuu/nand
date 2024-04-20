#include "llist.h"
#include "nand.h"
#include <assert.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4

typedef struct llist {
    size_t capacity;   // Current capacity of the array
    size_t size;       // Number of elements in the array
    nand_t **elements; // Array to store gate pointers
    unsigned *indices; // Array to store gate indices
} llist_t;

/*
 * Creates a new empty list.
 * Can fail and return null.
 * @returns a new empty list or null.
 */
llist_t *ll_new(void) {
    llist_t *const newEmptyList = malloc(sizeof(llist_t));
    if (newEmptyList) {
        newEmptyList->capacity = INITIAL_CAPACITY;
        newEmptyList->size = 0;
        newEmptyList->elements =
            malloc(sizeof(nand_t *) * newEmptyList->capacity);
        newEmptyList->indices =
            malloc(sizeof(unsigned) * newEmptyList->capacity);
        if (!newEmptyList->elements || !newEmptyList->indices) {
            free(newEmptyList->elements);
            free(newEmptyList->indices);
            free(newEmptyList);
            return NULL;
        }
    }
    return newEmptyList;
}

/*
 * Deletes the given linked list along with all its elements freeing memory.
 * @param list list to be deleted
 */
void ll_delete(llist_t *list) {
    free(list->elements);
    free(list->indices);
    free(list);
}

/*
 * @returns length of given linked list
 */
ssize_t ll_length(const llist_t *list) { return list->size; }

static int resize_if_needed(llist_t *list) {
    if (list->size == list->capacity) {
        // Double the capacity
        size_t new_capacity = list->capacity * 2;

        // Allocate new memory for elements and indices
        nand_t **new_elements =
            realloc(list->elements, sizeof(nand_t *) * new_capacity);
        unsigned *new_indices =
            realloc(list->indices, sizeof(unsigned) * new_capacity);

        // Check if both allocations succeeded
        if (new_elements && new_indices) {
            // Update list attributes with new allocations
            list->elements = new_elements;
            list->indices = new_indices;
            list->capacity = new_capacity;
            return 0; // Resize successful
        } else {
            // Free any partially allocated memory (only if realloc didn't
            // return the original pointers)
            if (new_elements != list->elements) {
                free(new_elements);
            }
            if (new_indices != list->indices) {
                free(new_indices);
            }
            return -1; // Resize failed due to allocation failure
        }
    }
    return 0; // No resize needed, return success
}

/*
 * Adds a given nand gate and index to the list.
 * Can fail due to allocation failures.
 * @param list list to which the elements will be added
 * @param index index which will be added
 * @param gate the gate which will be added
 * @returns 0 on success, -1 on allocation failure
 */
int ll_add_element(llist_t *list, nand_t const *gate, unsigned index) {
    int resize_result = resize_if_needed(list);
    if (resize_result < 0) {
        return -1; // Resize failed, indicate allocation failure
    }

    list->elements[list->size] = gate;
    list->indices[list->size] = index;
    list->size++;
    return 0;
}

/*
 * Deletes the first element with a given gate-index pair.
 * @param list from which an element should be deleted
 * @param gate gate value of the element that should be deleted
 * @param index index value of the element that should be deleted
 */
void ll_delete_element_with_given_value(llist_t *list, nand_t const *gate,
                                        unsigned index) {
    for (size_t i = 0; i < list->size; i++) {
        if (list->elements[i] == gate && list->indices[i] == index) {
            // Swap the element to be deleted with the last element
            nand_t *temp_gate = list->elements[i];
            unsigned temp_index = list->indices[i];
            list->elements[i] = list->elements[list->size - 1];
            list->indices[i] = list->indices[list->size - 1];
            list->elements[list->size - 1] = temp_gate;
            list->indices[list->size - 1] = temp_index;
            list->size--;
            return;
        }
    }
}

/*
 * Returns the gate-index pair of the head of the list.
 * The list should be non-empty.
 * @param list
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_get_head(const llist_t *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    assert(list->size > 0); // Assert that the list is not empty
    *return_gate_ptr = list->elements[0];
    *return_index_ptr = list->indices[0];
}

/*
 * Deletes the current list head element.
 * If non-null pointers are given, also returns its value via pointers.
 * @param list
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_pop_head(llist_t *list, nand_t **return_gate_ptr,
                 unsigned *return_index_ptr) {
    assert(list->size > 0); // Assert that the list is not empty
    if (return_gate_ptr) {
        *return_gate_ptr = list->elements[0];
    }
    if (return_index_ptr) {
        *return_index_ptr = list->indices[0];
    }
    // Swap the element to be deleted with the last element
    nand_t *temp_gate = list->elements[0];
    unsigned temp_index = list->indices[0];
    list->elements[0] = list->elements[list->size - 1];
    list->indices[0] = list->indices[list->size - 1];
    list->elements[list->size - 1] = temp_gate;
    list->indices[list->size - 1] = temp_index;
    list->size--;
}

/*
 * Gets the value of the kth element of the list.
 * @param list
 * @param k which element (0-based indexing)
 * @param return_gate_ptr pointer to where the gate should be stored
 * @param return_index_ptr pointer to where the index should be stored
 */
void ll_get_kth_element(const llist_t *list, ssize_t k,
                        nand_t **return_gate_ptr, unsigned *return_index_ptr) {
    assert(k >= 0 && k < list->size); // Assert that k is within the list bounds
    *return_gate_ptr = list->elements[k];
    *return_index_ptr = list->indices[k];
}
