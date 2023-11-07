default:
	@gcc -o perfect perfect.c
test: 
	@gcc -o perfect perfect.c
	@echo Test 1:
	./perfect
	@echo Test 2:
	./perfect -s 100 -e 10000
	@echo Test 3:
	./perfect -v
	@echo Test 4:
	./perfect -t 2 -v
	@echo Test 5:
	./perfect -t 4 -v
	@echo Test 6:
	./perfect -e 100000000 -t 20 -v

time:
	@gcc -o perfect perfect.c
	@echo Testing how -t option impacts executing time
	@echo I am using Ubuntu and my CPU have 20 cores.
	@echo Interval: [1, 1000000000]
	@echo Test running time no -t option :
	@time --format "Running time %E\n" ./perfect -e 10000000
	@echo Test running time with t = 2:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 2
	@echo Test running time with t = 5:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 5
	@echo Test running time with t = 8:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 8
	@echo Test running time with t = 10:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 10
	@echo Test running time with t = 12:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 12
	@echo Test running time with t = 15:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 15
	@echo Test running time with t = 18:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 18
	@echo Test running time with t = 20:
	@time --format "Running time %E\n" ./perfect -e 10000000 -t 20
	
clean:
	@rm -rf perfect *.exe *.o *.out