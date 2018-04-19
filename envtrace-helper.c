#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>
#include <errno.h>
#include <string.h>

extern char * program_invocation_name;

static FILE * logfile = 0;
static const char * logfile_name = 0;
static char * (*real_getenv) ( const char *name ) = 0;

char * getenv( const char *name )
{
	if(!real_getenv) {
		real_getenv = dlsym(RTLD_NEXT,"getenv");
		if(!real_getenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original getenv()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = real_getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",logfile,strerror(errno));
                        exit(1);
                }
	}

	char *result = real_getenv(name);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp, NULL);

	if(result) {	
		fprintf(logfile,"%ld: %ld %ld %s %s HIT %s\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name,result);
	} else {
		fprintf(logfile,"%ld: %ld %ld %s %s MISS\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name);
	}
	fflush(logfile);

	return result;
}

