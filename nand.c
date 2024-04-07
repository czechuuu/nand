// todo prettier comments
#include "nand.h"
#include "llist.h"
#include <errno.h>
#include <stdlib.h>

// TODO can (should) i do this??
static void unplug_all_io(nand_t *);
static void unplug_given_input(nand_t *receiver, unsigned k);

typedef struct dfs_info {
    bool visited;
    bool currently_visiting;
} dfs_info_t;

typedef struct evaluation_buffer {
    bool available;
    bool value;
} evaluation_buffer_t;

/*
 * under a given index i,  0 or 1 of the folowing should be nonnull:
 * - gate_input_array[i]
 * - signal_input_array[i]
 * i.e. cant have two inputs under the same index
 */
struct nand {
    unsigned number_of_inputs;
    bool const **signal_input_array;
    nand_t **gate_input_array;
    llist_t *connected_to_output_list;
    evaluation_buffer_t buffered_output;
    dfs_info_t cycle_detection_information;
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
                    new_gate->buffered_output.available = false;
                    new_gate->cycle_detection_information.visited = false;
                    new_gate->cycle_detection_information.currently_visiting =
                        false;
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
    if (!g || k >= g->number_of_inputs) {
        errno = EINVAL;
        return NULL;
    }

    if (g->gate_input_array[k]) {
        return g->gate_input_array[k];
    } else if (g->signal_input_array[k]) {
        return g->signal_input_array[k];
    } else {
        errno = 0;
        return NULL;
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

/*
 * resets all the dfs visited information
 */
static void nand_clear_dfs_info(nand_t *g) {
    if (g) {
        g->cycle_detection_information.visited = false;
        g->cycle_detection_information.currently_visiting = false;

        int n = g->number_of_inputs;
        for (size_t i = 0; i < n; i++) {
            nand_t *current_input = g->gate_input_array[i];
            // second part (of the if) should make it work even on cyclic
            // systems
            if (current_input &&
                current_input->cycle_detection_information.visited) {
                nand_clear_dfs_info(g->gate_input_array[i]);
            }
        }
    }
}

/*
 ! sets errno
 * a valid system:
 * - is nonnull (if not EINVAL)
 * - has no cycles (if not ECANCELED)
 * - has no components with missing inputs (if not EINVAL)
 */
static bool nand_validate_gate_system(nand_t *g) {
    if (!g) {
        errno = EINVAL;
        return false;
    }

    g->cycle_detection_information.visited = true;
    g->cycle_detection_information.currently_visiting = true;

    int n = g->number_of_inputs;
    for (size_t i = 0; i < n; i++) {
        nand_t *current_input = g->gate_input_array[i];
        if (current_input) { // could be a bool signal
            if (!current_input->cycle_detection_information.visited) {
                if (!nand_validate_gate_system(current_input)) {
                    return false; // propagating the invalidity of children
                                  // errno already set by them
                }
            } else if (current_input->cycle_detection_information
                           .currently_visiting) {
                // if both are true then this gates recursive calls led us here
                errno = ECANCELED;
                return false;
            }
        } else if (!g->signal_input_array[i]) {
            // gate doesnt have a gate input and a signal input under curr index
            errno = ECANCELED; // !what should it be
            return false;
        }
    }

    g->cycle_detection_information.currently_visiting = false;
    return true;
}

/*
 * given a gate array of size n checks if each entry represents a valid system
 * see above fucntion for validity definition
 */
static bool nand_validate_gate_system_array(nand_t **gs, size_t n) {
    for (size_t i = 0; i < n; i++) {
        nand_t *current_gate_system = gs[i];
        // kind of fishy if array of nulls passed
        nand_clear_dfs_info(current_gate_system); // make sure all are unvisited
        if (!nand_validate_gate_system(current_gate_system)) {
            return false;
        }
    }

    return true;
}

/*
 * assumes acyclic system
 */
static void nand_clear_evaluation_buffer(nand_t *g) {
    g->buffered_output.available = false;

    int n = g->number_of_inputs;
    for (size_t i = 0; i < n; i++) {
        nand_t *current_input = g->gate_input_array[i];
        if (current_input) {
            nand_clear_evaluation_buffer(g->gate_input_array[i]);
        }
    }
}

static void nand_clear_evaluation_buffer_array(nand_t **gs, size_t n) {
    for (size_t i = 0; i < n; i++) {
        nand_clear_evaluation_buffer(gs[i]);
    }
}

static bool nand_helper_evaluate(nand_t *g, ssize_t current_depth,
                                 ssize_t *max_depth) {
    //! i dont like this
    if (g->number_of_inputs > 0) {
        current_depth++;
    }

    if (current_depth > *max_depth) {
        *max_depth = current_depth;
    }

    bool and_result = true;
    for (unsigned i = 0; i < g->number_of_inputs; i++) {
        if (g->signal_input_array[i]) {
            and_result &= *g->signal_input_array[i]; // * does & work?
        } else if (g->gate_input_array[i]) {
            and_result &= nand_helper_evaluate(g->gate_input_array[i],
                                               current_depth, max_depth);
        }
    }

    return !and_result;
}

ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
    if (!g || !s || m == 0) {
        errno = EINVAL;
        return -1;
    }

    if (!nand_validate_gate_system_array(g, m)) {
        // errno already set
        return -1;
    }

    //! i dont understand how the critical path calculation works
    ssize_t critical_path = 0; // TODO verify recursion logic and base cases
    for (size_t i = 0; i < m; i++) {
        nand_t *current_gate = g[i];
        if (!current_gate->buffered_output.available) {
            current_gate->buffered_output.value =
                nand_helper_evaluate(current_gate, 0, &critical_path);
            current_gate->buffered_output.available = true;
        }
        s[i] = current_gate->buffered_output.value;
    }

    nand_clear_evaluation_buffer_array(g, m);
    return critical_path;
}