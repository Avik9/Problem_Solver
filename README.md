# Problem Solver
#### Avik Kadakia Eugene Stark

## Introduction

This program is an implementation of a "problem solver" program, called `polya`, which manages a
collection of "worker" processes that concurrently engage in solving
computationally intensive problems.

## Getting Started

You will need to install the `libgcrypt20-dev` crypto library in order to compile
the code.  Do this with the following command:

```
$ sudo apt install libgcrypt20-dev
```

Here is the structure of the base code:
<pre>
.
├── .gitlab-ci.yml
└── hw4
    ├── demo
    │   └── polya
    ├── hw4.sublime-project
    ├── include
    │   ├── debug.h
    │   └── polya.h
    ├── lib
    │   └── sf_event.o
    ├── Makefile
    ├── src
    │   ├── crypto_miner.c
    │   ├── main.c
    │   ├── master.c
    │   ├── problem.c
    │   ├── trivial.c
    │   ├── worker.c
    │   └── worker_main.c
    └── tests
        └── hw4_tests.c
</pre>

If you run `make`, the code should compile correctly, resulting in three
executables `bin/polya`, `bin/polya_worker`, and `bin/polya_test`.
The executable `bin/polya` is the main one, for the "master" process.
If you run this program, it doesn't do very much, because the main functions
are just stubs -- you have to write them!
The executable `bin/polya_worker` is the executable run by "worker" processes.
It is invoked by the master process -- it is not designed to be invoked from
the command line.
The executable `bin/polya_tests` runs some tests using Criterion, as usual.
However, the default tests supplied with the base code use the fully implemented
executable `demo/polya` for the master process instead of `bin/polya`,
which needs a lot of details filled in by you.  Using `demo/polya` will allow
you to get started more easily by first implementing and testing the code for
the worker processes and then progressing to the implementation of the code
for the master process, which is more complex.

### The `killall` Command

In the course of debugging this program, you will almost certainly end up in
situations where there are a number of "leftover" worker processes that have survived
beyond a particular test run of the program.  If you allow these processes to
accumulate, it can cause confusion, as well as consume resources on your computer.
The `ps(1)` (process status) command can be used to determine if there are any
such processes around; e.g.

```
$ ps alx | grep polya
```

If there are a lot of them, it can be tedious to kill them all using the `kill` command.
The `killall` command can be used to kill all processes running a program with a
particular name; e.g.

```
$ killall polya_worker
```

It might be necessary to use the additional option `-s KILL` to send these processes
a `SIGKILL`, which they cannot catch or ignore, as opposed to the `SIGTERM`, which is
sent by default.

## `Polya`: Functional Specification

### Overview

The `polya` program implements a facility for concurrently solving computationally
intensive "problems".  It is not particularly important to the core logic of `polya`
what the problems are that are being solved.  All that the core logic knows about
is that a "problem" consists of a *header*, which gives some basic information about
the problem, and some *data*, which is an arbitrary sequence of bytes that the core
logic does not otherwise interpret.  The header of a problem contains a *size* field,
which specifies the total length of the header plus data, a *type* field,
which specifies which one of a set of available *solvers* is to be used to solve the
problem, an *id* field, which contains a serial number for the problem, and some fields
that specify in which of a number of possible *variant forms* the problem is expressed.

After initialization is completed, the `polya` program in execution consists of a
a *master process* and one or more *worker processes*.  The master process is
responsible for obtaining problems to be solved from a *problem source* and
distributing those problems to the worker processes to be solved.
When a worker process receives a problem from the master process, it begins trying
to solve that problem.  It continues trying to solve the problem until one of the
following things happens: (1) A solution is found; (2) The solution procedure fails;
or (3) The master process notifies the worker to cancel the solution procedure.
In each case, the worker process sends back to the master process a *result*,
which indicates what happened and possibly contains a solution to the problem.
The structure of a result is similar to that of a problem and, just as for problems,
the core `polya` logic does not care about the detailed content of a result,
which is dependent on the problem type.  As far as the core logic is concerned,
a result consists of a *header* and some uninterpreted *data*.
The header contains a *size* field, which specifies the total length of the header
plus data, an *id* field, which gives the serial number of the problem to which the
result pertains, and a *failed* field, which indicates whether the solution procedure
failed or whether the result should be considered as a potential solution to the
original problem.

### Coordination Protocol

