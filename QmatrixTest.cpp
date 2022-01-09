#include <iostream>
#include <math.h>
#include <Eigen/Dense>

using namespace std;

struct Vertex {
    double x, y, z;
};
    
int main()
{
    Vertex v1, v2, v3;
    cin >> v1.x >> v1.y >> v1.z
        >> v2.x >> v2.y >> v2.z
        >> v3.x >> v3.y >> v3.z;
    
    Eigen::Vector3d vec12(v2.x-v1.x, v2.y-v1.y, v2.z-v1.z);
    Eigen::Vector3d vec23(v3.x-v2.x, v3.y-v2.y, v3.z-v2.z);

    Eigen::Vector3d veccross = vec12.cross(vec23);
    // veccross /= sqrt(veccross.array().pow(2).sum());
    veccross.normalize();

    Eigen::Vector4d faceEqn;
    faceEqn << veccross, -veccross[0]*v1.x -veccross[1]*v1.y -veccross[2]*v1.z;

    cout << veccross << endl << faceEqn << endl;
    
}