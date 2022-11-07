
main-debug:
	@g++ -I./ parser/*.cpp executor/*.cpp painter/*.cpp main.cpp -g -o main

main:
	@g++ -I./ parser/*.cpp executor/*.cpp painter/*.cpp main.cpp -O2 -o main

clean:
	@rm -f main
