#ifndef OPTIONS_H
#define OPTIONS_H

#include "ls.h"

/*
 * Initialize the options structure with default values based on
 * terminal detection, super-user privilege, and environment variables.
 */
void options_init(ls_options_t *opts);

/*
 * Parse command-line flags and handle mutual overrides according to the manual.
 * Returns the index of the first operand (optind) on success, or -1 on error.
 */
int options_parse(int argc, char **argv, ls_options_t *opts);

/* Print standard usage string to stderr */
void options_print_usage(const char *progname);

#endif /* OPTIONS_H */
