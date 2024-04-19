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
    nand_t *const new_gate = malloc(sizeof(nand_t));
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
                // if were here then only list failed
                // so we dont have to free anything
            }
            free(new_gate->gate_input_array); // in case list failed
        }
        free(new_gate->signal_input_array); // in case list or gate array failed
    }
    free(new_gate); // in case list or any array failed
    // if we havent left the function in the innermost if statement sth failed
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
 * internal function that unplugs all inputs and outputs of a given gate
 * assumes non null ptr given
 * @param gate gate which should be fully unplugged
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
 * internal function that unplugs given input
 * if it was a gate then also unplugs from the output list
 * @param receiver gate to which to input is connected
 * @param k index of the input
 */
static void unplug_given_input(nand_t *receiver, unsigned k) {
    nand_t const *const gate_connected_to_given_input =
        receiver->gate_input_array[k];
    if (gate_connected_to_given_input) {
        // if its a gate
        ll_delete_element_with_given_value(
            gate_connected_to_given_input->connected_to_output_list, receiver,
            k);
        receiver->gate_input_array[k] = NULL;
    } else {
        // if its a boolean signal

        // its possible to lose access to the bool signal
        // if its the only place where its stored
        // but thats not our problem
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
 * @param g gate
 * @param k input index
 */
void *nand_input(nand_t const *g, unsigned k) {
    if (!g || k >= g->number_of_inputs) {
        errno = EINVAL;
        return NULL;
    }

    if (g->gate_input_array[k]) {
        return (nand_t *)g->gate_input_array[k];
    } else if (g->signal_input_array[k]) {
        return (bool *)g->signal_input_array[k];
    } else {
        errno = 0;
        return NULL;
    }
}

/*
 * gets kth gate connected to the output of given gate
 * @param g gate
 * @param k output index
 * @returns gate connected to that output
 */
nand_t *nand_output(nand_t const *g, ssize_t k) {
    nand_t *output_connected_gate;
    unsigned index; // not used but llist interface has to return sth
    ll_get_kth_element(g->connected_to_output_list, k, &output_connected_gate,
                       &index);
    return output_connected_gate;
}

/*
 * connects a gate as an input
 * @param g_out the gate which will be connected
 * @param g_in the gate into which it will be connnected
 * @param k input index
 * @returns 0 on success -1 otherwise
 */
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
    if (!g_out || !g_in || k > g_in->number_of_inputs - 1) {
        errno = EINVAL;
        return -1;
    }

    // only setting output can fail
    int const error = ll_add_element(g_out->connected_to_output_list, g_in, k);
    if (error) {
        errno = ENOMEM;
        return -1;
    }
    unplug_given_input(g_in,
                       k); // should only be done when memerror is possible
    g_in->gate_input_array[k] = g_out;
    return 0;
}

/*
 * connects a signal as an input
 * @param s the boolean signal which will be connected
 * @param g_in the gate into which it will be connnected
 * @param k input index
 * @returns 0 on success -1 otherwise
 */
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
    if (!s || !g || k > g->number_of_inputs - 1) {
        errno = EINVAL;
        return -1;
    }

    // memerror cant happen
    unplug_given_input(g, k);
    g->signal_input_array[k] = s;
    return 0;
}

/*
 * internal funtion that clears visited info used by dfs
 * @param g gate which should be cleared
 */
static void nand_clear_dfs_info(nand_t *g) {
    if (g) {
        g->cycle_detection_information.visited = false;
        g->cycle_detection_information.currently_visiting = false;

        unsigned const n = g->number_of_inputs;
        for (size_t i = 0; i < n; i++) {
            nand_t *const current_input = g->gate_input_array[i];
            // second part (of the if) should make it work even on cyclic
            // systems
            if (current_input &&
                current_input->cycle_detection_information.visited) {
                nand_clear_dfs_info(current_input);
            }
        }
    }
}

/*
 * internal function that checks if a gate system is valid,
 * a valid system:
 * - is nonnull (if not EINVAL)
 * - has no cycles (if not ECANCELED)
 * - has no components with missing inputs (if not EINVAL)
 *
 * @returns true iff system is valid otherwise sets ERRNO
 */
static bool nand_validate_gate_system(nand_t *g) {
    if (!g) {
        errno = EINVAL;
        return false;
    }

    g->cycle_detection_information.visited = true;
    g->cycle_detection_information.currently_visiting = true;

    unsigned const n = g->number_of_inputs;
    for (size_t i = 0; i < n; i++) {
        nand_t *const current_input = g->gate_input_array[i];
        if (current_input) { // gate input
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
            errno = ECANCELED;
            return false;
        }
    }

    g->cycle_detection_information.currently_visiting = false;
    return true;
}

/*
 * checks if each entry in a gate system array is a valid system
 * see above for valid definition
 * @param gs gate array
 * @param n array size
 * @returns true iff all entries are valid
 */
static bool nand_validate_gate_system_array(nand_t **gs, size_t n) {
    for (size_t i = 0; i < n; i++) {
        nand_t *const current_gate_system = gs[i];
        nand_clear_dfs_info(current_gate_system); // make sure all are unvisited
        if (!nand_validate_gate_system(current_gate_system)) {
            return false;
        }
    }

    return true;
}

/*
 * clears buffered information about a gates output
 * @param g gate
 */
static void nand_clear_evaluation_buffer(nand_t *g) {
    g->buffered_output.available = false;

    unsigned const n = g->number_of_inputs;
    for (size_t i = 0; i < n; i++) {
        nand_t *const current_input = g->gate_input_array[i];
        if (current_input) {
            nand_clear_evaluation_buffer(g->gate_input_array[i]);
        }
    }
}

/*
 * clears buffered information of each element of gate array
 * @param gs gate array
 * @param n size of array
 */
static void nand_clear_evaluation_buffer_array(nand_t **gs, size_t n) {
    for (size_t i = 0; i < n; i++) {
        nand_clear_evaluation_buffer(gs[i]);
    }
}

/*
 * internal function that handles the recursion
 * needed to evaluate a gates output
 * @param g gate
 * @param current_depth critical path on the output of current gate
 * @param max_depth ptr to max critical path ever reached
 */
static bool nand_helper_evaluate(nand_t *g, ssize_t current_depth,
                                 ssize_t *max_depth) {
    //! i dont like this
    if (g->number_of_inputs > 0) {
        current_depth++;
        // TODO integer overflow
    }

    if (current_depth > *max_depth) {
        *max_depth = current_depth;
    }

    bool and_result = true;
    for (unsigned i = 0; i < g->number_of_inputs; i++) {
        if (g->signal_input_array[i]) {
            and_result &= *g->signal_input_array[i];
        } else if (g->gate_input_array[i]) {
            and_result &= nand_helper_evaluate(g->gate_input_array[i],
                                               current_depth, max_depth);
        }
    }

    return !and_result;
}

/*
 * evaluates the outputs of gate systems
 * @param g gate array
 * @param s boolean result array
 * @param m array size
 * @returns critical path of the system
 */
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