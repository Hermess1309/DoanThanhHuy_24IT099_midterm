#include "entry.h"

void entry_format_human_size(int64_t bytes, char *buf, size_t buflen) {
    if (!buf || buflen == 0) return;

#if defined(HN_AUTOSCALE)
    /* Utilize native NetBSD humanize_number with standard width 5 and HN_DECIMAL */
    if (humanize_number(buf, 5, bytes, "", HN_AUTOSCALE, HN_B | HN_NOSPACE | HN_DECIMAL) >= 0) {
        return;
    }
#endif

    /* Robust fallback implementation following NetBSD human-readable format */
    const char *prefixes[] = {"B", "K", "M", "G", "T", "P", "E"};
    int pidx = 0;
    double val = (double)bytes;

    if (val < 0) val = 0;

    if (val < 1000.0) {
        snprintf(buf, buflen, "%lldB", (long long)bytes);
        return;
    }

    while (val >= 1000.0 && pidx < 6) {
        val /= 1024.0;
        pidx++;
    }

    if (val < 10.0) {
        snprintf(buf, buflen, "%.1f%s", val, prefixes[pidx]);
    } else {
        snprintf(buf, buflen, "%.0f%s", val, prefixes[pidx]);
    }
}

int entry_init(file_entry_t *entry, const char *dirpath, const char *filename, const ls_options_t *opts) {
    if (!entry || !filename) return -1;

    memset(entry, 0, sizeof(file_entry_t));
    entry->name = strdup(filename);

    /* Construct entry full path */
    if (!dirpath || dirpath[0] == '\0') {
        entry->path = strdup(filename);
    } else {
        size_t dlen = strlen(dirpath);
        size_t flen = strlen(filename);
        size_t total_len = dlen + flen + 2;
        entry->path = malloc(total_len);
        if (!entry->path) {
            entry->path = strdup(filename);
        } else {
            if (dirpath[dlen - 1] == '/') {
                snprintf(entry->path, total_len, "%s%s", dirpath, filename);
            } else {
                snprintf(entry->path, total_len, "%s/%s", dirpath, filename);
            }
        }
    }

    /* Retrieve metadata using lstat */
    if (lstat(entry->path, &entry->st) != 0) {
        entry->stat_ok = -1;
        return -1;
    }
    entry->stat_ok = 0;

    mode_t mode = entry->st.st_mode;

    /* 1. Format 10-character file mode string */
    /* File type indicator */
    if (S_ISDIR(mode)) entry->mode_str[0] = 'd';
    else if (S_ISLNK(mode)) entry->mode_str[0] = 'l';
    else if (S_ISCHR(mode)) entry->mode_str[0] = 'c';
    else if (S_ISBLK(mode)) entry->mode_str[0] = 'b';
    else if (S_ISSOCK(mode)) entry->mode_str[0] = 's';
    else if (S_ISFIFO(mode)) entry->mode_str[0] = 'p';
#ifdef S_ISWHT
    else if (S_ISWHT(mode)) entry->mode_str[0] = 'w';
#endif
    else if (S_ISREG(mode)) entry->mode_str[0] = '-';
    else entry->mode_str[0] = '?';

    /* User permissions */
    entry->mode_str[1] = (mode & S_IRUSR) ? 'r' : '-';
    entry->mode_str[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        entry->mode_str[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        entry->mode_str[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    /* Group permissions */
    entry->mode_str[4] = (mode & S_IRGRP) ? 'r' : '-';
    entry->mode_str[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        entry->mode_str[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        entry->mode_str[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    /* Other permissions */
    entry->mode_str[7] = (mode & S_IROTH) ? 'r' : '-';
    entry->mode_str[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        entry->mode_str[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        entry->mode_str[9] = (mode & S_IXOTH) ? 'x' : '-';
    }
    entry->mode_str[10] = '\0';

    /* 2. Format user and group identification */
    if (opts && opts->opt_n) {
        snprintf(entry->user_str, sizeof(entry->user_str), "%u", (unsigned int)entry->st.st_uid);
        snprintf(entry->group_str, sizeof(entry->group_str), "%u", (unsigned int)entry->st.st_gid);
    } else {
        struct passwd *pwd = getpwuid(entry->st.st_uid);
        if (pwd && pwd->pw_name) {
            snprintf(entry->user_str, sizeof(entry->user_str), "%s", pwd->pw_name);
        } else {
            snprintf(entry->user_str, sizeof(entry->user_str), "%u", (unsigned int)entry->st.st_uid);
        }

        struct group *grp = getgrgid(entry->st.st_gid);
        if (grp && grp->gr_name) {
            snprintf(entry->group_str, sizeof(entry->group_str), "%s", grp->gr_name);
        } else {
            snprintf(entry->group_str, sizeof(entry->group_str), "%u", (unsigned int)entry->st.st_gid);
        }
    }

    /* 3. Format file size / device numbers */
    if (S_ISCHR(mode) || S_ISBLK(mode)) {
        snprintf(entry->size_str, sizeof(entry->size_str), "%u, %u",
                 (unsigned int)major(entry->st.st_rdev),
                 (unsigned int)minor(entry->st.st_rdev));
    } else if (opts && opts->opt_h) {
        entry_format_human_size(entry->st.st_size, entry->size_str, sizeof(entry->size_str));
    } else {
        snprintf(entry->size_str, sizeof(entry->size_str), "%lld", (long long)entry->st.st_size);
    }

    /* 4. Format filesystem block count */
    int64_t actual_bytes = (int64_t)entry->st.st_blocks * 512;
    if (opts && opts->opt_h) {
        entry_format_human_size(entry->st.st_size, entry->block_str, sizeof(entry->block_str));
        /* For directory total, NetBSD ls -lh reports total in human format */
        entry->display_blocks = actual_bytes;
    } else {
        long bs = (opts && opts->blocksize > 0) ? opts->blocksize : 512;
        int64_t blks = (actual_bytes + bs - 1) / bs;
        entry->display_blocks = blks;
        snprintf(entry->block_str, sizeof(entry->block_str), "%lld", (long long)blks);
    }

    /* 5. Select time for sorting and printing */
    if (opts && opts->opt_c) {
        entry->selected_time = entry->st.st_ctime;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
        entry->selected_nsec = entry->st.st_ctimensec;
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
        entry->selected_nsec = entry->st.st_ctim.tv_nsec;
#else
        entry->selected_nsec = 0;
#endif
    } else if (opts && opts->opt_u) {
        entry->selected_time = entry->st.st_atime;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
        entry->selected_nsec = entry->st.st_atimensec;
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
        entry->selected_nsec = entry->st.st_atim.tv_nsec;
#else
        entry->selected_nsec = 0;
#endif
    } else {
        entry->selected_time = entry->st.st_mtime;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
        entry->selected_nsec = entry->st.st_mtimensec;
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
        entry->selected_nsec = entry->st.st_mtim.tv_nsec;
#else
        entry->selected_nsec = 0;
#endif
    }

    /* Format timestamp (traditional BSD format: %b %e %H:%M or %b %e  %Y) */
    time_t now = time(NULL);
    struct tm *lt = localtime(&entry->selected_time);
    if (lt) {
        /* If modified within roughly 6 months (180 days) */
        if (entry->selected_time <= now + 3600 && entry->selected_time >= now - 180 * 24 * 3600) {
            strftime(entry->date_str, sizeof(entry->date_str), "%b %e %H:%M", lt);
        } else {
            strftime(entry->date_str, sizeof(entry->date_str), "%b %e  %Y", lt);
        }
    } else {
        snprintf(entry->date_str, sizeof(entry->date_str), "Jan  1  1970");
    }

    /* 6. Classification suffix (-F) */
    entry->suffix = '\0';
    if (opts && opts->opt_F) {
        if (S_ISDIR(mode)) {
            entry->suffix = '/';
        } else if (S_ISLNK(mode)) {
            entry->suffix = '@';
        } else if (S_ISSOCK(mode)) {
            entry->suffix = '=';
        } else if (S_ISFIFO(mode)) {
            entry->suffix = '|';
#ifdef S_ISWHT
        } else if (S_ISWHT(mode)) {
            entry->suffix = '%';
#endif
        } else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
            entry->suffix = '*';
        }
    }

    /* 7. Readlink target for symbolic links */
    entry->link_target = NULL;
    if (S_ISLNK(mode) && opts && (opts->opt_l || opts->opt_n)) {
        char target_buf[PATH_MAX + 1];
        ssize_t len = readlink(entry->path, target_buf, sizeof(target_buf) - 1);
        if (len != -1) {
            target_buf[len] = '\0';
            entry->link_target = strdup(target_buf);
        }
    }

    return 0;
}

void entry_free(file_entry_t *entry) {
    if (!entry) return;
    if (entry->name) {
        free(entry->name);
        entry->name = NULL;
    }
    if (entry->path) {
        free(entry->path);
        entry->path = NULL;
    }
    if (entry->link_target) {
        free(entry->link_target);
        entry->link_target = NULL;
    }
}
