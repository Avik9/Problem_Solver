#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>

#include "debug.h"
#include "polya.h"

volatile sig_atomic_t cancel_solution = 0;
struct problem *problem_read = NULL;

void handle_sigterm(int sig)
{
    _exit(EXIT_SUCCESS);
}

void handle_sighup(int sig)
{
    cancel_solution = 1;
}

/*
 * worker
 * (See polya.h for specification.)
 */
int worker(void)
{
    sigset_t mask_child;

    sigemptyset(&mask_child);
    sigaddset(&mask_child, SIGHUP);

    signal(SIGHUP, handle_sighup);
    signal(SIGTERM, handle_sigterm);

    raise(SIGSTOP);
    sleep(1);

    while (1)
    {
        sigprocmask(SIG_BLOCK, &mask_child, NULL);

        struct problem *problem_header = (struct problem *)(malloc(sizeof(struct problem)));
        if (problem_header == NULL)
        {
            fprintf(stderr, "WORKER: EXITING BECAUSE MALLOC == NULL\n");
            _exit(EXIT_FAILURE);
        }

        read(STDIN_FILENO, problem_header, sizeof(struct problem));

        if (problem_header->size > sizeof(struct problem))
        {
            problem_read = (struct problem *)realloc(problem_header, problem_header->size);
            if (problem_header == NULL)
            {
                fprintf(stderr, "WORKER: EXITING BECAUSE REALLOC == NULL\n");
                _exit(EXIT_FAILURE);
            }

            read(STDIN_FILENO, problem_read->data, problem_read->size - sizeof(struct problem));
        }
        else
            problem_read = problem_header;

        fflush(stdin);

        sigprocmask(SIG_UNBLOCK, &mask_child, NULL);

        struct result *result_returned = solvers[problem_read->type].solve(problem_read, &cancel_solution);

        sigprocmask(SIG_BLOCK, &mask_child, NULL);

        if (cancel_solution)
        {
            struct result temp = {sizeof(struct result), problem_read->id, 1, "", ""};
            result_returned = &temp;
        }

        write(STDOUT_FILENO, result_returned, sizeof(struct result));

        if (result_returned->size > sizeof(struct result))
            write(STDOUT_FILENO, result_returned->data, result_returned->size - sizeof(struct result));

        fflush(stdout);

        sigprocmask(SIG_UNBLOCK, &mask_child, NULL);

        raise(SIGSTOP);
        cancel_solution = 0;
    }

    return EXIT_SUCCESS;
}