The master process coordinates with the worker processes using a protocol that involves
*pipes* and *signals*.  During initialization, each time the master process creates a
worker process, it creates two pipes for communicating with that worker process:
one pipe for sending problems from the master to the worker, and one pipe for sending
results from the worker to the master.
The pipes themselves are created using the `pipe(2)` system call before the worker
process is "forked" using the `fork(2)` system call.  Once having forked, the child
process (which will be the worker) redirects its standard input and standard
output to the pipes using the `dup2(2)` system call, and the worker program
itself is executed using one of the system calls in the the `exec(3)` family .
 The actual reading and writing of data on the pipes can be done either using the low-level
"Unix I/O" `read(2)` and `write(2)` system calls, or else (probably somewhat more conveniently)
using the standard I/O library, after using `fdopen(3)` to wrap the pipe file descriptors
in `FILE` objects.

  > **Important:**  You **must** create pipes using the `pipe()` function, you must
  > create the worker processes using `fork()` and you must execute the
  > worker program using some form of `exec` (such as `execl()`).  You **must not**
  > use the `system()` function, nor use any form of shell in order to do this things,
  > as the purpose of the assignment is to give you experience with direct use of the
  > system calls involved in doing these things.

When the master process wants to send a problem to a worker process, it first writes
the fixed-size problem header to the pipe, and then continues by writing the problem
data.  When a worker process wants to read a problem sent by the master, it first reads
the fixed-size problem header, and continues by reading the problem data, the number of
bytes of which can be calculated by subtracting the size of the header from the total
size given in the size field of the header.  The procedure by which a worker sends a result
to the master process is symmetric.

In order to know when to send a problem to a worker process and when to read a result
from a worker process, the master process uses *signals* to control the worker process
and to track its state.  From the point of view of the master process, at any given time,
a worker process is in one of the following states:

* `STARTED` - The worker process has just been started, and is performing initialization.

* `IDLE` - The worker process has stopped (as a result of the worker having received a
`SIGSTOP` signal), and the master process knows this (as a result of having received
a `SIGCHLD` signal and subsequently having used the `waitpid(2)` system call to query
the reason for the signal).  In this `IDLE` state, there is no result available for the
master to read on the pipe from that worker.

* `CONTINUED` - A stopped worker process has been signaled to continue (by having been sent
a `SIGCONT` signal), but the master has not yet observed (via `SIGCHLD`/`waitpid()`)
that the worker is once again executing.  When the worker process does continue to execute,
it will attempt to read a problem on the pipe from the master process.

* `RUNNING` - The master process has received a `SIGCHLD` from a worker process that was
previously in the `CONTINUED` state, and the master process has used the `waitpid(2)`
system call to confirm that the worker process is no longer stopped.  The worker process is
now working on solving a problem that the master has sent.

* `STOPPED` - The worker process has stopped working on trying to solve a problem that was
previously sent, and has written a result (which might or might not be marked "failed")
on the pipe to the master process.  The master has observed (via `SIGCHLD`/`waitpid()`)
that the worker has stopped and the master will read the result sent by the worker.
A worker process in the `RUNNING` state arranges to enter the `STOPPED` state by
using `kill(2)` to send itself a `SIGSTOP` signal.

* `EXITED` - The worker process has exited normally using the `exit(3)` function, and the
master process has observed this (via `SIGCHLD`/`waitpid()`).

* `ABORTED` - The worker process has terminated abnormally as a result of a signal,
or has terminated normally but with a nonzero exit status, and the master process has
observed this (via `SIGCHLD`/`waitpid()`).

Normally, a worker that is signaled to continue will read a problem sent by the master
process and will try to solve that problem until either the solution procedure succeeds
in finding a solution or it fails to find any solution.  However, it is also possible
for the master process to notify a worker process to *cancel* the solution procedure
that it is currently executing.  The master process does this by sending a `SIGHUP`
signal to the worker process.  When a worker process receives `SIGHUP`, if the current
solution attempt has not already succeeded or failed, then it is abandoned and a
result marked "failed" is sent to the master process before the worker stops
by sending itself a `SIGSTOP` signal.

Finally, when a worker receives a `SIGTERM` signal, it will abandon any current solution
attempt and use `exit(3)` to terminate normally with status `EXIT_SUCCESS`.

### Concurrent Solution

