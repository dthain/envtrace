#define _GNU_SOURCE
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>
#include <dlfcn.h>
#include <time.h>
#include <limits.h>
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
static int (*real_openat) ( int dirfd, const char *pathname, int flags,... ) = 0;
static int (*real_creat) ( const char *pathname,mode_t mode ) = 0;
static int (*real_execv) ( const char *path,char *const argv[] ) = 0;
static int (*real_execl) ( const char *path,const char *arg0,... ) = 0;
static int (*real_execve) ( const char *path,char *const argv[],char *const envp[] ) = 0;
static int (*real_execle) ( const char *path,const char *arg0,... ) = 0;
static int (*real_execvp) ( const char *file,char *const argv[] ) = 0;
static int (*real_execlp) ( const char *file,const char *arg0,... ) = 0;
static int (*real_execvpe) ( const char *file,char *const argv[],char *const envp[] ) = 0;

void logfile_check()
{
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
}

char * getenv( const char *name )
{
	if(!real_getenv) {
		real_getenv = dlsym(RTLD_NEXT,"getenv");
		if(!real_getenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original getenv().\n");
			exit(1);
		}
	}

	logfile_check();

	char *result = real_getenv(name);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	if(result) {	
		fprintf(logfile,"%s@%s %ld.%ld: GETENV %ld %ld %s %s HIT %s\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,name,result);
	} else {
		fprintf(logfile,"%s@%s %ld.%ld: GETENV %ld %ld %s %s MISS\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,name);
	}
	fflush(logfile);

	return result;
}

int setenv( const char *name,const char *value,int overwrite )
{
	if(!real_setenv) {
		real_setenv = dlsym(RTLD_NEXT,"setenv");
		if(!real_setenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original setenv().\n");
			exit(1);
		}
	}

    logfile_check();

	const char *prev_val = getenv(name);
	int result = real_setenv(name,value,overwrite);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: SETENV %ld %ld %s %s %s %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,name,prev_val,value,result);
	fflush(logfile);

	return result;

}

int unsetenv( const char *name )
{
	if(!real_unsetenv) {
		real_unsetenv = dlsym(RTLD_NEXT,"unsetenv");
		if(!real_unsetenv) {
			fprintf(stderr,"envtrace-helper: couldn't find original unsetenv().\n");
			exit(1);
		}
	}

    logfile_check();

	const char *prev_val = getenv(name);
	int result = real_unsetenv(name);

	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: UNSET %ld %ld %s %s %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,name,prev_val,result);
	fflush(logfile);

	return result;

}

pid_t fork()
{
	if(!real_fork) {
		real_fork = dlsym(RTLD_NEXT,"fork");
		if(!real_fork) {
			fprintf(stderr,"envtrace-helper: couldn't find original fork().\n");
			exit(1);
		}
	}

    logfile_check();

	pid_t result = real_fork();
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(1024);
	size_t hostsize = 1024;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: FORK %ld %ld %s %"PRId64"\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,(int64_t) result);
	fflush(logfile);

	return result;
}

int clone( int (*fn)(void *),void *child_stack,int flags,void *arg,.../* pid_t *ptid, void *newtls, pid_t *ctid */ )
{
	if(!real_clone) {
		real_clone = dlsym(RTLD_NEXT,"clone");
		if(!real_clone) {
			fprintf(stderr,"envtrace-helper: couldn't find original clone().\n");
			exit(1);
		}
	}

    logfile_check();

	int result = real_clone(fn,child_stack,flags,arg);
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: CLONE %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
	fflush(logfile);

	return result;
}

int open( const char *pathname,int flags,... )
{
	if(!real_open) {
		real_open = dlsym(RTLD_NEXT,"open");
		if(!real_open) {
			fprintf(stderr,"envtrace-helper: couldn't find original open().\n");
			exit(1);
		}
	}

    logfile_check();

	va_list ap;
	int mode;
	va_start(ap, flags);
	mode = va_arg(ap, int);
	va_end(ap);

	int result = real_open(pathname,flags,mode);
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: OPEN %ld %ld %s %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,pathname,result);
	fflush(logfile);

	return result;
}

int openat ( int dirfd, const char *pathname, int flags,... )
{
	if(!real_openat) {
		real_openat = dlsym(RTLD_NEXT,"openat");
		if(!real_openat) {
			fprintf(stderr,"envtrace-helper: couldn't find original openat().\n");
			exit(1);
		}
	}

    logfile_check();

	va_list ap;
	int mode;
	va_start(ap, flags);
	mode = va_arg(ap, int);
	va_end(ap);

	int result = real_openat(dirfd,pathname,flags,mode);
	pid_t pid = getpid();
	pid_t ppid = getppid();
	struct timespec timestamp;
	clock_gettime(CLOCK_REALTIME,&timestamp);
	char *user = real_getenv("USER");
	char *hostname = malloc(sizeof(INT_MAX));
	size_t hostsize = INT_MAX;
	gethostname(hostname,hostsize);

	fprintf(logfile,"%s@%s %ld.%ld: OPENAT %ld %ld %s %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,pathname,result);
	fflush(logfile);

	return result;
}

int creat( const char *pathname,mode_t mode )
{
    if(!real_creat) {
        real_creat = dlsym(RTLD_NEXT,"creat");
        if(!real_creat) {
                fprintf(stderr,"envtrace-helper: couldn't find original creat().\n");
                exit(1);
        }
    }

    logfile_check();

    int result = real_creat(pathname,mode);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: CREAT %ld %ld %s %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,pathname,result);
    fflush(logfile);
    
    return result;
}

