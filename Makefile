envtrace-helper.so: envtrace-helper.c
	gcc -fPIC -shared $< -o $@

clean:
	rm -f envtrace-helper.so
