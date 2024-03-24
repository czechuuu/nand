#include "llist.h"
#include "nand.h"
#include <assert.h>
#include <stdio.h>

#define NEW nand_new(0)

// Function to test ll_new() function
void test_ll_new() {
    llist_t *list = ll_new();
    assert(list != NULL);
    assert(ll_length(list) == 0);
    ll_delete(list);
}

// Function to test ll_add_element() function
void test_ll_add_element() {
    llist_t *list = ll_new();
    nand_t *gate1 = NEW; // Initialize your nand_t objects accordingly
    nand_t *gate2 = NEW;
    int result = ll_add_element(list, gate1, 1);
    assert(result == 0);
    assert(ll_length(list) == 1);
    result = ll_add_element(list, gate2, 2);
    assert(result == 0);
    assert(ll_length(list) == 2);

    nand_delete(gate1);
    nand_delete(gate2);
    ll_delete(list);
}

// Function to test ll_get_kth_element() function
void test_ll_get_kth_element() {
    llist_t *list = ll_new();
    // Add elements to the list
    // Assuming you have gates available
    nand_t *gate1 = NEW; // Initialize your nand_t objects accordingly
    nand_t *gate2 = NEW;
    ll_add_element(list, gate1, 1);
    ll_add_element(list, gate2, 2);
    // Test retrieving kth element
    nand_t *gate_ptr = NULL;
    unsigned index = 0;
    ll_get_kth_element(list, 0, &gate_ptr, &index);
    assert(gate_ptr == gate1);
    assert(index == 1);
    ll_get_kth_element(list, 1, &gate_ptr, &index);
    assert(gate_ptr == gate2);
    assert(index == 2);

    nand_delete(gate1);
    nand_delete(gate2);
    ll_delete(list);
}

void test_ll_pop_head() {
    nand_t *g0 = NEW;
    nand_t *g1 = NEW;
    nand_t *g2 = NEW;
    nand_t *g3 = NEW;
    nand_t *g4 = NEW;
    nand_t *g5 = NEW;

    llist_t *l = ll_new();
    ll_add_element(l, g0, 0);
    ll_add_element(l, g1, 1);
    ll_add_element(l, g2, 2);
    ll_add_element(l, g3, 3);
    ll_add_element(l, g4, 4);
    ll_add_element(l, g5, 5);
    assert(ll_length(l) == 6);

    nand_t *curr_g;
    unsigned curr_index;
    ll_pop_head(l, &curr_g, &curr_index);
    assert(curr_g == g0);
    assert(curr_index == 0);
    assert(ll_length(l) == 5);
    ll_pop_head(l, &curr_g, &curr_index);
    assert(curr_g == g1);
    assert(curr_index == 1);
    assert(ll_length(l) == 4);
    ll_pop_head(l, &curr_g, &curr_index);
    assert(curr_g == g2);
    assert(curr_index == 2);
    assert(ll_length(l) == 3);

    nand_delete(g0);
    nand_delete(g1);
    nand_delete(g2);
    nand_delete(g3);
    nand_delete(g4);
    nand_delete(g5);
    ll_delete(l);
}

int main() {
    test_ll_new();
    test_ll_add_element();
    test_ll_get_kth_element();
    test_ll_pop_head();
    printf("All tests passed!\n");
    return 0;
}
