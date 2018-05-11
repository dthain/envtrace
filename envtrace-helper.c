#define _GNU_SOURCE
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>

extern char * program_invocation_name;

static FILE * logfile = 0;
static const char * logfile_name = 0;
static char * (*real_getenv) ( const char *name ) = 0;
static int (*real_setenv) ( const char *name,const char *value,int overwrite ) = 0;
static int (*real_unsetenv) ( const char *name ) = 0;
static pid_t (*real_fork) ( ) = 0;
static int (*real_clone) ( int (*fn)(void *),void *child_stack,int flags,void *arg,... ) = 0;
static int (*real_open) ( const char *pathname,int flags,... ) = 0;
static int (*real_creat) ( const char *pathname,mode_t mode ) = 0;

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
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}

	char *result = real_getenv(name);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp,NULL);

	if(result) {	
		fprintf(logfile,"%ld: %ld %ld %s %s HIT %s\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name,result);
	} else {
		fprintf(logfile,"%ld: %ld %ld %s %s MISS\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name);
	}
	fflush(logfile);

	return result;
}

int setenv(const char *name, const char *value, int overwrite)
{
	if(!real_setenv) {
		real_setenv = dlsym(RTLD_NEXT,"setenv");
		if(!real_setenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original setenv()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}

	int result = real_setenv(name,value,overwrite);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp,NULL);

	fprintf(logfile,"%ld: SETENV %ld %ld %s %s %s %d\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name,value,result);
	fflush(logfile);

	return result;

}

int unsetenv(const char *name)
{
	if(!real_unsetenv) {
		real_unsetenv = dlsym(RTLD_NEXT,"unsetenv");
		if(!real_unsetenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original unsetenv()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}

	int result = real_unsetenv(name);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp,NULL);
	fprintf(logfile,"%ld: UNSET %ld %ld %s %s %d\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,name,result);
	fflush(logfile);

	return result;

}

pid_t fork()
{
	if(!real_fork) {
		real_fork = dlsym(RTLD_NEXT,"fork");
		if(!real_fork) {
			fprintf(stderr,"envtrace-helper: couldn't find original fork()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}
	
	pid_t result = real_fork();
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp,NULL);
	fprintf(logfile,"%ld: FORK %ld %ld %s %"PRId64"\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,(int64_t) result);
	fflush(logfile);

	return result;
}

int clone( int (*fn)(void *),void *child_stack,int flags,void *arg,.../* pid_t *ptid, void *newtls, pid_t *ctid */ )
{
	if(!real_clone) {
		real_clone = dlsym(RTLD_NEXT,"clone");
		if(!real_clone) {
			fprintf(stderr,"envtrace-helper: couldn't find original clone()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}
	
	int result = real_clone(fn,child_stack,flags,arg);
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
    	gettimeofday(&timestamp,NULL);
	fprintf(logfile,"%ld: CLONE %ld %ld %s %d\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,result);
	fflush(logfile);

	return result;
}


int open( const char *pathname,int flags,... )
{
	if(!real_open) {
		real_open = dlsym(RTLD_NEXT,"open");
		if(!real_open) {
			fprintf(stderr,"envtrace-helper: couldn't find original open()\n");
			exit(1);
		}
	}

	if(!logfile_name) {
		logfile_name = getenv("ENVTRACE_LOGFILE");
		if(!logfile_name) {
			fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
			exit(1);
		}
	}

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
	}

	va_list ap;
	int mode;
	va_start(ap, flags);
	mode = va_arg(ap, int);
	va_end(ap);

	int result = real_open(pathname,flags,mode);
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timeval timestamp;
	gettimeofday(&timestamp,NULL);
	fprintf(logfile,"%ld: OPEN %ld %ld %s %d\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,result);
	fflush(logfile);

	return result;
}

int creat( const char *pathname,mode_t mode )
{
        if(!real_creat) {
                real_creat = dlsym(RTLD_NEXT,"creat");
                if(!real_creat) {
                        fprintf(stderr,"envtrace-helper: couldn't find original creat()\n");
                        exit(1);
                }
        }

        if(!logfile_name) {
                logfile_name = getenv("ENVTRACE_LOGFILE");
                if(!logfile_name) {
                        fprintf(stderr,"envtrace-helper: ENVTRACE_LOGFILE is not set.\n");
                        exit(1);
                }
        }

        if(!logfile) {
                logfile = fopen(logfile_name,"a");
                if(!logfile) {
                        fprintf(stderr,"envtrace-helper: couldn't open %s for logging: %s\n",(char *)logfile,strerror(errno));
                        exit(1);
                }
        }

        int result = real_creat(pathname,mode);
        pid_t pid = getpid();
        pid_t ppid = getppid();
        struct timeval timestamp;
        gettimeofday(&timestamp,NULL);
        fprintf(logfile,"%ld: CREAT %ld %ld %s %d\n",(long)timestamp.tv_sec,(long)ppid,(long)pid,program_invocation_name,result);
        fflush(logfile);

        return result;

}
