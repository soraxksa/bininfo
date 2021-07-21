




all:
	g++ -std=c++17 loader.cc main.cc memory.cc -o loader -lbfd -lcapstone -fpermissive 