The underlying idea of `polya` is to exploit the possibility of concurrent execution
of worker processes (*e.g.* on multiple cores) to speed up the process of finding a
solution to a problem.  To support this, each problem to be solved with `polya` can be
created in one of several *variant forms*, where a solution to any one of the variant
forms constitutes a solution to the original problem.  Concurrency is achieved by
assigning a different variant form to each of a collection of concurrently executing
worker processes.  To the extent that the work involved in solving one variant form
does not overlap with that involved in solving another, the concurrency will speed up
the solution procedure.

As a concrete example, the main example problem type that has been included with the `polya`
basecode is the "crypto miner" problem type.  This problem type is modeled after the
computations performed by "miners" in a cryptocurrency system such as Bitcoin.
In such a system, the "miners" generate "blocks", which contain lists of transactions,
and for each block it creates a miner attempts to certify to the other miners in the
system that it has put significant computational effort into creating the block.
The other miners will only accept blocks with such a certification -- this is the concept
of *proof of work* that underlies the security of Bitcoin and other similar cryptocurrency
systems.  The proof of work for a block is achieved by finding a solution to a computationally
intensive problem associated with the block and including that solution with the block.

In Bitcoin, the type of problem that is solved is to find a sequence of bytes,
called a "nonce", with the property that when the nonce is appended to the block and the
result is hashed using a cryptographic hash function, then the resulting hash
(which is just a sequence of bits) has a specified number of leading zero bits.
The solution to the problem is the nonce, and although anyone can readily verify the
solution by concatenating the nonce with the block data and computing its hash,
it is in general not possible to efficiently find a nonce that solves a given block -- the best
that can be done is just to try one nonce after another until a solution is found.
This is what Bitcoin miners spend their time (and electrical power) doing.
The number of leading zero bits required for a solution allows the "difficulty" of the
problem to be solved to be adjusted: requiring fewer zero bits makes it easier to solve
the problem and requiring more zero bits makes it harder.
The nature of this kind of problem makes concurrency useful in solving it:
we can just partition the space of all possible nonce values into disjoint subsets,
and assign a separate concurrent process to work on each subset.

For the "crypto miner" solver included with the basecode, "blocks" are just filled with
random data (because we are not really trying to process transactions) and the SHA256
hash algorithm is used to hash the block contents, followed by a 8-byte nonce
(the Bitcoin system actually uses something like three rounds of the older SHA1 hash
algorithm rather than one round of SHA256).
The problem "difficulty" ranges from 20 to 25 bits of leading zeros, which should produce
a reasonable test range of solution times for our purposes on most computers.
The Bitcoin system dynamically adjusts the difficulty to adapt to the total number of
miners and the power of their computers so that, on average, about one block will
be solved every ten minutes in the entire system.

### Problem Source

In `polya`, problems to be solved are obtained from a "problem source" module,
which maintains a notion of the "current problem", creates variant forms of the
current problem, and keeps track of when the current problem has been solved
and it is time to create a new problem.  When a new problem is created, one of
the enabled problem types is chosen randomly and its "constructor" method is invoked
to produce a new problem.  When a worker process produces a result, the result is
posted to the problem source module, which checks the solution and clears the
current problem if a solution has indeed been found.

The problem module has been introduced basically to encapsulate the creation and
management of problems to be solved (which is not our primary interest here)
from the mechanism by which the problems are solved by concurrent worker processes
(which is our main interest).  The problems that we create are just built from
random data, though it would also be possible to have some of the "constructor"
methods read real problem data from a file.  Your master process will have to call
methods of the problem source to obtain problems and post solutions, but otherwise
you should not worry about how this all works, or even how meaningful it might all be.

### Functions You Must Implement

As part of your implementation of `polya`, you are to provide implementations for
the following two functions:

* `int master(int workers);` -- Main function for the master process.

	When invoked, this function performs required initialization, creates
	a number of worker processes (and associated pipes) as specified by the
	`workers` parameter, and then enters a main loop that repeatedly assigns problems
	to idle workers and posts results received from workers, until finally all of
	the worker processes have become idle and a `NULL` return from the `get_problem_variant`
	function (see the "Problem Source" section below) indicates that there are no further
	problems to be solved.  Once this has occurred, a `SIGTERM` signal is sent to
	each of the worker processes, which catch this signal and then exit normally.
	When all of the worker processes have terminated, the master process itself also
	terminates.  The exit status of the master process is `EXIT_SUCCESS` if all worker
    processes have terminated normally with status `EXIT_SUCCESS`;
    otherwise the exit status of the master process is `EXIT_FAILURE`.

