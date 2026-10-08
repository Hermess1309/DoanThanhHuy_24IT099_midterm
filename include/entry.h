#ifndef ENTRY_H
#define ENTRY_H

#include "ls.h"

/*
 * Initialize a file entry by collecting its metadata using stat/lstat.
 * Resolves permissions, owners, timestamps, formatting, and classification suffixes.
 *
 * Parameters:
 *   entry: pointer to the file_entry_t structure to populate
 *   dirpath: parent directory path, or NULL if path is already complete
 *   filename: file basename
 *   opts: command line options controlling formatting and time selection
 *
 * Returns 0 on success, or -1 on stat error.
 */
int entry_init(file_entry_t *entry, const char *dirpath, const char *filename, const ls_options_t *opts);

/*
 * Free dynamically allocated members of a file_entry_t.
 */
void entry_free(file_entry_t *entry);

/*
 * Humanize a byte size into standard representation (e.g., 512B, 4.0K, 1.2M).
 */
void entry_format_human_size(int64_t bytes, char *buf, size_t buflen);

#endif /* ENTRY_H */
