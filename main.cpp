#include <iostream>
#include "MyHEMesh.h"
#include <ctime>
// #include <gperftools/profiler.h>

int main()
{
    MyHEMesh mesh;
    std::string read, save;
    std::getline(std::cin >> std::ws, read);
    std::getline(std::cin >> std::ws, save);
    int target;
    std::cin >> target;
    // int v1, v2;
    // std::cin >> v1 >> v2;
    mesh.ReadFromOBJ(read);
    clock_t start, end;
    start = clock();
    // ProfilerStart((save + ".prof").c_str());
    mesh.ContractModel(target);
    // mesh.ContractModel(10000);
    // mesh.ContractInitModel(1922, 3640);
    // mesh.ContractModel(mesh.GetFaceNum()/10);
    // mesh.ContractVPair(v1, v2);
    // ProfilerStop();
    end = clock();
    std::cout << "contract time = " << double(end-start) / CLOCKS_PER_SEC << "s" << std::endl;
    mesh.SaveToOBJ(save);
}