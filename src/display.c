#include "display.h"
#include "entry.h"

void display_print_total(int64_t total_blocks, const ls_options_t *opts) {
    if (!opts) return;

    if (opts->opt_h) {
        char buf[32];
        entry_format_human_size(total_blocks, buf, sizeof(buf));
        printf("total %s\n", buf);
    } else {
        printf("total %lld\n", (long long)total_blocks);
    }
}

void display_print_filename(const char *name, bool opt_q) {
    if (!name) return;

    for (const unsigned char *p = (const unsigned char *)name; *p; p++) {
        if (opt_q && !isprint(*p)) {
            putchar('?');
        } else {
            putchar(*p);
        }
    }
}

void display_entries(const file_entry_t *entries, size_t count, const ls_options_t *opts) {
    if (!entries || count == 0 || !opts) return;

    int max_inode_w = 0;
    int max_block_w = 0;
    int max_nlink_w = 0;
    int max_user_w = 0;
    int max_group_w = 0;
    int max_size_w = 0;

    /* Compute column widths for proper alignment */
    for (size_t i = 0; i < count; i++) {
        if (opts->opt_i) {
            char tmp[32];
            int len = snprintf(tmp, sizeof(tmp), "%llu", (unsigned long long)entries[i].st.st_ino);
            if (len > max_inode_w) max_inode_w = len;
        }
        if (opts->opt_s) {
            int len = (int)strlen(entries[i].block_str);
            if (len > max_block_w) max_block_w = len;
        }
        if (opts->opt_l || opts->opt_n) {
            char tmp[32];
            int len = snprintf(tmp, sizeof(tmp), "%lu", (unsigned long)entries[i].st.st_nlink);
            if (len > max_nlink_w) max_nlink_w = len;

            len = (int)strlen(entries[i].user_str);
            if (len > max_user_w) max_user_w = len;

            len = (int)strlen(entries[i].group_str);
            if (len > max_group_w) max_group_w = len;

            len = (int)strlen(entries[i].size_str);
            if (len > max_size_w) max_size_w = len;
        }
    }

    /* Print each entry */
    for (size_t i = 0; i < count; i++) {
        const file_entry_t *e = &entries[i];

        /* Inode column (-i) */
        if (opts->opt_i) {
            printf("%*llu ", max_inode_w, (unsigned long long)e->st.st_ino);
        }

        /* Block size column (-s) */
        if (opts->opt_s) {
            printf("%*s ", max_block_w, e->block_str);
        }

        /* Long format fields (-l, -n) */
        if (opts->opt_l || opts->opt_n) {
            printf("%s  %*lu %-*s  %-*s  %*s %s ",
                   e->mode_str,
                   max_nlink_w, (unsigned long)e->st.st_nlink,
                   max_user_w, e->user_str,
                   max_group_w, e->group_str,
                   max_size_w, e->size_str,
                   e->date_str);
        }

        /* File name */
        display_print_filename(e->name, opts->opt_q);

        /* Type suffix (-F) */
        if (opts->opt_F && e->suffix != '\0') {
            putchar(e->suffix);
        }

        /* Link destination for symbolic links in long listing */
        if ((opts->opt_l || opts->opt_n) && e->link_target != NULL) {
            printf(" -> %s", e->link_target);
        }

        putchar('\n');
    }
}
