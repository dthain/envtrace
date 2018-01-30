envtrace - trace environment lookups
------------------------------------

Complex programs can often be difficult to debug
when they make use of environment variables:
it's not at all obvious what variables a program
may access, or what value they had at the time of access.

envtrace allows you to run a program and observe how
it accesses the environment.  First, build the software:

```
make
```

And then use `envtrace` as a wrapper around your program:
```
./envtrace ps
````

The results are placed into `env.trace` in the current working
directory.  Each line in this file gives the program name,
variable name, whether the lookup succeeded, and the value returned:

```
ps COLUMNS HIT 120
ps LINES MISS
ps PS_PERSONALITY MISS
ps I_WANT_A_BROKEN_PS MISS
...
```

This program is a 15-minute hack written to explore an idea and
see if it turns up any surprises.  In fact, this README is already longer than the source code of the program.
This has been tested on RHEL7 and might work on other Linuxes.
It probably needs some adjustment to work on other Unixes.
Please set your expectations
appropriately.

Douglas Thain
`dthain@nd.edu`