* `int worker(void);` -- Main function for a worker process.

	When invoked, this function performs required initialization, then stops
	by sending itself a `SIGSTOP` signal.  Upon continuing (when the master process
	sends it a `SIGCONT` signal), it reads a problem sent by the master and attempts
	to solve the problem.  It continues the solution attempt until it either succeeds
	in finding a solution, fails to find a solution, or is notified (by the master
	process sending it a `SIGHUP` signal) to cancel the current solution attempt.
	In each case, it sends a result to the master process before once again stopping
	by sending itself a `SIGSTOP` signal.  This procedure, of reading a problem, solving,
	writing a result, and stopping continues until a `SIGTERM` signal is received
	from the master process, at which point the worker process terminates by calling
	`exit(3)` with exit status `EXIT_SUCCESS`.

### Event Functions You Must Call

In order to make it possible for us to trace the actions of your program,
we have provided several functions that you **must** call in particular situations
from within your master process (your worker processes should not call any of
these functions).  The provided functions are:

* `void sf_start(void);`

	To be called when the master process has just begun to execute.

* `void sf_end(void);`

	To be called when the master process is about to terminate.

* `void sf_change_state(int pid, int oldstate, int newstate);`

    This function is to be called when the master process causes or observes a
	worker process to change state.  The `pid` parameter is the process ID
	of the worker process, the `oldstate` parameter is the state the the worker
	process was in before the change, and the `newstate` parameter is the state
	that the worker process is in after the change.

* `void sf_send_problem(int pid, struct problem *prob);`

	This function is to be called when the master process is about to send a
	problem over the pipe to a worker process.  The `pid` parameter is the process ID
	of the worker process, the `prob` parameter is the problem structure that
	is being sent.

* `void sf_recv_result(int pid, struct result *result);`

	This function is to be called when the master process has received a result over
	the pipe from a worker process.  The `pid` parameter is the process ID of the
	worker process, and the `result` parameter is the result structure that was
	received.

* `void sf_cancel(int pid);`

	This function is to be called when the master process is about to notify a worker
    process that the current solution attempt is to be canceled.  The `pid`
	parameter is the process ID of the worker process that is being notified.

The above functions are provided to you as an object file that will be linked with
your master program.  As implemented in the basecode, these functions will announce
on `stderr` that they have been called, so that you can verify that you are
calling them properly.  Should you desire not to see this printout, you can
set the global variable `sf_suppress_chatter` to a nonzero value.
When we grade your program, we will replace the basecode implementations of
these functions with different versions that will allow us to track automatically
whether events are actually occurring in the order they are supposed to be.

### Command-Line Arguments

The master program (exectuable `bin/polya`) has the following usage synopsis:

```
$ polya [-w num_workers] [-p num_probs] [-t prob_type]
```

where:
 * `num_workers` is the number of worker processes to use (min 1, max 32, default 1)
 * `num_probs` is the total number of problems to be solved (default 0)
 * `prob_type` is an integer specifying a problem type whose solver
    is to be enabled (min 0, max 31).  The -t flag may be repeated to enable
    multiple problem types.

These options have already been implemented in the `main.c` that is provided
in the base code.  You should not have to change this.

The worker program, whose executable is `bin/polya_worker` and whose `main()`
function is in the file `worker_main.c()`, ignores its command-line arguments.
Instead, the worker will receive problems from the master process by reading
its standard input, and it should send results back to the master process by
writing to its standard output.

### Signal Handling

The `polya` master process must install a `SIGCHLD` handler so that it can be
notified when worker processes stop and continue.  It might also be useful for
the master process to catch or ignore `SIGPIPE`, so that it is not inadvertently
terminated by the premature exit of a worker process.

A `polya_worker` worker process must install handlers for `SIGHUP` and `SIGTERM`.
As already discussed, the `SIGHUP` signal is sent by the master process to notify
a worker to cancel its current solution attempt.
A `SIGTERM` signal is sent by the master to cause graceful termination of a
worker process.  Note that if you do not catch `SIGTERM`, receipt of a `SIGTERM`
signal will cause the termination of a worker process, but the exit status
of that process as returned from `waitpid(2)` will show the process as having
terminated abnormally by a signal, rather than normally via `exit()`.
Termination by a signal does not count as "graceful".

