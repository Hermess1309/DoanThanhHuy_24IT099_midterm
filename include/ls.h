#ifndef LS_H
#define LS_H

#define _NETBSD_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE   700

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#ifdef __NetBSD__
#include <util.h>
#endif

/* Configuration options structure for ls */
typedef struct {
    bool opt_A;      /* -A: list all except '.' and '..' (default for super-user) */
    bool opt_a;      /* -a: include directory entries starting with '.' */
    bool opt_c;      /* -c: use status change time (st_ctime) for sort (-t) or print (-l) */
    bool opt_d;      /* -d: directories as plain files, no recursive search */
    bool opt_F;      /* -F: append type indicator: / * @ % = | */
    bool opt_f;      /* -f: output is not sorted */
    bool opt_h;      /* -h: human-readable sizes (bytes, KB, MB, etc.) */
    bool opt_i;      /* -i: print inode number */
    bool opt_k;      /* -k: sizes reported in kilobytes */
    bool opt_l;      /* -l: long listing format */
    bool opt_n;      /* -n: numeric UID and GID long format */
    bool opt_q;      /* -q: replace non-printable characters with '?' */
    bool opt_R;      /* -R: recursively list subdirectories */
    bool opt_r;      /* -r: reverse sort order */
    bool opt_S;      /* -S: sort by size (largest first) */
    bool opt_s;      /* -s: display filesystem block count */
    bool opt_t;      /* -t: sort by time modified (newest first) */
    bool opt_u;      /* -u: use access time (st_atime) for sort (-t) or print (-l) */
    bool opt_w;      /* -w: force raw printing of non-printable characters */

    long blocksize;  /* Block size in bytes (default 512, or env BLOCKSIZE, or 1024 for -k) */
    int exit_status; /* 0 on success, >0 if any error occurred */
} ls_options_t;

/* Representation of a single file/directory entry */
typedef struct {
    char *name;              /* Basename */
    char *path;              /* Full or relative pathname */
    struct stat st;          /* Stat metadata */
    int stat_ok;             /* 0 on success, -1 on stat error */
    char *link_target;       /* Target if symbolic link */
    char mode_str[12];       /* e.g., "-rwxr-xr-x" */
    char user_str[36];       /* User name or numeric UID */
    char group_str[36];      /* Group name or numeric GID */
    char size_str[36];       /* Size or "major, minor" */
    char block_str[36];      /* Formatted block count */
    int64_t display_blocks;  /* Numerical block count for directory total */
    char date_str[36];       /* Formatted date string */
    char suffix;             /* Type suffix for -F ('\0' if none) */
    time_t selected_time;    /* Time selected for sorting/printing */
    long selected_nsec;      /* Nanoseconds for tie-breaking */
} file_entry_t;

#endif /* LS_H */
