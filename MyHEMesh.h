#pragma once
#ifndef _MYMESH_H_
#define _MYMESH_H_

#if __INTELLISENSE__  //solve intellisence error on arm mac.
#undef __ARM_NEON     //see https://github.com/microsoft/vscode-cpptools/issues/7413
#undef __ARM_NEON__
#endif

#include <vector>
#include <string>
#include <map>
#include <set>
#include <Eigen/Dense>

struct HEdge;
struct Face;
struct Vertex;
struct VertexPair;

struct HEdge {
    HEdge* pair = nullptr;
    HEdge* next = nullptr; //paired HEdge and next HEdge
    Vertex* v; //point at v
    Face* f; //left face
    VertexPair* vpair;
};

struct Face {
    Vertex* v[3];
    HEdge* h; //arbitrary Hedge of the face
};

struct Vertex {
    double x, y, z;
    unsigned char r=255, g=255, b=255;
    Eigen::Matrix4d Q = Eigen::Matrix4d::Zero();
    int index; //works when output
    HEdge* h = nullptr;
};

inline Eigen::Vector3d GetPosVec(const Vertex* v);

struct VertexPairKey {
    Vertex* start;
    Vertex* end;
    VertexPairKey(Vertex* v1, Vertex* v2) {
        start = v1;
        end = v2;
    }
    bool operator< (const VertexPairKey& other) const {
        return start == other.start ? end < other.end : start < other.start;
    }
};

struct VertexPair {
    Vertex* v1;
    Vertex* v2;
    double cost = INFINITY;
    Eigen::Vector4d v;
    bool erased = false;
    bool operator< (const VertexPair& other) const {
        return cost < other.cost;
    }
    bool operator> (const VertexPair& other) const {
        return cost > other.cost;
    }
};
struct VPairPtrGreater {
    bool operator() (const VertexPair* v1, const VertexPair* v2) {
        return v1->cost > v2->cost;
    }
};

class MyHEMesh {
private:
    std::vector<HEdge*> HEdgeVec_;
    std::map<VertexPairKey, HEdge*> HEdgeMap_;
    std::set<Face*> FaceSet_;
    std::vector<Vertex*> VertexVec_;
    std::set<Vertex*> VertexSet_;
    std::vector<VertexPair*> VPairHeap_;
    std::set<VertexPairKey> DelVPairSet_;
    Vertex* InsertrVertex(double x, double y, double z);
    Face* InsertFace(Vertex* v1, Vertex* v2, Vertex* v3);
    HEdge* InsertHEdge(Vertex* v1, Vertex* v2);
    bool HasFoldFace(const VertexPair* vpair, const std::vector<Face*>& NeighbFaceVec);
    bool IsCollapseable(const std::vector<Vertex*>& VertexVec, const Vertex* v1, const Vertex* v2);

public:
    void ReadFromOBJ(const std::string& path);
    void SaveToOBJ(const std::string& path);
    void UpdateQMatrix(Vertex& v);
    void UpdateAllQMatrix();
    void UpdateVPairCost(VertexPair* vpair);
    void UpdateAllVPairCost();
    void MakeVPairHeap();
    void ReaddVPair(double threshold);
    void ContractModel(long facenum);
    void ContractLeastCostPair();
    void ContractVPair(VertexPair* vpair);
    void ContractInitModel(long v1index, long v2index);

    static Eigen::Vector4d CalcP(const Face* f);
};

#endif