int execv ( const char *path,char *const argv[] )
{
    if(!real_execv) {
        real_execv = dlsym(RTLD_NEXT,"execv");
        if(!real_execv) {
                fprintf(stderr,"envtrace-helper: couldn't find original execv().\n");
                exit(1);
        }
    }

    logfile_check();

    int result = real_execv(path,argv);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECV %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execl ( const char *path,const char *arg0,... )
{
     if(!real_execl) {
        real_execl = dlsym(RTLD_NEXT,"execl");
        if(!real_execl) {
                fprintf(stderr,"envtrace-helper: couldn't find original execl().\n");
                exit(1);
        }
    }

    logfile_check();

    ptrdiff_t argc;
    va_list ap;
    va_start (ap, arg0);
    for (argc = 1; va_arg (ap, const char *); argc++)
    {
        if (argc == INT_MAX)
        {
            va_end (ap);
            errno = E2BIG;
            return -1;
        }
    }
    va_end (ap);

    ptrdiff_t i;
    char *argv[argc + 1];
    va_start (ap, arg0);
    argv[0] = (char *) arg0;
    for (i = 1; i <= argc; i++)
    {
        argv[i] = va_arg (ap, char *);
    }
    va_end (ap);

    if(!real_execv) {
        real_execv = dlsym(RTLD_NEXT,"execv");
        if(!real_execv) {
                fprintf(stderr,"envtrace-helper: couldn't find original execv() for executing execl().\n");
                exit(1);
        }
    }
    int result = real_execv(path,argv);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECL %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execve ( const char *path,char *const argv[],char *const envp[] )
{
    if(!real_execve) {
        real_execve = dlsym(RTLD_NEXT,"execve");
        if(!real_execve) {
                fprintf(stderr,"envtrace-helper: couldn't find original execve().\n");
                exit(1);
        }
    }

    logfile_check();

    int result = real_execve(path,argv,envp);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECVE %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execle ( const char *path,const char *arg0,... )
{
    if(!real_execle) {
        real_execle = dlsym(RTLD_NEXT,"execle");
        if(!real_execle) {
                fprintf(stderr,"envtrace-helper: couldn't find original execle().\n");
                exit(1);
        }
    }

    logfile_check();

    ptrdiff_t argc;
    va_list ap;
    va_start (ap, arg0);
    for (argc = 1; va_arg (ap, const char *); argc++)
    {
        if (argc == INT_MAX)
        {
            va_end (ap);
            errno = E2BIG;
            return -1;
        }
    }
    va_end (ap);

    ptrdiff_t i;
    char *argv[argc + 1];
    char **envp;
    va_start (ap, arg0);
    argv[0] = (char *) arg0;
    for (i = 1; i <= argc; i++)
    {
        argv[i] = va_arg (ap, char *);
    }
    envp = va_arg (ap, char **);
    va_end (ap);

    if(!real_execve) {
        real_execve = dlsym(RTLD_NEXT,"execve");
        if(!real_execve) {
                fprintf(stderr,"envtrace-helper: couldn't find original execve() for executing execle().\n");
                exit(1);
        }
    }
    int result = real_execve(path,argv,envp);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECLE %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execvp ( const char *file,char *const argv[] )
{
    if(!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT,"execvp");
        if(!real_execvp) {
                fprintf(stderr,"envtrace-helper: couldn't find original execvp().\n");
                exit(1);
        }
    }

    logfile_check();

    int result = real_execvp(file,argv);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECVP %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execlp ( const char *file,const char *arg0,... )
{
     if(!real_execlp) {
        real_execlp = dlsym(RTLD_NEXT,"execlp");
        if(!real_execlp) {
                fprintf(stderr,"envtrace-helper: couldn't find original execlp().\n");
                exit(1);
        }
    }

    logfile_check();

    ptrdiff_t argc;
    va_list ap;
    va_start (ap, arg0);
    for (argc = 1; va_arg (ap, const char *); argc++)
    {
        if (argc == INT_MAX)
        {
            va_end (ap);
            errno = E2BIG;
            return -1;
        }
    }
    va_end (ap);

    ptrdiff_t i;
    char *argv[argc + 1];
    va_start (ap, arg0);
    argv[0] = (char *) arg0;
    for (i = 1; i <= argc; i++)
    {
        argv[i] = va_arg (ap, char *);
    }
    va_end (ap);

    if(!real_execvp) {
        real_execvp = dlsym(RTLD_NEXT,"execvp");
        if(!real_execvp) {
                fprintf(stderr,"envtrace-helper: couldn't find original execvp() for executing execlp().\n");
                exit(1);
        }
    }
    int result = real_execvp(file,argv);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECLP %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}

int execvpe ( const char *file,char *const argv[],char *const envp[] )
{
    if(!real_execvpe) {
        real_execvpe = dlsym(RTLD_NEXT,"execvpe");
        if(!real_execvpe) {
                fprintf(stderr,"envtrace-helper: couldn't find original execvpe().\n");
                exit(1);
        }
    }

    logfile_check();

    int result = real_execvpe(file,argv,envp);
    pid_t pid = getpid();
    pid_t ppid = getppid();
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME,&timestamp);
    char *user = real_getenv("USER");
    char *hostname = malloc(sizeof(INT_MAX));
    size_t hostsize = INT_MAX;
    gethostname(hostname,hostsize);

    fprintf(logfile,"%s@%s %ld.%ld: EXECVPE %ld %ld %s %d\n",user,hostname,(long)timestamp.tv_sec,timestamp.tv_nsec,(long)ppid,(long)pid,program_invocation_name,result);
    fflush(logfile);
    
    return result;
}
