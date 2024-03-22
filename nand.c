// todo prettier comments
#include "nand.h"
#include "llist.h"
#include <errno.h>
#include <stdlib.h>

// TODO can (should) i do this??
static void unplug_all_io(nand_t *);
static void unplug_given_input(nand_t *receiver, unsigned k);

/*
 * under a given index i,  0 or 1 of the folowing should be nonnull:
 * - gate_input_array[i]
 * - signal_input_array[i]
 * i.e. cant have two inputs under the same index
 */
struct nand {
    unsigned number_of_inputs;
    bool **signal_input_array;
    nand_t **gate_input_array;
    bool output;
    llist_t *connected_to_output_list;
};

/*
TODO ugly af
 * creates new nand gate with n inputs
 * calloc used to fill all arrays with nulls
 * NULLABLE
 * @param n number of inputs
 * @returns new gate or NULL
 */
nand_t *nand_new(unsigned n) {
    nand_t *new_gate = malloc(sizeof(nand_t));
    if (new_gate) {
        new_gate->signal_input_array = calloc(n, sizeof(bool *));
        if (new_gate->signal_input_array) {
            new_gate->gate_input_array = calloc(n, sizeof(nand_t *));
            if (new_gate->gate_input_array) {
                new_gate->connected_to_output_list = ll_new();
                if (new_gate->connected_to_output_list) {
                    // if we got here everything allocated succesfully
                    new_gate->number_of_inputs = n;
                    return new_gate;
                }
            }
            free(new_gate->gate_input_array); // in case list failed
        }
        free(new_gate->signal_input_array); // in case list or gate array failed
    }
    free(new_gate); // in case list or any array failed
    // if we havent left the function in the innermost if sth failed
    // have to set errno
    errno = ENOMEM;
    return NULL;
}

/*
 * unplugs inputs and outputs then frees all memory associated with the gate
 * renders given ptr unusable, if called on NULL does nothing
 * @param g gate to be deleted
 */
void nand_delete(nand_t *g) {
    if (g) {
        unplug_all_io(g);
        free(g->gate_input_array);
        free(g->signal_input_array);
        ll_delete(g->connected_to_output_list);

        free(g);
    }
}

/*
?? mayve extract second loop into a unplug_outpuit function
 * internal assumes non null
 * if there was an input bool signal its address is forgotten
 */
static void unplug_all_io(nand_t *gate) {
    for (unsigned i = 0; i < gate->number_of_inputs; i++) {
        unplug_given_input(gate, i);
    }
    while (ll_length(gate->connected_to_output_list)) {
        nand_t *receiving_gate;
        unsigned input_index;
        ll_get_head(gate->connected_to_output_list, &receiving_gate,
                    &input_index);
        unplug_given_input(receiving_gate, input_index);
    }
}

/*
 * internal
 * unplugs given input
 * if it was a gate then also unplugs from the output list
 */
static void unplug_given_input(nand_t *receiver, unsigned k) {
    nand_t *gate_connected_to_given_input = receiver->gate_input_array[k];
    if (gate_connected_to_given_input) {
        // if its a gate
        ll_delete_element_with_given_value(
            gate_connected_to_given_input->connected_to_output_list, receiver,
            k);
        receiver->gate_input_array[k] = NULL;
    } else {
        // if its a boolean signal
        receiver->signal_input_array[k] = NULL;
    }
}

/*
 * returns the number of gate inputs into which given gates outputs connected
 * @returns number of outputs or -1 if NULL passed
 */
ssize_t nand_fan_out(nand_t const *g) {
    if (g) {
        return ll_length(g->connected_to_output_list);
    } else {
        errno = EINVAL;
        return -1;
    }
}

/*
 * returns ptr to gate or bool signal connected to the k-th input of g
 * if nothing is connected to the k-th input returns null
 */
void *nand_input(nand_t const *g, unsigned k) {
    if (g->gate_input_array[k]) {
        return g->gate_input_array[k];
    } else {
        return g->signal_input_array[k];
    }
}

/*
 * gets kth gate connected to the output of given gate
 */
nand_t *nand_output(nand_t const *g, ssize_t k) {
    nand_t *output_connected_gate;
    unsigned index; // * not used
    ll_get_kth_element(g->connected_to_output_list, k, &output_connected_gate,
                       &index);
    return output_connected_gate;
}

int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
    if (!g_out || !g_in || k > g_in->number_of_inputs - 1) {
        errno = EINVAL;
        return -1;
    }

    // only setting output can fail
    //  TODO ENUM for return value?
    int error = ll_add_element(g_out->connected_to_output_list, g_in, k);
    if (error) {
        errno = ENOMEM;
        return -1;
    }
    unplug_given_input(g_in,
                       k); // should only be done when memerror is possible
    g_in->gate_input_array[k] = g_out;
    return 0;
}

int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (!s || !g || k > g->number_of_inputs - 1) {
        errno = EINVAL;
        return -1;
    }

    // * memerror cant happen??
    unplug_given_input(g, k);
    g->signal_input_array[k] = s;
    return 0;
}

static bool nand_helper_evaluate(nand_t *g, ssize_t *current_depth,
                                 ssize_t *max_depth) {
    if (*current_depth > *max_depth) {
        *max_depth = *current_depth;
    }

    bool and_result = true;
    for (unsigned i = 0; i < g->number_of_inputs; i++) {
        if (g->signal_input_array[i]) {
            and_result &= *g->signal_input_array[i]; // * does & work?
        } else if (g->gate_input_array[i]) {
            *current_depth++;
            and_result &= nand_helper_evaluate(g->gate_input_array[i],
                                               current_depth, max_depth);
            *current_depth--;
        } else {
            and_result = false; // lack of input treated as a false
            // function return false iff all inputs were true
        }
    }

    return !and_result;
}

ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    ssize_t critical_path = 0; // TODO verify recursion logic and base cases
    for (size_t i = 0; i < m; i++) {
        ssize_t current_critical_path = 0;
        s[i] =
            nand_helper_evaluate(g[i], &current_critical_path, &critical_path);
    }

    return critical_path;
}