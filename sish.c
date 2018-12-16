#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "extern.h"

#define MAX_ARGLEN 16

static void usage(void);

int last_status;

int
main(int argc, char **argv)
{
    int ch;
    struct sish_opt *opts;
    struct sigaction sa, intsa;
    struct sish_command *comm;
    
    setprogname(argv[0]);

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if ((opts = malloc(sizeof *opts)) == NULL) {
	fprintf(stderr, "malloc: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &sa, &intsa) == -1)
	err(1, "unable to handle SIGINT");

    while ((ch = getopt(argc, argv, "cx")) != -1) {
	switch (ch) {
	case 'c':
	    opts->command = 1;
	    break;
	case 'x':
	    opts->trace = 1;
	    break;
	default:
	    free(opts);
	    sigaction(SIGINT, &intsa, NULL);
	    usage();
	    /* NOT REACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc == 1) {
	opts->run = argv[0];
    }

    last_status = 0; /* default */

    for (;;) {
	printf("sish$ ");
	if ((comm = parse()) == NULL) {
	    printf("\n");
	    break;
	}
	if (sish_builtin(comm) == 0) {
	    if (opts->trace == 1)
		printf("+ %s\n", comm->command);
	    last_status = sish_execute(comm);
	}
	free_command(comm);
    }

    free(opts);
    sigaction(SIGINT, &intsa, NULL);
}

static void
usage(void)
{
    fprintf(stderr, "usage: %s [-x] [-c command]\n", getprogname());
    exit(EXIT_FAILURE);
}
