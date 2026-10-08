#ifndef SORT_H
#define SORT_H

#include "ls.h"

/*
 * Sort an array of file entries based on command-line options:
 * - Default: lexicographical order by filename
 * - -S: largest size first
 * - -t: newest timestamp first (mtime, or ctime with -c, or atime with -u)
 * - -r: reverses the sorting order
 * - -f: leaves entries unsorted (no-op)
 */
void sort_entries(file_entry_t *entries, size_t count, const ls_options_t *opts);

#endif /* SORT_H */
