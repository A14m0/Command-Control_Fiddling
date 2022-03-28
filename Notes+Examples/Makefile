
build:
	gcc -c -fPIC so_malloc_test.c -o example.o
	gcc example.o -shared -o libexample.so

	gcc objldr.c -ldl