// Implement member functions for SimplicialComplexOperators class.
#include "simplicial-complex-operators.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;

/*
 * Assign a unique index to each vertex, edge, and face of a mesh.
 * All elements are 0-indexed.
 *
 * Input: None. Access geometry via the member variable <geometry>, and pointer to the mesh via <mesh>.
 * Returns: None.
 */
void SimplicialComplexOperators::assignElementIndices() {

    // Needed to access geometry->vertexIndices, etc. as cached quantities.
    // Not needed if you're just using v->getIndex(), etc.
    geometry->requireVertexIndices();
    geometry->requireEdgeIndices();
    geometry->requireFaceIndices();


    // You can set the index field of a vertex via geometry->vertexIndices[v], where v is a Vertex object (or an
    // integer). Similarly you can do edges and faces via geometry->edgeIndices, geometry->faceIndices, like so:
    size_t idx = 0;
    for (Vertex v : mesh->vertices()) {
        idx = geometry->vertexIndices[v];
        idx++;
    }
    idx= 0;

    for (Edge e : mesh->edges()) {
        idx = geometry->edgeIndices[e];
        idx++;
    }
    idx = 0;

    for (Face f : mesh->faces()) {
        idx = geometry->faceIndices[f];
        idx++;
    }

    // You can more easily get the indices of mesh elements using the function getIndex(), albeit less efficiently and
    // technically less safe (although you don't need to worry about it), like so:
    //
    //      v.getIndex()
    //
    // where v can be a Vertex, Edge, Face, Halfedge, etc. For example:
    idx = 0;
    for (Vertex v : mesh->vertices()) {
        idx = v.getIndex(); // == geometry->vertexIndices[v])
        idx++;
    }

    // Geometry Central already sets the indices for us, though, so this function is just here for demonstration.
    // You don't have to do anything :)
}

/*
 * Construct the unsigned vertex-edge adjacency matrix A0.
 *
 * Input:
 * Returns: The sparse vertex-edge adjacency matrix which gets stored in the global variable A0.
 */
SparseMatrix<size_t> SimplicialComplexOperators::buildVertexEdgeAdjacencyMatrix() const {

    // TODO
    // Note: You can build an Eigen sparse matrix from triplets, then return it as a Geometry Central SparseMatrix.
    // See <https://eigen.tuxfamily.org/dox/group__TutorialSparse.html> for documentation.
    int rows =mesh->nEdges();
    int cols = mesh->nVertices();
    SparseMatrix<size_t> vertexEdgeMatrix(rows,cols);
    for (auto edge : mesh->edges()) {
		int eidx = edge.getIndex();
        int  v1idx = edge.firstVertex().getIndex();
        int v2idx = edge.secondVertex().getIndex();

        vertexEdgeMatrix.insert(eidx,v1idx) = 1;
        vertexEdgeMatrix.insert(eidx,v2idx) = 1;

	}
    return vertexEdgeMatrix; // placeholder
}

/*
 * Construct the unsigned face-edge adjacency matrix A1.
 *
 * Input:
 * Returns: The sparse face-edge adjacency matrix which gets stored in the global variable A1.
 */
SparseMatrix<size_t> SimplicialComplexOperators::buildFaceEdgeAdjacencyMatrix() const {

    // TODO
    int rows = mesh->nFaces();
    int cols = mesh->nEdges();
    SparseMatrix<size_t> edgeFaceMatrix(rows,cols);
    for (auto face : mesh->faces()) {
        int fidx = face.getIndex();
        for (auto edge : face.adjacentEdges()) {
            int eidx = edge.getIndex();
            edgeFaceMatrix.insert(fidx,eidx) = 1;
        }
    }

    return edgeFaceMatrix; // placeholder
}

/*
 * Construct a vector encoding the vertices in the selected subset of simplices.
 *
 * we need to find if what vertices of this subset 
 * Input: Selected subset buildVertexVector simplices.
 * Returns: Vector of length |V|, where |V| = # of vertices in the mesh.
 */
Vector<size_t> SimplicialComplexOperators::buildVertexVector(const MeshSubset& subset) const {

    Vector<size_t> v(mesh->nVertices());
    for (int i = 0; i < mesh->nVertices(); i++) {
        if (subset.vertices.count(i)) {
            v[i] = 1;
        } else {
            v[i] = 0;
        }
    }
    return v;
    
}