If you want your programs to work reliably, you must only use async-signal-safe
functions in your signal handlers.
You should make every effort not to do anything "complicated" in a signal handler;
rather the handlers should just set flags to communicate back to the main program
what has occurred and the main program should check these flags and perform whatever
actions are necessary.
Variables used for communication between the handler and the main program should
generally be declared `volatile` so that a handler will always see values that
are up-to-date (otherwise, the C compiler might generate code to cache updated
values in a register for an indefinite period, which could make it look to a
handler like the value of a variable has not been changed when in fact it has).
Ideally, such variables should be of type `sig_atomic_t`, which means that they
are just integer flags that are read and written by single instructions.
Note that operations, such as the auto-increment `x++`, and certainly more complex
operations such as structure assignments, will generally not be performed as a single
instruction.  This means that it would be possible for a signal handler to be
invoked "in the middle" of such an operation, which could lead to "flaky"
behavior if it were to occur.

  > :nerd: Note that you will need to use `sigprocmask()` to block signals at
  > appropriate times, to avoid races between the handler and the main program,
  > the occurrence of which can also result in indeterminate behavior.
  > In general, signals must be blocked any time the main program is actively
  > involved in manipulating variables that are shared with a signal handler.

Note that standard I/O functions such as `fprintf()` are not async-signal-safe,
and thus cannnot reliably be used in signal handlers.  For example, suppose the
main program is in the middle of doing an `fprintf()` when a signal handler is invoked
asynchronously and the handler itself tries to do `fprintf()`.  The two invocations
of `fprintf()` share state (not just the `FILE` objects that are being printed to,
but also static variables used by functions that do output formatting).
The `fprintf()` in the handler can either see an inconsistent state left by the
interrupted `fprintf()` of the main program, or it can make changes to this state that
are then visible upon return to the main program.  Although it can be quite useful
to put debugging printout in a signal handler, you should be aware that you can
(and quite likely will) see anomalous behavior resulting from this, especially
as the invocations of the handlers become more frequent.  Definitely be sure to
remove or disable this debugging printout in any "production" version of your
program, or you risk unreliability.

### Reading Input and Writing Output

In `polya` the master and worker processes communicate with each other by sending
"problems" and "results" over pipes.  During the creation of each worker process,
the master process creates the associated pipes and performs I/O redirection so
that the worker always reads problems from its standard input (file descriptor 0)
and writes results to its standard output (file descriptor 1).
As the master process has to communicate with multiple workers, however, it does
not redirect its own standard input and output but rather uses the file descriptors
originally returned by the `pipe()` function.  It will need to use suitable
data structure (such as arrays) to keep track of all these file descriptors.

In `polya`, a "problem" sent over a pipe consists of a fixed-size header, followed
by an arbitrary number of data bytes.  A problem header is represented by a structure
of type `struct problem` (see `polya.h` for details).  The header is written to
the pipe as a sequence of `sizeof(struct problem)` bytes.  This can be done either
using the low-level `write(2)` call to write the header in a single operation,
or it can be done by using a pointer to go in a byte-by-byte fashion through the
`struct problem` structure and using the standard I/O function `fputc()` to write
each byte to the output stream.  Once the header has been written, then the
data bytes are written, so that the total of the number of data bytes and
`sizeof(struct problem)` adds up to the value in the `size` field of the header.
If the standard I/O library is used, then `fflush()` should be called once
the entire problem has been written, to be sure that all the data is immediately
pushed into the pipe.

Reading a problem from a pipe involves the reverse procedure.  First we read
`sizeof(struct problem)` bytes from the input and store it into a `struct problem`
variable.  Then, we examine the `size` field of this structure and read an
additional `size - sizeof(struct problem)` bytes.  Since once we have read the
header we know how many bytes of additional data we have to read, we can use
`malloc()` to allocate storage for this data.  Although once again the reading
can be done using either the low-level `read(2)` call or a higher-level
standard I/O library call such as `fgetc()`, in view of the buffering that takes
place in the standard I/O library you really have to pick one way or the other
of doing things and not try to "mix-and-match".  Probably the standard I/O
library is more convenient.

