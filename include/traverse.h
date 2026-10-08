#ifndef TRAVERSE_H
#define TRAVERSE_H

#include "ls.h"

/*
 * Process all command-line operands:
 * - Separates operands into non-directory files and directories
 * - Displays non-directory files first, sorted
 * - Displays directories subsequently with headers as required
 * - Handles recursive directory traversals (-R)
 * - Reports errors for inaccessible files to stderr with exit_status = 1
 */
int traverse_operands(int count, char **operands, ls_options_t *opts);

/*
 * Read and display the contents of a single directory.
 * If -R is enabled, recursively traverses child directories.
 */
int traverse_directory(const char *dirpath, ls_options_t *opts, bool print_header, bool *need_newline);

#endif /* TRAVERSE_H */
