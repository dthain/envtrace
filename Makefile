envtrace-helper.so: envtrace-helper.c
	gcc -fPIC -Wall -shared $< -o $@

clean:
	rm -f envtrace-helper.so
