default:
	@gcc -o coins coins.c

test:
	@gcc -o coins coins.c
	@echo The execution times measured on a PC with an 4.9 GHZ 12th Gen Intel Core i7-12700F Processor and 8x4 GB 2133 MHz LPDDR4 RAM: > time.txt
	@./coins -n 1000 >> time.txt

clean:
	@rm -rf coins *.out