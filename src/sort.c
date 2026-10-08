#include "sort.h"

static const ls_options_t *s_active_opts = NULL;

static int compare_file_entries(const void *p1, const void *p2) {
    const file_entry_t *a = (const file_entry_t *)p1;
    const file_entry_t *b = (const file_entry_t *)p2;
    int cmp = 0;

    if (s_active_opts && s_active_opts->opt_S) {
        /* Sort by size: largest first */
        if (a->st.st_size > b->st.st_size) {
            cmp = -1;
        } else if (a->st.st_size < b->st.st_size) {
            cmp = 1;
        } else {
            cmp = strcmp(a->name, b->name);
        }
    } else if (s_active_opts && s_active_opts->opt_t) {
        /* Sort by time: most recent first */
        if (a->selected_time > b->selected_time) {
            cmp = -1;
        } else if (a->selected_time < b->selected_time) {
            cmp = 1;
        } else {
            /* Nanosecond tie-breaker */
            if (a->selected_nsec > b->selected_nsec) {
                cmp = -1;
            } else if (a->selected_nsec < b->selected_nsec) {
                cmp = 1;
            } else {
                cmp = strcmp(a->name, b->name);
            }
        }
    } else {
        /* Default: lexicographical sort */
        cmp = strcmp(a->name, b->name);
    }

    /* Reverse sort if -r is requested */
    if (s_active_opts && s_active_opts->opt_r) {
        cmp = -cmp;
    }

    return cmp;
}

void sort_entries(file_entry_t *entries, size_t count, const ls_options_t *opts) {
    if (!entries || count <= 1 || !opts) return;

    /* If -f is specified, output is not sorted */
    if (opts->opt_f) {
        return;
    }

    s_active_opts = opts;
    qsort(entries, count, sizeof(file_entry_t), compare_file_entries);
    s_active_opts = NULL;
}
