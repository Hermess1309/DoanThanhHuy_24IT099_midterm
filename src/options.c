#include "options.h"

void options_init(ls_options_t *opts) {
    if (!opts) return;

    memset(opts, 0, sizeof(ls_options_t));

    /* Check terminal for default non-printable format: -q for tty, -w otherwise */
    if (isatty(STDOUT_FILENO)) {
        opts->opt_q = true;
        opts->opt_w = false;
    } else {
        opts->opt_w = true;
        opts->opt_q = false;
    }

    /* Check super-user privileges: -A is always set for super-user (UID == 0) */
    if (geteuid() == 0) {
        opts->opt_A = true;
    }

    /* Default blocksize is 512 bytes */
    opts->blocksize = 512;

    /* Check BLOCKSIZE environment variable */
    const char *bs_env = getenv("BLOCKSIZE");
    if (bs_env && *bs_env) {
        long bs = atol(bs_env);
        if (bs > 0) {
            opts->blocksize = bs;
        }
    }

    opts->exit_status = 0;
}

void options_print_usage(const char *progname) {
    fprintf(stderr, "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n",
            progname ? progname : "ls");
}

int options_parse(int argc, char **argv, ls_options_t *opts) {
    if (!opts) return -1;

    int ch;
    /*
     * Manual synopsis:
     * ls [-AacdFfhiklnqRrSstuw] [file ...]
     */
    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
            case 'A':
                opts->opt_A = true;
                break;
            case 'a':
                opts->opt_a = true;
                break;
            case 'c':
                /* -c and -u override each other */
                opts->opt_c = true;
                opts->opt_u = false;
                break;
            case 'd':
                /* -R and -d override each other */
                opts->opt_d = true;
                opts->opt_R = false;
                break;
            case 'F':
                opts->opt_F = true;
                break;
            case 'f':
                /* Output is not sorted; in BSD, -f also implies -a */
                opts->opt_f = true;
                opts->opt_a = true;
                break;
            case 'h':
                /* -k and -h override each other, rightmost wins */
                opts->opt_h = true;
                opts->opt_k = false;
                break;
            case 'i':
                opts->opt_i = true;
                break;
            case 'k':
                /* -k and -h override each other */
                opts->opt_k = true;
                opts->opt_h = false;
                opts->blocksize = 1024;
                break;
            case 'l':
                /* -l and -n override each other */
                opts->opt_l = true;
                opts->opt_n = false;
                break;
            case 'n':
                /* -l and -n override each other */
                opts->opt_n = true;
                opts->opt_l = false;
                break;
            case 'q':
                /* -w and -q override each other */
                opts->opt_q = true;
                opts->opt_w = false;
                break;
            case 'R':
                /* -R and -d override each other */
                opts->opt_R = true;
                opts->opt_d = false;
                break;
            case 'r':
                opts->opt_r = true;
                break;
            case 'S':
                opts->opt_S = true;
                opts->opt_t = false;
                break;
            case 's':
                opts->opt_s = true;
                break;
            case 't':
                opts->opt_t = true;
                opts->opt_S = false;
                break;
            case 'u':
                /* -c and -u override each other */
                opts->opt_u = true;
                opts->opt_c = false;
                break;
            case 'w':
                /* -w and -q override each other */
                opts->opt_w = true;
                opts->opt_q = false;
                break;
            case '?':
            default:
                options_print_usage(argv[0]);
                opts->exit_status = 1;
                return -1;
        }
    }

    return optind;
}
