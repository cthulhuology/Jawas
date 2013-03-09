
#include "include.h"
#include "defines.h"

void
demon(int detach)
{
	int murder = 0;
	int child = 0;
	int done = 0;
	int child_status = 0;
restart:
	if (murder) kill(child,SIGTERM);
	if (done) exit(JAWAS_EXIT_DONE);
	if (detach) child = fork();
	if (child == 0) return;
	char pid[17];
	int pidfd = open("jawas.pid",O_CREAT|O_WRONLY|O_TRUNC,0600);
	if (pidfd > 0) {
		snprintf(&pid[0],16,"%i\n",getpid());
		if (write(pidfd,pid,strlen(pid)) < 0)
			fprintf(stderr,"Failed to write pid");
		close(pidfd);
	} else perror("open");
	void kill_all(int sig) {
		murder = 1;
		done = 1;
	}
	void kill_child(int sig) {
		murder = 1;
	}
	void restart_child(int sig) {
		kill(child,SIGHUP);
	}
	signal(SIGQUIT,kill_all);
	signal(SIGTERM,kill_all);
	signal(SIGHUP,restart_child);
	while (!waitpid(child,&child_status,WNOHANG)) {
		if (murder) kill(child,SIGTERM);
		sleep(1);
	}
	goto restart;
}