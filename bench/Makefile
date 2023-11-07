all:
	@gcc -o bench bench.c

test:
	@echo Test 1:
	./bench -w 2 -d 4 sleep 1
	@echo Test 2:
	./bench -w 2 -d 1 -- sh -c 'date > /dev/null'
	@echo Test 3:
	./bench -w 2 -d 1 -- sh -c 'echo 1 > /dev/null'
	@echo Test 4:
	./bench -- sh -c 'env foo=bar > /dev/null'
clean:
	find . -type f -executable -delete