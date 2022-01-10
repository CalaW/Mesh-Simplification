#include <iostream>
#include "MyHEMesh.h"

int main(int argc, char* argv[])
{
    // if (argc != 3) {
    //     std::cout << "Invalid syntax!" << std::endl;
    // }
    MyHEMesh mesh;
    std::string read, save;
    std::getline(std::cin >> std::ws, read);
    std::getline(std::cin >> std::ws, save);
    // int target;
    // std::cin >> target;
    int v1, v2;
    std::cin >> v1 >> v2;
    mesh.ReadFromOBJ(read);
    // mesh.ContractModel(target);
    mesh.ContractVPair(v1, v2);
    mesh.SaveToOBJ(save);
    // mesh.ReadFromOBJ(argv[1]);
    // mesh.SaveToOBJ(argv[2]);
}