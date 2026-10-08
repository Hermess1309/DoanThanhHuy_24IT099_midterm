#ifndef DISPLAY_H
#define DISPLAY_H

#include "ls.h"

/*
 * Print the "total <blocks>" header line for a directory if -l or -s is enabled.
 */
void display_print_total(int64_t total_blocks, const ls_options_t *opts);

/*
 * Print an array of file entries to stdout according to formatting flags:
 * - One entry per line
 * - -i: includes inode number (right-aligned)
 * - -s: includes block size (right-aligned)
 * - -l / -n: includes full mode, links, owner, group, size, timestamp, and symlink targets
 * - -F: appends file classification indicators
 * - -q / -w: filters non-printable characters or prints raw bytes
 */
void display_entries(const file_entry_t *entries, size_t count, const ls_options_t *opts);

/*
 * Print a single file name with proper handling of non-printable characters (-q vs -w).
 */
void display_print_filename(const char *name, bool opt_q);

#endif /* DISPLAY_H */