/*
 * Construct a vector encoding the edges in the selected subset of simplices.
 *
 * Input: Selected subset of simplices.
 * Returns: Vector of length |E|, where |E| = # of edges in mesh.
 */
Vector<size_t> SimplicialComplexOperators::buildEdgeVector(const MeshSubset& subset) const {

    // TODO

    Vector<size_t> edgeVectors(mesh->nEdges());
    for (int i = 0; i < mesh->nEdges(); i++) {
        if (subset.edges.count(i)) {
            edgeVectors[i] = 1;
        } else {
            edgeVectors[i] = 0;
        }
    }
    return edgeVectors;
}

/*
 * Construct a vector encoding the faces in the selected subset of simplices.
 *
 * Input: Selected subset of simplices.
 * Returns: Vector of length |F|, where |F| = # of faces in mesh.
 */
Vector<size_t> SimplicialComplexOperators::buildFaceVector(const MeshSubset& subset) const {
    Vector<size_t> faceVectors(mesh->nFaces());
    for (int i = 0; i < mesh->nFaces(); i++) {
        if (subset.edges.count(i)) {
            faceVectors[i] = 1;
        } else {
            faceVectors[i] = 0;
        }
    }
    return faceVectors;
}

/*
 * Compute the simplicial star St(S) of the selected subset of simplices.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: The star of the given subset.
 */
MeshSubset SimplicialComplexOperators::star(const MeshSubset& subset) const {
    
    
    //current set is already part of star
    MeshSubset starSet(subset);
     
    //outerSize --> columns
    //innerSize --> rows

    int cols = A0.outerSize();
    int rows = A0.innerSize();
    for (auto vertex : subset.vertices) {
        
        /*starSet.addVertex(vertex);*/
        for (int e = 0; e < mesh->nEdges(); e++) {
            if (A0.coeff(e, vertex)) {
                starSet.addEdge(e);
            }
        }
        
    }

    for (auto edge : starSet.edges) {
        for (int f = 0; f < mesh->nFaces(); f++) {
            if (A1.coeff(f, edge)) {
                starSet.addFace(f);
            }
        }
    }
    // TODO
    return starSet; // placeholder
}


/*
 * Compute the closure Cl(S) of the selected subset of simplices.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: The closure of the given subset.
 */
MeshSubset SimplicialComplexOperators::closure(const MeshSubset& subset) const {
    MeshSubset closureSet(subset);
    for (auto face : subset.faces) {
        for (int i = 0; i < mesh->nEdges(); i++) {
            if (A1.coeff(face, i)) {
                closureSet.addEdge(i);
            }
        }
    }
    for (auto edge : closureSet.edges) {
        for (int i = 0; i < mesh->nVertices(); i++) {
            if (A0.coeff(edge, i)) {
                closureSet.addVertex(i);
            }
        }
    }

    // TODO
    return closureSet; // placeholder
}

/*
 * Compute the link Lk(S) of the selected subset of simplices.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: The link of the given subset.
 */
MeshSubset SimplicialComplexOperators::link(const MeshSubset& subset) const {

    // TODO
    return subset; // placeholder
}

/*
 * Return true if the selected subset is a simplicial complex, false otherwise.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: True if given subset is a simplicial complex, false otherwise.
 */
bool SimplicialComplexOperators::isComplex(const MeshSubset& subset) const {

    // TODO
    return false; // placeholder
}

/*
 * Check if the given subset S is a pure simplicial complex. If so, return the degree of the complex. Otherwise, return
 * -1.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: int representing the degree of the given complex (-1 if not pure)
 */
int SimplicialComplexOperators::isPureComplex(const MeshSubset& subset) const {

    // TODO
    return -1; // placeholder
}

/*
 * Compute the set of simplices contained in the boundary bd(S) of the selected subset S of simplices.
 *
 * Input: A MeshSubset object containing the indices of the currently active vertices, edges, and faces, respectively.
 * Returns: The boundary of the given subset.
 */
MeshSubset SimplicialComplexOperators::boundary(const MeshSubset& subset) const {

    // TODO
    return subset; // placeholder
}