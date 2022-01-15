#include <iostream>
#include "MyHEMesh.h"
// #include <gperftools/profiler.h>

int main(int argc, char* argv[])
{
    // if (argc != 3) {
    //     std::cout << "Invalid syntax!" << std::endl;
    // }
    MyHEMesh mesh;
    // std::string read, save;
    // std::getline(std::cin >> std::ws, read);
    // std::getline(std::cin >> std::ws, save);
    // int target;
    // std::cin >> target;
    // int v1, v2;
    // std::cin >> v1 >> v2;
    const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/galera_409576.obj";
    const std::string save = "galera.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/hand_654666.obj";
    // const std::string save = "hand.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/tortuga_152728.obj";
    // const std::string save = "tortuga.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/dragon_871306.obj";
    // const std::string save = "dragon.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/Armadillo_345944.obj";
    // const std::string save = "Armadillo.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/angel_474048.obj";
    // const std::string save = "angel.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/3DBenchy_225143.obj";
    // const std::string save = "benchy.obj";
    // const std::string read = "/Users/calaw/Documents/Program/DSA/Project/projectb/data/data_advanced1/happy_1087440.obj";
    // const std::string save = "happy.obj";
    mesh.ReadFromOBJ(read);
    // mesh.ContractModel(target);
    // ProfilerStart((save + ".prof").c_str());
    // mesh.ContractModel(target);
    mesh.ContractModel(10000);
    // ProfilerStop();
    // mesh.ContractVPair(v1, v2);
    mesh.SaveToOBJ(save);
    // mesh.ReadFromOBJ(argv[1]);
    // mesh.SaveToOBJ(argv[2]);
}