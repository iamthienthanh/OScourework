build:
	@gcc -o quiz quiz.c fetch.c parse.c play.c -ljansson
check:
	valgrind --leak-check=full --show-leak-kinds=all ./quiz
run:
	./quiz
clean: 
	@rm -r quiz