#pragma once
#ifndef _MYMESH_H_
#define _MYMESH_H_

#include <vector>
#include <string>
#include <map>
#include <set>
#include <Eigen/Dense>

struct HEdge;
struct Face;
struct Vertex;

struct HEdge {
    HEdge* pair, *next; //paired HEdge and next HEdge
    Vertex* v; //point at v
    Face* f; //left face
};

struct Face {
    Vertex* v[3];
    HEdge* h; //arbitrary Hedge of the face
};

struct Vertex {
    double x, y, z;
    unsigned char r=255, g=255, b=255;

    int index; //works when output
    HEdge* h;
};

struct VertexPair {
    Vertex* start;
    Vertex* end;
    VertexPair(Vertex* v1, Vertex* v2) {
        start = v1;
        end = v2;
    }
    bool operator< (const VertexPair& other) const {
        return start == other.start ? end < other.end : start < other.start;
    }
};

class MyHEMesh {
private:
    std::vector<HEdge*> HEdgeVec_;
    std::map<VertexPair, HEdge*> HEdgeMap_;
    std::set<Face*> FaceSet_;
    std::vector<Vertex*> VertexVec_;
    std::set<Vertex*> VertexSet_;

public:
    void ReadFromOBJ(const std::string& path);
    void SaveToOBJ(const std::string& path);
    Vertex* InsertrVertex(double x, double y, double z);
    Face* InsertFace(Vertex* v1, Vertex* v2, Vertex* v3);
    HEdge* InsertHEdge(Vertex* v1, Vertex* v2);
    void UpdateQMatrix(Vertex* v);
    void UpdateAllQMatrix();
};

#endif
