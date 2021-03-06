#if __longELLISENSE__  //solve longellisence error on arm mac.
#undef __ARM_NEON     //see https://github.com/microsoft/vscode-cpptools/issues/7413
#undef __ARM_NEON__
#endif

#include <iostream>
#include <fstream>
#include <exception>
#include <iostream>
#include <algorithm>
#include "MyHEMesh.h"

using std::vector;
using std::string;
using std::to_string;
using Eigen::Vector3d;
using Eigen::Vector4d;
using Eigen::Matrix4d;
using Eigen::seq;
using Eigen::fix;

Vector3d GetPosVec(const Vertex* v) {
    return Vector3d(v->x, v->y, v->z);
}

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

    h1->f = f;
    h2->f = f;
    h3->f = f;

    f->h = h1;
    f->v[0]->h = h1;
    f->v[1]->h = h2;
    f->v[2]->h = h3;

    return f;
}

HEdge* MyHEMesh::InsertHEdge(Vertex* v1, Vertex* v2) {
    HEdge* h;
    auto search = HEdgeMap_.find(VertexPairKey(v1, v2));
    if (search == HEdgeMap_.end()) {
        h = new HEdge;
        h->v = v1;
        HEdgeMap_.insert({VertexPairKey(v1, v2), h});
        // std::cout << "insert HEdge from " << v1 << " to " << v2 << std::endl;
        HEdge* hpair = new HEdge;
        hpair->v = v2;
        auto ins = HEdgeMap_.insert({VertexPairKey(v2, v1), hpair});
        // std::cout << "insert HEdge from " << v2 << " to " << v1 << std::endl << std::endl;
        h->pair = hpair;
        hpair->pair = h;

        VertexPair* vpair = new VertexPair;
        vpair->v1 = v1;
        vpair->v2 = v2;
        VPairHeap_.push_back(vpair);
    }
    else {
        h = search->second;
        // std::cout << "Already has" << v1 << " to " << v2 << std::endl << std::endl;
    }
    return h;
}

void MyHEMesh::ReadFromOBJ(const string& path) {
    std::ifstream obj_file(path);
    if (!obj_file.is_open()) {
        throw std::invalid_argument("Cannot open file \"" + path + "\"");
    }
    long line_num = 0;
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
                long v1, v2, v3;
                obj_file >> v1 >> v2 >> v3;
                InsertFace(VertexVec_[v1-1], VertexVec_[v2-1], VertexVec_[v3-1]);
                break;
            default:
                throw std::invalid_argument("Invalid flag at line " + to_string(line_num));
        }
    }
    for (auto vptr : VertexVec_) {
        CheckBound(vptr);
    }
    UpdateAllQMatrix();
    UpdateAllVPairCost();
    MakeVPairHeap();
}

