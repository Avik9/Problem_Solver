#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <stdio.h>

#include "debug.h"
#include "polya.h"

int checkAllWorkers(int status);
int getPosition(pid_t pid, pid_t workers[], int num_workers);
void handle_sigchld(int sig);
void send_sighup(int problem_id, int pid);
void change_status(int worker_position, int prev_stat, int new_stat);

volatile int worker_status[MAX_WORKERS];
pid_t worker_array[MAX_WORKERS];
int current_problem_for_worker[MAX_WORKERS];
int num_workers;
int num_problems;

/*
 * master
 * (See polya.h for specification.)
 */
int master(int workers)
{
    sf_start();
    signal(SIGCHLD, handle_sigchld);

    int i;
    FILE *fd_array_master_stdin[workers];
    FILE *fd_array_master_stdout[workers];
    int fd_array_worker_to_master[2];
    int fd_array_master_to_worker[2];
    char *args[] = {"./bin/polya_worker", NULL};
    num_workers = workers;
    sigset_t empty_mask, mask_parent;

    sigemptyset(&mask_parent);
    sigaddset(&mask_parent, SIGCHLD);

    sigprocmask(SIG_BLOCK, &mask_parent, NULL);

    for (i = 0; i < workers; i++)
    {
        pipe(fd_array_worker_to_master);
        pipe(fd_array_master_to_worker);

        fd_array_master_stdin[i] = fdopen(fd_array_worker_to_master[0], "r");
        fd_array_master_stdout[i] = fdopen(fd_array_master_to_worker[1], "w");

        if ((worker_array[i] = fork()) > 0) // Parent pocessing
        {
            close(fd_array_worker_to_master[1]);
            close(fd_array_master_to_worker[0]);

            change_status(i, 0, WORKER_STARTED);
        }
        else if (worker_array[i] == 0) // Child Process
        {
            close(fd_array_master_to_worker[1]);
            close(fd_array_worker_to_master[0]);

            dup2(fd_array_worker_to_master[1], STDOUT_FILENO);
            dup2(fd_array_master_to_worker[0], STDIN_FILENO);

            if ((execv(args[0], args)) == -1)
            {
                debug("WORKER PID %d: ABORTING BECAUSE COULD NOT CALL execv", getpid());
                change_status(i, worker_status[i], WORKER_ABORTED);

                _exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "WORKER %d: ABORTING BECAUSE COULD NOT FORK\n", i);
            change_status(i, worker_status[i], WORKER_ABORTED);
        }
    }

    sigprocmask(SIG_UNBLOCK, &mask_parent, NULL);

    sigemptyset(&empty_mask);
    sigfillset(&empty_mask);
    sigdelset(&empty_mask, SIGCHLD);
    sigsuspend(&empty_mask);

    struct problem *curr_problem;
    struct result *result_read;

    for (i = 0; i <= workers; i++)
    {
        if (i == workers)
            i = 0;

        // Write the problem
        if (worker_status[i] == WORKER_IDLE)
        {
            curr_problem = get_problem_variant(workers, i);
            if ((curr_problem == NULL))
                break;

            sigprocmask(SIG_BLOCK, &mask_parent, NULL);

            fwrite(curr_problem, sizeof(struct problem), 1, fd_array_master_stdout[i]);

            if (curr_problem->size > sizeof(struct problem))
                fwrite(curr_problem->data, curr_problem->size - sizeof(struct problem), 1, fd_array_master_stdout[i]);

            fflush(fd_array_master_stdout[i]);
            sf_send_problem(worker_array[i], curr_problem);

            current_problem_for_worker[i] = curr_problem->id;

            change_status(i, worker_status[i], WORKER_CONTINUED);

            sigprocmask(SIG_UNBLOCK, &mask_parent, NULL);

            kill(worker_array[i], SIGCONT);
        }

        // Read the result
        if (worker_status[i] == WORKER_STOPPED)
        {
            sigprocmask(SIG_BLOCK, &mask_parent, NULL);

            result_read = (struct result *)(malloc(sizeof(struct result)));

            if (result_read == NULL)
            {
                fprintf(stderr, "WORKER PID %d: EXITING BECAUSE RESULT MALLOC == NULL\n", worker_array[i]);
                change_status(i, worker_status[i], WORKER_ABORTED);

                continue;
            }
            fread(result_read, sizeof(struct result), 1, fd_array_master_stdin[i]);

            if (result_read->size > sizeof(struct result))
            {
                result_read = (struct result *)realloc(result_read, result_read->size);
                if (result_read == NULL)
                {
                    fprintf(stderr, "WORKER PID %d: EXITING BECAUSE RESULT REALLOC == NULL\n", worker_array[i]);
                    change_status(i, worker_status[i], WORKER_ABORTED);

                    continue;
                }

                fread(result_read->data, result_read->size - sizeof(struct result), 1, fd_array_master_stdin[i]);
                fflush(fd_array_master_stdin[i]);
            }

            sigprocmask(SIG_UNBLOCK, &mask_parent, NULL);

            if (!result_read->failed)
            {
                sf_recv_result(worker_array[i], result_read);
                post_result(result_read, curr_problem);
                send_sighup(curr_problem->id, worker_array[i]);
            }
            else
                change_status(i, worker_status[i], WORKER_IDLE);
        }
    }

    for (int j = 0; j < workers; j++)
    {
        fclose(fd_array_master_stdin[i]);
        fclose(fd_array_master_stdout[i]);

        if (worker_status[i] != WORKER_ABORTED)
        {
            kill(worker_array[i], SIGCONT);
            change_status(i, worker_status[i], WORKER_RUNNING);

            kill(worker_array[i], SIGTERM);
            change_status(i, WORKER_RUNNING, WORKER_EXITED);
        }
    }

    int return_stat = EXIT_SUCCESS;

    if (!checkAllWorkers(WORKER_EXITED))
        return_stat = EXIT_FAILURE;

    sf_end();
    return return_stat;
}

