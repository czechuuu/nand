#include "nand.h"
#include "llist.h"
#include <stdlib.h>

struct nand {
    unsigned number_of_inputs;
    bool **input_array;
    bool output;
    llist_t *connected_to_output_list;
};

nand_t *nand_new(unsigned n) {
    nand_t *new_gate = malloc(sizeof(nand_t));
    if (new_gate) {
        new_gate->input_array = calloc(n, sizeof(bool *)); // all NULLs
        if (new_gate->input_array) {
            new_gate->number_of_inputs = n;
            new_gate->connected_to_output_list = ll_new();

            return new_gate;
        }
    }
    // memory error occurred
    // if gate allocation succeeded and arrays hasnt it needs to be freed
    // in any other case nothing needs freeing
    free(new_gate);
}
