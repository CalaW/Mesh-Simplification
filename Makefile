main: main.cpp MyHEMesh.cpp MyHEMesh.h
	clang++ main.cpp MyHEMesh.cpp -o main -std=c++11
debug: main.cpp MyHEMesh.cpp MyHEMesh.h
	clang++ main.cpp MyHEMesh.cpp -o main -std=c++11 -g
spd: main.cpp MyHEMesh.cpp MyHEMesh.h
	clang++ main.cpp MyHEMesh.cpp -o main -std=c++20 -O3