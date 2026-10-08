#include "ls.h"
#include "options.h"
#include "traverse.h"

int main(int argc, char **argv) {
    ls_options_t opts;
    options_init(&opts);

    int opt_idx = options_parse(argc, argv, &opts);
    if (opt_idx < 0) {
        return opts.exit_status > 0 ? opts.exit_status : 1;
    }

    int operand_count = argc - opt_idx;
    char **operands = &argv[opt_idx];

    traverse_operands(operand_count, operands, &opts);

    return opts.exit_status > 0 ? 1 : 0;
}
