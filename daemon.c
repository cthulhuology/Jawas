
#include "include.h"

void
demon(int detach)
{
	int murder = 0;
	int child = 0;
	int done = 0;
	int child_status = 0;
restart:
	if (murder) kill(child,SIGTERM);
	if (done) exit(0);
	if (detach) child = fork();
	if (child == 0) return;
	char* pid = NULL;
	int pidfd = open("jawas.pid",O_CREAT|O_WRONLY|O_TRUNC,0600);
	if (pidfd > 0) {
		asprintf(&pid,"%i\n",getpid());
		write(pidfd,pid,strlen(pid));
		close(pidfd);
	} else perror("open");
	void kill_all(int sig) {
		murder = 1;
		done = 1;
	}
	void kill_child(int sig) {
		murder = 1;
	}
	signal(SIGQUIT,kill_all);
	signal(SIGTERM,kill_all);
	signal(SIGHUP,kill_child);
	while (!waitpid(child,&child_status,WNOHANG)) {
		if (murder) kill(child,SIGTERM);
		sleep(1);
	}
	goto restart;
}
