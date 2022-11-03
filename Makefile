main-debug:
	@g++ *.cpp -g -o main

main:
	@g++ *.cpp -O2 -o main

clean:
	@rm -f main
