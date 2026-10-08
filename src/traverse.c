#include "traverse.h"
#include "entry.h"
#include "sort.h"
#include "display.h"

static bool should_include_entry(const char *name, const ls_options_t *opts) {
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return opts->opt_a;
    }
    if (name[0] == '.') {
        return opts->opt_a || opts->opt_A;
    }
    return true;
}

int traverse_directory(const char *dirpath, ls_options_t *opts, bool print_header, bool *need_newline) {
    if (!dirpath || !opts) return -1;

    DIR *dir = opendir(dirpath);
    if (!dir) {
        fprintf(stderr, "ls: %s: %s\n", dirpath, strerror(errno));
        opts->exit_status = 1;
        return -1;
    }

    if (print_header) {
        if (need_newline && *need_newline) {
            putchar('\n');
        }
        printf("%s:\n", dirpath);
        if (need_newline) *need_newline = true;
    }

    /* Collect directory contents */
    size_t capacity = 32;
    size_t count = 0;
    file_entry_t *entries = malloc(capacity * sizeof(file_entry_t));
    if (!entries) {
        closedir(dir);
        fprintf(stderr, "ls: out of memory\n");
        opts->exit_status = 1;
        return -1;
    }

    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL) {
        if (!should_include_entry(dp->d_name, opts)) {
            continue;
        }

        if (count >= capacity) {
            capacity *= 2;
            file_entry_t *new_entries = realloc(entries, capacity * sizeof(file_entry_t));
            if (!new_entries) {
                fprintf(stderr, "ls: out of memory\n");
                opts->exit_status = 1;
                break;
            }
            entries = new_entries;
        }

        if (entry_init(&entries[count], dirpath, dp->d_name, opts) == 0) {
            count++;
        } else {
            /* Stat error on child entry */
            fprintf(stderr, "ls: %s/%s: %s\n", dirpath, dp->d_name, strerror(errno));
            opts->exit_status = 1;
        }
    }
    closedir(dir);

    /* Sort entries */
    sort_entries(entries, count, opts);

    /* Print total blocks if long format or (-s is active AND stdout is a terminal) */
    bool show_total = (opts->opt_l || opts->opt_n || (opts->opt_s && isatty(STDOUT_FILENO)));
    if (show_total) {
        int64_t total_blocks = 0;
        for (size_t i = 0; i < count; i++) {
            total_blocks += entries[i].display_blocks;
        }
        display_print_total(total_blocks, opts);
    }

    /* Display all entries */
    display_entries(entries, count, opts);
    if (need_newline) *need_newline = true;

    /* Handle recursive traversal (-R) if enabled */
    if (opts->opt_R && !opts->opt_d) {
        /*
         * Collect child directories for recursive listing.
         * NetBSD ls traverses directories encountering them, excluding '.' and '..'
         * and without following symbolic links.
         */
        for (size_t i = 0; i < count; i++) {
            if (S_ISDIR(entries[i].st.st_mode) &&
                strcmp(entries[i].name, ".") != 0 &&
                strcmp(entries[i].name, "..") != 0) {
                traverse_directory(entries[i].path, opts, true, need_newline);
            }
        }
    }

    /* Clean up allocated resources */
    for (size_t i = 0; i < count; i++) {
        entry_free(&entries[i]);
    }
    free(entries);

    return 0;
}

int traverse_operands(int count, char **operands, ls_options_t *opts) {
    if (!opts) return -1;

    bool need_newline = false;

    /* If no operands are provided, default to listing the current directory */
    if (count == 0) {
        return traverse_directory(".", opts, false, &need_newline);
    }

    /*
     * Separate operands into:
     * 1. Non-directory files (and directories if -d is set)
     * 2. Directories to be traversed
     */
    size_t file_cap = 16, file_cnt = 0;
    file_entry_t *file_entries = malloc(file_cap * sizeof(file_entry_t));

    size_t dir_cap = 16, dir_cnt = 0;
    file_entry_t *dir_entries = malloc(dir_cap * sizeof(file_entry_t));

    if (!file_entries || !dir_entries) {
        fprintf(stderr, "ls: out of memory\n");
        opts->exit_status = 1;
        free(file_entries);
        free(dir_entries);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        const char *op = operands[i];
        struct stat st;
        int stat_res;

        if (opts->opt_d) {
            stat_res = lstat(op, &st);
        } else {
            /* Try stat first to resolve symlinks pointing to directories */
            stat_res = stat(op, &st);
            if (stat_res != 0) {
                stat_res = lstat(op, &st);
            }
        }

        if (stat_res != 0) {
            fprintf(stderr, "ls: %s: %s\n", op, strerror(errno));
            opts->exit_status = 1;
            continue;
        }

        if (opts->opt_d || !S_ISDIR(st.st_mode)) {
            /* Treat as file entry */
            if (file_cnt >= file_cap) {
                file_cap *= 2;
                file_entries = realloc(file_entries, file_cap * sizeof(file_entry_t));
            }
            if (entry_init(&file_entries[file_cnt], NULL, op, opts) == 0) {
                file_cnt++;
            }
        } else {
            /* Treat as directory to be traversed */
            if (dir_cnt >= dir_cap) {
                dir_cap *= 2;
                dir_entries = realloc(dir_entries, dir_cap * sizeof(file_entry_t));
            }
            if (entry_init(&dir_entries[dir_cnt], NULL, op, opts) == 0) {
                dir_cnt++;
            }
        }
    }

    /* Sort non-directory and directory entries separately */
    sort_entries(file_entries, file_cnt, opts);
    sort_entries(dir_entries, dir_cnt, opts);

    /* 1. Display non-directory files first */
    if (file_cnt > 0) {
        display_entries(file_entries, file_cnt, opts);
        need_newline = true;
    }

    /* Clean up file entries */
    for (size_t i = 0; i < file_cnt; i++) {
        entry_free(&file_entries[i]);
    }
    free(file_entries);

    /* 2. Display directories */
    bool print_header = (count > 1 || file_cnt > 0);
    for (size_t i = 0; i < dir_cnt; i++) {
        traverse_directory(dir_entries[i].name, opts, print_header, &need_newline);
    }

    /* Clean up directory entries */
    for (size_t i = 0; i < dir_cnt; i++) {
        entry_free(&dir_entries[i]);
    }
    free(dir_entries);

    return 0;
}
