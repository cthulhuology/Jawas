
#include "include.h"
#include "defines.h"

int murder = 0;
int child = 0;
int done = 0;
int child_status = 0;

void kill_all(int sig) {
	fprintf(stderr,"process %d got killall\n", getpid());
	murder = 1;
	done = 1;
	kill(0,SIGTERM);	// kill our process group
	exit(0);
}

void kill_child(int sig) {
	fprintf(stderr,"process %d got kill child %d\n", getpid(), child);
	murder = 1;
	kill(child,SIGTERM);	// kill our child
}

void restart_child(int sig) {
	fprintf(stderr,"process %d got restart child %d\n", getpid(), child);
	kill(child,SIGHUP);
}

void
demon(int detach)
{
restart:
	if (murder) kill(child,SIGTERM);
	if (done) exit(JAWAS_EXIT_DONE);
	if (detach) {
		child = fork();
		if (child != 0) exit(0);
		child = fork();
	}
	if (child == 0) return;
	char pid[17];
	int pidfd = open("jawas.pid",O_CREAT|O_WRONLY|O_TRUNC,0600);
	if (pidfd > 0) {
		snprintf(&pid[0],16,"%i\n",getpid());
		if (write(pidfd,pid,strlen(pid)) < 0)
			fprintf(stderr,"Failed to write pid");
		close(pidfd);
	} else perror("open");
	signal(SIGQUIT,kill_all);
	signal(SIGTERM,kill_all);
	signal(SIGHUP,restart_child);
	while (!waitpid(child,&child_status,WNOHANG)) {
		if (murder) kill_child(SIGTERM);
		sleep(1);
	}
	goto restart;
}
