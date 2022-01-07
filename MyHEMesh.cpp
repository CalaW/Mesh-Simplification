#include <iostream>
#include <fstream>
#include <exception>
#include <iostream>
#include "MyHEMesh.h"

using std::vector;
using std::string;
using std::to_string;

Vertex* MyHEMesh::InsertrVertex(double x, double y, double z) {
    Vertex* v = new Vertex;
    v->x = x; v->y = y; v->z = z;
    VertexVec_.push_back(v);
    VertexSet_.insert(v);
    return v;
}

Face* MyHEMesh::InsertFace(Vertex* v1, Vertex* v2, Vertex* v3) {
    Face* f = new Face;
    f->v[0] = v1; f->v[1] = v2; f->v[2] = v3;
    FaceSet_.insert(f);
    HEdge* h1 = InsertHEdge(v1, v2);
    HEdge* h2 = InsertHEdge(v2, v3);
    HEdge* h3 = InsertHEdge(v3, v1);
    h1->next = h2;
    h2->next = h3;
    h3->next = h1;

    f->h = h1;
    f->v[0]->h = h1;
    f->v[1]->h = h2;
    f->v[2]->h = h3;

    return f;
}

HEdge* MyHEMesh::InsertHEdge(Vertex* v1, Vertex* v2) {
    HEdge* h = new HEdge;
    h->v = v2;
    auto search = HEdgeMap_.find(VertexPair(v1, v2));
    if (search == HEdgeMap_.end()) {
        HEdgeMap_.insert({VertexPair(v1, v2), h});
        // std::cout << "insert HEdge from " << v1 << " to " << v2 << std::endl;
    }
    search = HEdgeMap_.find(VertexPair(v2, v1));
    if (search != HEdgeMap_.end()) {
        h->pair = search->second;
        search->second->pair = h;
        // std::cout << "find reverse HEdge from " << v2 << " to " << v1 << std::endl;
    } else {
        // std::cout << "going to insert reverse HEdge from " << v2 << " to " << v1 << std::endl;
        InsertHEdge(v2, v1);
    }
    return h;
}

void MyHEMesh::ReadFromOBJ(const string& path) {
    std::ifstream obj_file(path);
    if (!obj_file.is_open()) {
        throw std::invalid_argument("Cannot open file \"" + path + "\"");
    }
    int line_num = 0;
    while (obj_file.peek() != EOF) {
        char flag;
        string temp_str;
        line_num++;
        flag = (obj_file >> std::ws).get();
        if (obj_file.peek() != ' ')
            throw std::invalid_argument("Invalid format at line " + to_string(line_num));
        Vertex* v;
        switch (flag) {
            case '#':
                std::getline(obj_file, temp_str);
                continue;
            case 'v':
                double x, y, z;
                obj_file >> x >> y >> z >> std::ws;
                v = InsertrVertex(x, y, z);
                v->index = line_num; //TODO
                if (std::isdigit(obj_file.peek())) {
                    int r, g, b;
                    obj_file >> r >> g >> b;
                    v->r = r; v->g = g; v->b = b;
                }
                break;
            case 'f':
                int v1, v2, v3;
                obj_file >> v1 >> v2 >> v3;
                InsertFace(VertexVec_[v1-1], VertexVec_[v2-1], VertexVec_[v3-1]);
                break;
            default:
                throw std::invalid_argument("Invalid flag at line " + to_string(line_num));
        }
    }
}

void MyHEMesh::SaveToOBJ(const string& path) {
    std::ofstream obj_file(path);
    if (!obj_file.is_open()) {
        throw std::invalid_argument("Cannot create file \"" + path + "\"");
    }
    obj_file << "# " << VertexSet_.size() << " vertices\n";
    int vertex_cnt = 0;
    for (auto it : VertexSet_) {
        vertex_cnt++; it->index = vertex_cnt;
        obj_file << "v " << it->x << ' ' << it->y << ' ' << it->z;
        if (it->r != 255 || it->g != 255 || it->b != 255) {
            obj_file << ' ' << (int)it->r << ' ' << (int)it->g << ' ' << (int)it->b;
        }
        obj_file << '\n';
    }
    obj_file << std::endl;

    obj_file << "# " << FaceSet_.size() << " faces\n";
    for (const auto it : FaceSet_) {
        obj_file << "f " << it->v[0]->index << ' ' << it->v[1]->index << ' ' << it->v[2]->index << '\n';
    }
    obj_file << std::endl;
}

void MyHEMesh::UpdateQMatrix(Vertex* v) {

}

void MyHEMesh::UpdateAllQMatrix() {
    for (auto it : VertexSet_) {
        UpdateQMatrix(it);
    }
}