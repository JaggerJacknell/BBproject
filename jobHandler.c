
void handleUserCommand()
{
        if ((checkBuiltInCommands(commandArgv[0])) == 0) {
                launchJob(commandArgv, "STANDARD", 0, FOREGROUND);
        }
}

/**
 * built-in commands: exit, in, out, bg, fg, jobs, kill
 * returns 	1 if a built-in command is executed,
 * 			0 otherwise
 */
int checkBuiltInCommands()
{
        if (strcmp("exit", commandArgv[0]) == 0) {
                exit(EXIT_SUCCESS);
        }
        if (strcmp("cd", commandArgv[0]) == 0) {
                changeDirectory();
                return 1;
        }
        if (strcmp("in", commandArgv[0]) == 0) {
                launchJob(commandArgv + 2, *(commandArgv + 1), STDIN, FOREGROUND);
                return 1;
        }
        if (strcmp("out", commandArgv[0]) == 0) {
                launchJob(commandArgv + 2, *(commandArgv + 1), STDOUT, FOREGROUND);
                return 1;
        }
        if (strcmp("bg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                if (strcmp("in", commandArgv[1]) == 0)
                        launchJob(commandArgv + 3, *(commandArgv + 2), STDIN, BACKGROUND);
                else if (strcmp("out", commandArgv[1]) == 0)
                        launchJob(commandArgv + 3, *(commandArgv + 2), STDOUT, BACKGROUND);
                else
                        launchJob(commandArgv + 1, "STANDARD", 0, BACKGROUND);
                return 1;
        }
        if (strcmp("fg", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                int jobId = (int) atoi(commandArgv[1]);
                t_job* job = getJob(jobId, BY_JOB_ID);
                if (job == NULL)
                        return 0;
                if (job->status == SUSPENDED || job->status == WAITING_INPUT)
                        putJobForeground(job, TRUE);
                else                                                                                                // status = BACKGROUND
                        putJobForeground(job, FALSE);
                return 1;
        }
        if (strcmp("jobs", commandArgv[0]) == 0) {
                printJobs();
                return 1;
        }
        if (strcmp("kill", commandArgv[0]) == 0) {
                if (commandArgv[1] == NULL)
                        return 0;
                killJob(atoi(commandArgv[1]));
                return 1;
        }
        return 0;
}

/**
 * executes a program redirecting STDIN or STDOUT if newDescriptor != STANDARD
 */
void executeCommand(char *command[], char *file, int newDescriptor,
                    int executionMode)
{
        int commandDescriptor;
        /**
         *  Set the STDIN/STDOUT channels of the new process.
         */
        if (newDescriptor == STDIN) {
                commandDescriptor = open(file, O_RDONLY, 0600);                                        // open file for read only (it's STDIN)
                dup2(commandDescriptor, STDIN_FILENO);
                close(commandDescriptor);
        }
        if (newDescriptor == STDOUT) {
                commandDescriptor = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600); // open (create) the file truncating it at 0, for write only
                dup2(commandDescriptor, STDOUT_FILENO);
                close(commandDescriptor);
        }
        if (execvp(*command, command) == -1)
                perror("BD-shell(execvp)");
}

/**
 * forks a process and launches a program as child
 */
void launchJob(char *command[], char *file, int newDescriptor,
               int executionMode)
{
        pid_t pid;
        pid = fork();
        switch (pid) {
        case -1:
                perror("BD-shell(fork)");
                exit(EXIT_FAILURE);
                break;
        case 0:
                /**
                 *  we set the handling for job control signals back to the default.
                 */
                signal(SIGINT, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGCHLD, &signalHandler_child);
                signal(SIGTTIN, SIG_DFL);
                usleep(20000);                                                             // fixes a synchronization bug. Needed for short commands like ls
                setpgrp();                                                                                     // make the child as new process group leader
                if (executionMode == FOREGROUND)
                        tcsetpgrp(BDSH_TERMINAL, getpid());                                           // if we want the process to be in foreground
                if (executionMode == BACKGROUND)
                        printf("[%d] %d\n", ++numActiveJobs, (int) getpid());              // inform the user about the new job in bg

                executeCommand(command, file, newDescriptor, executionMode);

                exit(EXIT_SUCCESS);
                break;
        default:
                setpgid(pid, pid);                                                                        // we also make the child a new process group leader from here
                // to avoid race conditions
                jobsList = insertJob(pid, pid, *(command), file, (int) executionMode); // insert the job in the list

                t_job* job = getJob(pid, BY_PROCESS_ID);                             // and get it as job object

                if (executionMode == FOREGROUND) {
                        putJobForeground(job, FALSE);                                              // put the job in foreground (if desired)
                }
                if (executionMode == BACKGROUND)
                        putJobBackground(job, FALSE);                                             // put the job in background (if desired)
                break;
        }
}

/**
 * puts a job in foreground. If continueJob = TRUE, sends the process group
 * a SIGCONT signal to wake it up. After the job is waited successfully, it
 * restores the control of the terminal to the shell
 */
void putJobForeground(t_job* job, int continueJob)
{
        job->status = FOREGROUND;                                                   // set its status in the list as FOREGROUND
        tcsetpgrp(BDSH_TERMINAL, job->pgid);                                 // give it the control of the terminal
        if (continueJob) {                                                                        // continue the job (if desired)
                if (kill(-job->pgid, SIGCONT) < 0)                                           // by sending it a SIGCONT signal
                        perror("kill (SIGCONT)");
        }

        waitJob(job);                                                                                 // wait for the job
        tcsetpgrp(BDSH_TERMINAL, BDSH_PGID);                              // give the shell control of the terminal
}

/**
 * puts a job in background, and sends the job a continue signal, if continueJob = TRUE
 * puts the shell in foreground
 */
void putJobBackground(t_job* job, int continueJob)
{
        if (job == NULL)
                return;

        if (continueJob && job->status != WAITING_INPUT)
                job->status = WAITING_INPUT;		// fixes another synchronization bug: if the child process launches
		// a SIGCHLD and is set to WAITING_INPUT before this point has been
        // reached, then it would be set to BACKGROUND again

        if (continueJob)                        // if desired, continue the job
                if (kill(-job->pgid, SIGCONT) < 0)
                        perror("kill (SIGCONT)");

        tcsetpgrp(BDSH_TERMINAL, BDSH_PGID);                             // paranoia: give the shell control of terminal
}

/**
 * waits for a job, blocking unless it has been suspended.
 * Deletes the job after it has been executed
 */
void waitJob(t_job* job)
{
        int terminationStatus;
        while (waitpid(job->pid, &terminationStatus, WNOHANG) == 0) {      // while there are child to be waited
                if (job->status == SUSPENDED)                              // exit if the job has been set to be stopped
                        return;
        }
        jobsList = delJob(job);                                            // delete the job
}

/**
 * kills a Job given its number
 */
void killJob(int jobId)
{
        t_job *job = getJob(jobId, BY_JOB_ID);                                   // get the job from the list
        kill(job->pid, SIGKILL);                                                               // send the job a SIGKILL signal
}


t_job* insertJob(pid_t pid, pid_t pgid, char* name, char* descriptor,
                 int status)
{
        usleep(10000);
        t_job *newJob = malloc(sizeof(t_job));

        newJob->name = (char*) malloc(sizeof(name));
        newJob->name = strcpy(newJob->name, name);
        newJob->pid = pid;
        newJob->pgid = pgid;
        newJob->status = status;
        newJob->descriptor = (char*) malloc(sizeof(descriptor));
        newJob->descriptor = strcpy(newJob->descriptor, descriptor);
        newJob->next = NULL;

        if (jobsList == NULL) {
                numActiveJobs++;
                newJob->id = numActiveJobs;
                return newJob;
        } else {
                t_job *auxNode = jobsList;
                while (auxNode->next != NULL) {
                        auxNode = auxNode->next;
                }
                newJob->id = auxNode->id + 1;
                auxNode->next = newJob;
                numActiveJobs++;
                return jobsList;
        }
}

/**
 * modifies the status of a job
 */
int changeJobStatus(int pid, int status)
{
        usleep(10000);
        t_job *job = jobsList;
        if (job == NULL) {
                return 0;
        } else {
                int counter = 0;
                while (job != NULL) {
                        if (job->pid == pid) {
                                job->status = status;
                                return TRUE;
                        }
                        counter++;
                        job = job->next;
                }
                return FALSE;
        }
}

/**
 *	deletes a no more active process from the linked list
 */
t_job* delJob(t_job* job)
{
        usleep(10000);
        if (jobsList == NULL)
                return NULL;
        t_job* currentJob;
        t_job* beforeCurrentJob;

        currentJob = jobsList->next;
        beforeCurrentJob = jobsList;

        if (beforeCurrentJob->pid == job->pid) {

                beforeCurrentJob = beforeCurrentJob->next;
                numActiveJobs--;
                return currentJob;
        }

        while (currentJob != NULL) {
                if (currentJob->pid == job->pid) {
                        numActiveJobs--;
                        beforeCurrentJob->next = currentJob->next;
                }
                beforeCurrentJob = currentJob;
                currentJob = currentJob->next;
        }
        return jobsList;
}

/**
 * searches a job in the active jobs list, by pid, job id, job status
 */
t_job* getJob(int searchValue, int searchParameter)
{
        usleep(10000);
        t_job* job = jobsList;
        switch (searchParameter) {
        case BY_PROCESS_ID:
                while (job != NULL) {
                        if (job->pid == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_ID:
                while (job != NULL) {
                        if (job->id == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        case BY_JOB_STATUS:
                while (job != NULL) {
                        if (job->status == searchValue)
                                return job;
                        else
                                job = job->next;
                }
                break;
        default:
                return NULL;
                break;
        }
        return NULL;
}

/**
 * prints the active processes launched by the shell
 */
void printJobs()
{
        printf("\nActive jobs:\n");
        printf(
                "---------------------------------------------------------------------------\n");
        printf("| %7s  | %30s | %5s | %10s | %6s |\n", "job no.", "name", "pid",
               "descriptor", "status");
        printf(
                "---------------------------------------------------------------------------\n");
        t_job* job = jobsList;
        if (job == NULL) {
                printf("| %s %62s |\n", "No Jobs.", "");
        } else {
                while (job != NULL) {
                        printf("|  %7d | %30s | %5d | %10s | %6c |\n", job->id, job->name,
                               job->pid, job->descriptor, job->status);
                        job = job->next;
                }
        }
        printf(
                "---------------------------------------------------------------------------\n");