void handle_sigchld(int sig)
{
    int child_status;
    sigset_t mask_parent;

    sigemptyset(&mask_parent);
    sigaddset(&mask_parent, SIGCHLD);

    pid_t wpid;
    while ((wpid = waitpid(-1, &child_status, WNOHANG | WUNTRACED | WCONTINUED | WSTOPPED)) > 0)
    {
        int i = getPosition(wpid, worker_array, num_workers);

        if (worker_status[i] == WORKER_STARTED)
            change_status(i, worker_status[i], WORKER_IDLE);

        else if (worker_status[i] == WORKER_CONTINUED)
            change_status(i, worker_status[i], WORKER_RUNNING);

        else if (worker_status[i] == WORKER_RUNNING && WIFEXITED(child_status))
            change_status(i, worker_status[i], WORKER_EXITED);

        else if (worker_status[i] == WORKER_RUNNING)
            change_status(i, worker_status[i], WORKER_STOPPED);

        else if (WIFSIGNALED(child_status) != 0)
            change_status(i, worker_status[i], WORKER_ABORTED);
    }
}

int getPosition(pid_t pid, pid_t workers[], int num_workers)
{
    for (int j = 0; j < num_workers; j++)
    {
        if (workers[j] == pid)
            return j;
    }
    return -1;
}

int checkAllWorkers(int status)
{
    for (int j = 0; j < num_workers; j++)
    {
        if (worker_status[j] != status)
            return 0;
    }
    return 1;
}

void send_sighup(int problem_id, int wpid)
{
    for (int i = 0; i < num_workers; i++)
    {
        if (current_problem_for_worker[i] == problem_id)
        {
            if (wpid != worker_array[i])
            {
                sf_cancel(worker_array[i]);
                kill(SIGHUP, worker_array[i]);
            }
            else
                change_status(i, worker_status[i], WORKER_IDLE);
        }
    }
}

void change_status(int worker_position, int prev_stat, int new_stat)
{
    sf_change_state(worker_array[worker_position], prev_stat, new_stat);
    worker_status[worker_position] = new_stat;
}