void MyHEMesh::SaveToOBJ(const string& path) {
    std::ofstream obj_file(path);
    if (!obj_file.is_open()) {
        throw std::invalid_argument("Cannot create file \"" + path + "\"");
    }
    obj_file << "# " << VertexSet_.size() << " vertices\n";
    long vertex_cnt = 0;
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

Eigen::Vector4d MyHEMesh::CalcP(const Face* f) {
    Vector3d vec12(f->v[1]->x - f->v[0]->x, f->v[1]->y - f->v[0]->y, f->v[1]->z - f->v[0]->z);
    Vector3d vec23(f->v[2]->x - f->v[1]->x, f->v[2]->y - f->v[1]->y, f->v[2]->z - f->v[1]->z);

    Vector3d veccross = vec12.cross(vec23);
    veccross.normalize();

    Vector4d faceEqn;
    faceEqn << veccross, -veccross[0]*f->v[0]->x -veccross[1]*f->v[0]->y -veccross[2]*f->v[0]->z;

    return faceEqn;
}

void MyHEMesh::UpdateQMatrix(Vertex& v) {
    v.Q = Matrix4d::Zero();
    if (v.isBound) {
        HEdge* h = v.h->pair->next;
        HEdge* prev = v.h;
        while (h != nullptr) {
            Face* f = h->f;
            Vector4d p;
            p = CalcP(f);
            // std::cout << p.transpose() << '\n';
            v.Q += p * p.transpose();
            h = h->pair->next;
        }

        Vector3d initEdgeVec = GetPosVec(v.h->pair->v) - GetPosVec(&v);
        initEdgeVec.normalize();
        Vector4d initEdgeP(initEdgeVec(0), initEdgeVec(1), initEdgeVec(2), 1);
        v.Q += BoundPenalty * initEdgeP * initEdgeP.transpose();

        Vector3d endEdgeVec = GetPosVec(prev->pair->v) - GetPosVec(&v);
        endEdgeVec.normalize();
        Vector4d endEdgeP(initEdgeVec(0), initEdgeVec(1), initEdgeVec(2), 1);
        v.Q += BoundPenalty * endEdgeP * endEdgeP.transpose();
    } else {
        HEdge* h = v.h;
        // std::cout << "Update QMatrix of "<< v.x << v.y << v.z << '\n';
        do {
            Face* f = h->f;
            Vector4d p;
            p = CalcP(f);
            // std::cout << p.transpose() << '\n';
            v.Q += p * p.transpose();
            h = h->pair->next;
        } while (h != v.h);
    }

    // std::cout << std::endl;
}

void MyHEMesh::UpdateAllQMatrix() {
    for (auto it : VertexSet_) {
        UpdateQMatrix(*it);
    }
}

void MyHEMesh::UpdateVPairCost(VertexPair* vpair) {
    Matrix4d Qbar = vpair->v1->Q + vpair->v2->Q;
    Matrix4d A = Qbar;
    A(3,0) = 0; A(3,1) = 0; A(3,2) = 0; A(3,3) = 1;

    vpair->v = A.colPivHouseholderQr().solve(Vector4d(0, 0, 0, 1));
    // std::cout << vpair->v.transpose() << std::endl;
    if (abs((A*vpair->v - Vector4d(0,0,0,1)).norm()) > 1e-10) {
        // std::cout << (A*vpair->v - Vector4d(0,0,0,1)).norm();
        vpair->v(0) = (vpair->v1->x + vpair->v2->x) / 2;
        vpair->v(1) = (vpair->v1->y + vpair->v2->y) / 2;
        vpair->v(2) = (vpair->v1->z + vpair->v2->z) / 2;
    }
    vpair->cost = vpair->v.transpose() * Qbar * vpair->v;
    // std::cout << "update cost of " << vpair->v1->x << ' ' << vpair->v1->y << ' ' << vpair->v1->z << " & "
    //     << vpair->v2->x << ' ' << vpair->v2->y << ' ' << vpair->v2->z << '\n';
    // std::cout << vpair.cost << '\n' << Qbar << '\n' << vpair.v << '\n' << std::endl;
}

void MyHEMesh::UpdateAllVPairCost() {
    for (auto it : VPairHeap_) {
        UpdateVPairCost(it);
    }
}

void MyHEMesh::MakeVPairHeap() {
    std::make_heap(VPairHeap_.begin(), VPairHeap_.end(), VPairPtrGreater());
    // for (auto it : VPairHeap_) {
    //     std::cout << it.cost << ' ';
    // }
    // std::cout << std::endl;
}

void MyHEMesh::ReaddVPair(double threshold) {
    VPairHeap_.clear();
    for (auto it : HEdgeMap_) {
        VertexPair* vpair = new VertexPair;
        vpair->v1 = it.first.start;
        vpair->v2 = it.first.end;
        VPairHeap_.push_back(vpair);
    }
    MakeVPairHeap();
}

void MyHEMesh::ContractModel(long facenum) {
    long PrevFaceSize = 0;
    while (FaceSet_.size() > facenum && VPairHeap_.size() > 0) {
        // if (FaceSet_.size() % 500 == 0 && FaceSet_.size() != PrevFaceSize) {
            std::cout << FaceSet_.size() << ' ' << VPairHeap_.size() << '\n';
            PrevFaceSize = FaceSet_.size();
        // }
        if (FaceSet_.size() < 140)
            SaveToOBJ("halsph-debug" + to_string(FaceSet_.size()) + ".obj");
        ContractLeastCostPair();
    }
}

void MyHEMesh::ContractLeastCostPair() {
    VertexPair vpair = *VPairHeap_.front();
    // for (auto it = VPairHeap_.begin(); it < VPairHeap_.begin() + 10; it ++) {
    //     std::cout << (*it)->cost << ' ';
    // }
    // std::cout << std::endl;
    std::pop_heap(VPairHeap_.begin(), VPairHeap_.end(), VPairPtrGreater());
    VPairHeap_.pop_back();
    // if (!vpair.erased) ContractVPair(&vpair);
    if (DelVPairSet_.find(VertexPairKey(vpair.v1, vpair.v2)) == DelVPairSet_.end()
        && DelVPairSet_.find(VertexPairKey(vpair.v2, vpair.v1)) == DelVPairSet_.end())
    ContractVPair(&vpair);
}

void MyHEMesh::ContractVPair(VertexPair* vpair) {
    std::cout << "Contract pair" << vpair->v1->x << ' ' << vpair->v1->y << ' ' << vpair->v1->z
                        << " & " << vpair->v2->x << ' ' << vpair->v2->y << ' ' << vpair->v2->z;

    Vertex* v = new Vertex;
    v->x = vpair->v(0); v->y = vpair->v(1); v->z = vpair->v(2);
    // v->g = 0;

    // std::cout << "Target: " << v->x << ' ' << v->y << ' ' << v->z << '\n';

    vector<VertexPairKey> NeighbVPKVec;
    vector<Vertex*> NeighbVertexVec;
    vector<Face*> NeighbFaceVec;
    Vertex* vstart;
    Vertex* vother;
    if (vpair->v1->isBound && vpair->v2->isBound) {
        vstart = (vpair->v1->h->pair->v == vpair->v2) ? vpair->v2 : vpair->v1;
        vother = (vpair->v1->h->pair->v == vpair->v2) ? vpair->v1 : vpair->v2;
    } else if (vpair->v1->isBound) {
        vstart = vpair->v1;
        vother = vpair->v2;
    } else if (vpair->v2->isBound) {
        vstart = vpair->v2;
        vother = vpair->v1;
    } else {
        vstart = vpair->v1;
        vother = vpair->v2;
    }
    HEdge* h = vstart->h;
    do {
        // std::cout << "Traverse edge" << h->v->x << ' ' << h->v->y << ' ' << h->v->z << " to "
        //           << h->pair->v->x << ' ' << h->pair->v->y << ' ' << h->pair->v->z << std::endl;
        if (h->pair->v == vother || h->pair->v == vstart) {
            // if (h->next == vstart->h || h->next == nullptr) break;
            if (h->next == vstart->h) break;
            NeighbFaceVec.push_back(h->f);
            NeighbVPKVec.push_back(VertexPairKey(h->next->v, h->next->pair->v));
            if (h->next->isBound) {
                NeighbVertexVec.push_back(h->next->pair->v);
                NeighbVPKVec.push_back(VertexPairKey(h->pair->v, h->next->pair->v));
            }
            h = h->next->pair->next;
            continue;
        }
        NeighbVPKVec.push_back(VertexPairKey(h->v, h->pair->v));
        // NeighbVPKVec.push_back(VertexPairKey(h->pair->v, h->v));
        NeighbVertexVec.push_back(h->pair->v);
        if (h->f != nullptr) NeighbFaceVec.push_back(h->f);
        h = h->pair->next;
        // h = (h.v == vpair.v2 && h->pair->v == vpair.v1) || (h.v == vpair.v1 && h->pair->v == vpair.v2)
        //     ? h->next->pair : h->pair->next;
    } while (h != vstart->h && h != nullptr);
    // } while (h != vstart->h && h != nullptr);
    // bool IsEdge = (h == nullptr) ? true : false;
    bool IsEdge = false;

    if (!IsCollapseable(NeighbVertexVec, vpair->v1, vpair->v2)) {
        // std::cout << "find non-manifold" << std::endl;
        return;
    }

    if (HasFoldFace(vpair, NeighbFaceVec)) {
        // std::cout << "find folded" << std::endl;
        return;
    }

    NeighbVPKVec.push_back(VertexPairKey(vpair->v1, vpair->v2));
    if (NeighbVertexVec.front() == NeighbVertexVec.back()) NeighbVertexVec.pop_back();
    for (auto &it : NeighbVPKVec) {
        auto MapIt = HEdgeMap_.find(it);
        if (MapIt->second) delete MapIt->second;
        HEdgeMap_.erase(MapIt);
        MapIt = HEdgeMap_.find(VertexPairKey(it.end, it.start));
        if (MapIt->second) delete MapIt->second;
        HEdgeMap_.erase(MapIt);
    }
    for (auto &VPKit : NeighbVPKVec) {
        DelVPairSet_.insert(VPKit);
    }

    for (auto it : NeighbFaceVec) {
        FaceSet_.erase(it);
        delete it;
    }

    for (auto it = NeighbVertexVec.begin() + 1; it != NeighbVertexVec.end(); ++it) {
        InsertFace(v, *it, *(it - 1));
    }
    if (!IsEdge) {
        InsertFace(v, NeighbVertexVec.front(), NeighbVertexVec.back());
    }
    VertexSet_.erase(vpair->v1); VertexSet_.erase(vpair->v2);
    VertexSet_.insert(v);
    delete vpair->v1; delete vpair->v2;

    for (auto it : NeighbVertexVec) {
        UpdateQMatrix(*it);
    }
    UpdateQMatrix(*v);

    for (long i = NeighbVertexVec.size(); i > 0; --i) {
        UpdateVPairCost(*(VPairHeap_.end() - i));
        std::push_heap(VPairHeap_.begin(), VPairHeap_.end() - i + 1, VPairPtrGreater());
        // for (auto it : VPairHeap_) {
        //     std::cout << it->cost << ' ';
        // }
        // std::cout << std::endl;
    }
}

Vector3d GetNormalVec(Face* f) {
    return (GetPosVec(f->v[2]) - GetPosVec(f->v[1])).cross(GetPosVec(f->v[1]) - GetPosVec(f->v[0]));
}

bool MyHEMesh::HasFoldFace(const VertexPair* vpair, const std::vector<Face*>& NeighbFaceVec) {
    Vector3d newPos = vpair->v(seq(fix<0>, fix<2>));
    for (auto it = NeighbFaceVec.begin(); it != NeighbFaceVec.end(); ++it) {
        Face* fptr = *it;
        Vector3d OriFaceNorm, NewFaceNorm;
        Vertex* vStart;
        char ind1, ind2;
        if (fptr->v[0] == vpair->v1 || fptr->v[0] == vpair->v2) {
            vStart = (fptr->v[0] == vpair->v1) ? vpair->v1 : vpair->v2;
            ind1 = 1; ind2 = 2;
        } else if (fptr->v[1] == vpair->v1 || fptr->v[1] == vpair->v2) {
            vStart = (fptr->v[1] == vpair->v1) ? vpair->v1 : vpair->v2;
            ind1 = 0; ind2 = 2;
        } else if (fptr->v[2] == vpair->v1 || fptr->v[2] == vpair->v2) {
            vStart = (fptr->v[2] == vpair->v1) ? vpair->v1 : vpair->v2;
            ind1 = 0; ind2 = 1;
        }
        OriFaceNorm = (GetPosVec(vStart) - GetPosVec(fptr->v[ind1])).cross(GetPosVec(vStart) - GetPosVec(fptr->v[ind2]));
        NewFaceNorm = (newPos - GetPosVec(fptr->v[ind1])).cross(newPos - GetPosVec(fptr->v[ind2]));
        if (OriFaceNorm.dot(NewFaceNorm) < 0) {
            return true;
        }
    }
    return false;
}

bool MyHEMesh::IsCollapseable(const std::vector<Vertex*>& NeighbVertexVec, const Vertex* v1, const Vertex* v2) {
    for (auto &v : NeighbVertexVec) {
        HEdge* v1v = nullptr;
        HEdge* v2v = nullptr;
        HEdge* v1v2 = nullptr;
        HEdge* h = v->h;
        do {
            if (h->pair->v == v1) v1v = h->pair;
            if (h->pair->v == v2) v2v = h->pair;
            h = h->pair->next;
        } while (h != v->h && h != nullptr);
        if (v1v != nullptr && v2v != nullptr) {
            h = (v1->isBound) ? v1->h : v2->h;
            do {
                if (h->pair->v == v2 || h->pair->v == v1) {
                    v1v2 = h;
                    break;
                }
                h = h->pair->next;
            } while (h != v->h && h != nullptr);
            if (v1v2->isBound) {
                if (v1v2->next != nullptr && v1v2->next->pair->v != v) {
                    return false;
                } else if (v1v2->pair->next != nullptr && v1v2->pair->next->v != v) {
                    return false;
                }
            } else {
                if (v1v2->next->pair->v != v && v1v2->pair->next->pair->v != v) {
                    return false;
                }
            }
        }
    }
    return true;
}

void MyHEMesh::ContractInitModel(long v1index, long v2index) {
    Vertex* v = new Vertex;
    VertexPair vpair;
    vpair.v1 = VertexVec_[v1index];
    vpair.v2 = VertexVec_[v2index];
    UpdateVPairCost(&vpair);
    v->x = vpair.v(0); v->y = vpair.v(1); v->z = vpair.v(2);

    vector<VertexPairKey> NeighbVPKVec;
    vector<Vertex*> NeighbVertexVec;
    vector<Face*> NeighbFaceVec;
    Vertex* vstart = (vpair.v1->h->pair->v == vpair.v2) ? vpair.v2 : vpair.v1;
    Vertex* vother = (vpair.v1->h->pair->v == vpair.v2) ? vpair.v1 : vpair.v2;
    // HEdge* h = (vpair.v1->h->pair->v == vpair.v2) ? vpair.v2->h : vpair.v1->h;
    HEdge* h = vstart->h;
    do {
        // std::cout << "Traverse edge" << h->v->x << ' ' << h->v->y << ' ' << h->v->z << " to "
        //           << h->pair->v->x << ' ' << h->pair->v->y << ' ' << h->pair->v->z << std::endl;
        if (h->pair->v == vother || h->pair->v == vstart) {
            // if (h->next == vstart->h || h->next == nullptr) break;
            if (h->next == vstart->h) break;
            NeighbFaceVec.push_back(h->f);
            NeighbVPKVec.push_back(VertexPairKey(h->next->v, h->next->pair->v));
            h = h->next->pair->next;
            continue;
        }
        NeighbVPKVec.push_back(VertexPairKey(h->v, h->pair->v));
        // NeighbVPKVec.push_back(VertexPairKey(h->pair->v, h->v));
        NeighbVertexVec.push_back(h->pair->v);
        NeighbFaceVec.push_back(h->f);
        h = h->pair->next;
        // h = (h.v == vpair.v2 && h->pair->v == vpair.v1) || (h.v == vpair.v1 && h->pair->v == vpair.v2)
        //     ? h->next->pair : h->pair->next;
    } while (h != vstart->h);
    // } while (h != vstart->h && h != nullptr);
    // bool IsEdge = (h == nullptr) ? true : false;
    bool IsEdge = false;

    if (!IsCollapseable(NeighbVertexVec, vpair.v1, vpair.v2)) {
        // std::cout << "find non-manifold" << std::endl;
        return;
    }

    if (HasFoldFace(&vpair, NeighbFaceVec)) {
        // std::cout << "find folded" << std::endl;
        return;
    }

    for (auto it : NeighbFaceVec) {
        FaceSet_.erase(it);
        delete it;
    }

    for (auto it = NeighbVertexVec.begin() + 1; it != NeighbVertexVec.end(); ++it) {
        InsertFace(v, *it, *(it - 1));
    }
    if (!IsEdge) {
        InsertFace(v, NeighbVertexVec.front(), NeighbVertexVec.back());
    }

    for (auto vptr : NeighbVertexVec) {
        CheckBound(vptr);
    }
    CheckBound(v);
    VertexSet_.erase(vpair.v1); VertexSet_.erase(vpair.v2);
    VertexSet_.insert(v);
    delete vpair.v1; delete vpair.v2;
}

void MyHEMesh::CheckBound(Vertex* v) {
    HEdge* h = v->h;
    HEdge* prev;
    do {
        prev = h;
        h = h->pair->next;
    } while(h != v->h && h != nullptr);

    if (h == nullptr) {
        v->isBound = true;
        prev->isBound = true;
        prev->pair->isBound = true;
        prev = v->h;
        h = v->h->next;
        while (h->next)
        // while (h != nullptr) {
        //     h = h->next->pair;
        //     prev = h;
        //     h = h->next;
        // }
        prev->isBound = true;
        prev->pair->isBound = true;
        v->h = prev;
    }
}