Note that, when reading or writing a `struct problem` header, we are treating it
simply as a sequence of `sizeof(struct problem)` bytes to be read or written.
The bytes are written out in the order in which they occur in memory
(*i.e.* in increasing address order), and they are read back into memory in the
same order.  As long as the reading and writing is being done on the same system,
with the same "endianness", the values of multibyte fields (such as the `size`
field) in the structure will be preserved.  However, note that the simple technique
that we are using here will not preserve the values of such fields, in general,
if the reading and writing is performed on systems with different "endiannesses",
as they might be if the stream on which we are writing is a network connection to
a computer of a different type, or if we are writing the data to a disk file on one
system and reading it back into memory on a different system.
We will not worry about this issue for the current assignment.

## Provided Components

### The `polya.h` Header File

The `polya.h` header file that we have provided defines various constants and
data structures shared between different parts of the `polya` system,
gives function prototypes for the functions that you are to use and those that
you are to implement, and contains specifications for these functions that you
should read carefully in conjunction with the information provided in the
present assignment document.

  > :scream: **Do not make any changes to `polya.h`.  It will be replaced
  > during grading, and if you change it, you will get a zero!**

### The `main.c` Source File

The `main.c` file contains the `main()` function for the `polya` master program,
whose executable is left by `make` in `bin/polya`.  It already contains code
to process command-line arguments.
**It will be replaced during grading -- don't change it.**

### The `worker_main.c` Source File

The `main.c` file contains the `main()` function for the `polya` worker program,
whose executable is left by `make` in `bin/polya_worker`.  **It will be replaced
during grading -- don't change it.**

### The `problem.c` Source File

The `problem.c` file contains the functions that implement the "problem source"
module.  The `init_problems` function is called from `main.c` and `worker_main.c`.
You will need to call `get_problem_variant` and `post_result` from your master
process.  **This file will be replaced during grading -- don't change it.**

### The `trivial.c` Source File

The `trivial.c` file implements the solver for the "trivial" problem type.
You have been provided with the source code because there is no reason to conceal
it and you might learn something from studying it.  **This file will be replaced
during grading -- don't change it.**

### The `crypto_miner.c` Source File

The `crypto miner.c` file implements the solver for the "crypto miner" problem type.
You have been provided with the source code because there is no reason to conceal
it and you might learn something from studying it.  **This file will be replaced
during grading -- don't change it.**

### The `master.c` Source File

The `master.c` file contains a stub for the `master()` function that you are to
implement.  Put your implementation of this function in this file.
You may also create helper files, if you like.

### The `worker.c` Source File

The `worker.c` file contains a stub for the `worker()` function that you are to
implement.  Put your implementation of this function in this file.
You may also create helper files, if you like.

### The `lib/sf_event.o` Object File

The `lib/sf_event.o` object file contains implementations of the functions that
you are to call upon occurrence of various events (see *Event Functions* above).
This file will be linked with your master program.  As indicated above, the basecode
implementation simply prints a message on `stderr` each time one of the functions
is called.  During grading, we will replace this implementation with a different
one, which will help us to automatically track the actions of your program.

### The `demo/polya` Executable

To help answer questions about what you are expected to implement and to make
it easier for you to get something working that you can test, I have included a
demonstration version of the master program as `demo/polya`.
You invoke this program by typing `demo/polya`, giving it the same command-line
arguments as what your master program will take.
The `demo/polya` program expects to execute `bin/polya_worker` as the worker
program.  It prints out fairly extensive debugging trace output to help you
understand what is going on.

Given the presence of `demo/polya`, your best strategy for attacking this assignment
is probably to first work on implementing your `worker()` function, using the
`demo/polya` program as a test driver for it.  You will be able to fill in functionality
of the worker process a bit at a time and the debugging trace produced by `demo/polya`
will help you through the rough spots.  Try it first with just one worker process,
until you are pretty sure that things are working well that way.  Then proceed to multiple
worker processes, which will produce more complex behaviors.

Once you have completed your `worker()` function, you can then work on implementing
your `master()` function; ultimately using your own `bin/polya` as the driver program
rather than `demo/polya`.

### Sample Traces

To help you out even more, I have provided below debugging traces of the execution of
the complete `polya` system when tests are run with the following command:

```
bin/polya_tests --verbose -j1 --filter demo_master_suite/miner_test_three_workers
```

Debugging trace output is provided from both the worker and master processes,
but because I don't want to unduly bias you as to how to organize your code,
I have used a version of the debugging macros that omits the source file name,
line number, and function name.
