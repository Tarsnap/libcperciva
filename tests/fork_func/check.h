#ifndef CHECK_H_
#define CHECK_H_

/**
 * check_exit(void):
 * Check that fork_func() works with exit codes.
 */
int check_exit(void);

/**
 * check_exec(void):
 * Check that fork_func() works with an exec* function.
 */
int check_exec(void);

/**
 * check_order(void):
 * Check that fork_func() doesn't impose any order on multiple functions.
 */
int check_order(void);

/**
 * check_perftest(child_ms, num_reps):
 * Check the amount of delay added by func_fork() and func_fork_wait().
 */
int check_perftest(int, int);

#endif /* !CHECK_H_ */
