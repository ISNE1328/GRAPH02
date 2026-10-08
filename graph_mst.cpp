#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <utility>
#include <algorithm>
#include <stdexcept>

using namespace std;

class Node {
public:
    string name;

    Node(const string& n) : name(n) {}
};

class Edge {
public:
    int source;
    int target;
    int weight;
    bool directed;

    Edge(int s, int t, int w, bool d)
        : source(s), target(t), weight(w), directed(d) {}

    pair<int, int> key() const {
        if (directed)
            return make_pair(source, target);
        // undirected: A-B กับ B-A ถือเป็นคู่เดียวกัน
        return make_pair(min(source, target), max(source, target));
    }
};

bool edgeLess(const Edge& a, const Edge& b) {
    return a.weight < b.weight;
}

class UnionFind {
private:
    vector<int> parent;
    vector<int> rnk;

public:
    UnionFind(int n) : parent(n), rnk(n, 0) {
        for (int i = 0; i < n; i++) parent[i] = i;
    }

    int find(int x) {
        if (parent[x] != x)
            parent[x] = find(parent[x]);
        return parent[x];
    }

    bool unite(int a, int b) {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rnk[ra] < rnk[rb]) swap(ra, rb);
        parent[rb] = ra;
        if (rnk[ra] == rnk[rb]) rnk[ra]++;
        return true;
    }
};

class Graph {
private:
    vector<Node> nodes;
    vector<Edge> edges;
    bool directed;

    int indexOf(const string& name) const {
        for (size_t i = 0; i < nodes.size(); i++)
            if (nodes[i].name == name) return (int)i;
        throw invalid_argument("Node not found: " + name);
    }

public:
    Graph(const vector<vector<int> >& matrix) : directed(false) {
        int n = matrix.size();

        for (int i = 0; i < n; i++)
            if ((int)matrix[i].size() != n)
                throw invalid_argument("Adjacency matrix must be square (n x n)");

        for (int i = 0; i < n; i++)
            nodes.push_back(Node(string(1, (char)('A' + i))));

        for (int i = 0; i < n && !directed; i++)
            for (int j = 0; j < n; j++)
                if (matrix[i][j] != matrix[j][i]) {
                    directed = true;
                    break;
                }

        for (int i = 0; i < n; i++) {
            int start = directed ? 0 : i;
            for (int j = start; j < n; j++) {
                if (matrix[i][j] != 0)
                    edges.push_back(Edge(i, j, matrix[i][j], directed));
            }
        }
    }

    void addEdge(const string& src, const string& dst, int weight = 1) {
        edges.push_back(Edge(indexOf(src), indexOf(dst), weight, directed));
    }

    bool isMultigraph() const {
        set<pair<int, int> > seen;
        for (size_t k = 0; k < edges.size(); k++) {
            pair<int, int> key = edges[k].key();
            if (seen.count(key)) return true;
            seen.insert(key);
        }
        return false;
    }

    bool isPseudograph() const {
        for (size_t k = 0; k < edges.size(); k++)
            if (edges[k].source == edges[k].target) return true;
        return false;
    }

    bool isDigraph() const {
        return directed;
    }

    bool isWeighted() const {
        for (size_t k = 0; k < edges.size(); k++)
            if (edges[k].weight != 1) return true;
        return false;
    }

    bool isComplete() const {
        set<pair<int, int> > pairs;
        for (size_t k = 0; k < edges.size(); k++) {
            pairs.insert(make_pair(edges[k].source, edges[k].target));
            if (!directed)
                pairs.insert(make_pair(edges[k].target, edges[k].source));
        }
        int n = nodes.size();
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++)
                if (u != v && !pairs.count(make_pair(u, v)))
                    return false;
        return true;
    }

    vector<string> isolatedNodes() const {
        vector<bool> connected(nodes.size(), false);
        for (size_t k = 0; k < edges.size(); k++) {
            if (edges[k].source != edges[k].target) {
                connected[edges[k].source] = true;
                connected[edges[k].target] = true;
            }
        }
        vector<string> result;
        for (size_t i = 0; i < nodes.size(); i++)
            if (!connected[i]) result.push_back(nodes[i].name);
        return result;
    }

    bool isDisjointed() const {
        return !isolatedNodes().empty();
    }

    vector<Edge> kruskalMST(int& totalWeight) const {
        if (directed)
            throw logic_error("MST is defined for undirected graphs only");

        int n = nodes.size();
        totalWeight = 0;

        vector<Edge> sorted = edges;
        stable_sort(sorted.begin(), sorted.end(), edgeLess);

        UnionFind uf(n);

        vector<Edge> mst;
        for (size_t k = 0; k < sorted.size(); k++) {
            const Edge& e = sorted[k];
            if (uf.unite(e.source, e.target)) {
                mst.push_back(e);
                totalWeight += e.weight;
                if ((int)mst.size() == n - 1) break;
            }
        }

        if (n > 0 && (int)mst.size() != n - 1)
            throw logic_error("Graph is not connected: no spanning tree exists");

        return mst;
    }

    Graph minimumSpanningTree() const {
        int total = 0;
        vector<Edge> mst = kruskalMST(total);

        int n = nodes.size();
        vector<vector<int> > m(n, vector<int>(n, 0));
        for (size_t k = 0; k < mst.size(); k++) {
            m[mst[k].source][mst[k].target] = mst[k].weight;
            m[mst[k].target][mst[k].source] = mst[k].weight;
        }
        return Graph(m);
    }

    void report() const {
        cout << boolalpha;

        cout << "Nodes (" << nodes.size() << "): ";
        for (size_t i = 0; i < nodes.size(); i++)
            cout << nodes[i].name << " ";
        cout << "\n";

        cout << "Edges (" << edges.size() << "):\n";
        for (size_t k = 0; k < edges.size(); k++) {
            const Edge& e = edges[k];
            cout << "  " << nodes[e.source].name
                 << (e.directed ? " -> " : " -- ")
                 << nodes[e.target].name
                 << "  (w=" << e.weight << ")\n";
        }

        cout << endl << "Multigraph  : " << isMultigraph()  << "\n";
        cout << "Pseudograph : " << isPseudograph() << "\n";
        cout << "Digraph     : " << isDigraph()     << "\n";
        cout << "Weighted    : " << isWeighted()    << "\n";
        cout << "Complete    : " << isComplete()    << "\n";
        cout << "Disjointed  : " << isDisjointed()  << "  (isolated: ";
        vector<string> iso = isolatedNodes();
        for (size_t i = 0; i < iso.size(); i++) cout << iso[i] << " ";
        cout << ")\n";
    }

    void printMST() const {
        int total = 0;
        vector<Edge> mst = kruskalMST(total);

        cout << "MST edges (" << mst.size() << "):\n";
        for (size_t k = 0; k < mst.size(); k++)
            cout << "  " << nodes[mst[k].source].name << " -- "
                 << nodes[mst[k].target].name
                 << "  (w=" << mst[k].weight << ")\n";
        cout << "Total weight = " << total << "\n";
    }
};

int main() {
    vector<vector<int> > matrix = {
        {0,  2,  1,  7,  0,  0,  0,  0,  0},
        {2,  0,  5,  5,  0,  0,  0,  0,  0},
        {1,  5,  0,  4,  0,  9,  0,  0,  0},
        {7,  5,  4,  0,  8,  0,  0,  0,  0},
        {0,  0,  0,  8,  0,  3,  7,  0,  0},
        {0,  0,  9,  0,  3,  0,  5, 10,  0},
        {0,  0,  0,  0,  7,  5,  0, 11,  6},
        {0,  0,  0,  0,  0, 10, 11,  0,  3},
        {0,  0,  0,  0,  0,  0,  6,  3,  0}
    };

    cout << "Sample graph\n";
    Graph g(matrix);
    g.report();

    cout << "\nPart 2: Minimum Spanning Tree (Kruskal)\n";
    g.printMST();

    cout << "\nMST as a new Graph\n";
    Graph mst = g.minimumSpanningTree();
    mst.report();

    cout << boolalpha;

    cout << "\nTest: parallel edge + loop\n";
    Graph g2(matrix);
    g2.addEdge("A", "B", 9);
    g2.addEdge("C", "C", 1);
    cout << "Multigraph  : " << g2.isMultigraph()  << "\n";
    cout << "Pseudograph : " << g2.isPseudograph() << "\n";
    g2.printMST();

    cout << "\nTest: directed\n";
    Graph g3({{0, 1, 0},
              {0, 0, 1},
              {1, 0, 0}});
    cout << "Digraph     : " << g3.isDigraph() << "\n";
    try {
        g3.printMST();
    } catch (const logic_error& e) {
        cout << "Error: " << e.what() << "\n";
    }

    cout << "\nTest: complete K3\n";
    Graph g4({{0, 1, 1},
              {1, 0, 1},
              {1, 1, 0}});
    cout << "Complete    : " << g4.isComplete() << "\n";
    cout << "Weighted    : " << g4.isWeighted() << "\n";
    g4.printMST();

    cout << "\nTest: isolated node\n";
    Graph g5({{0, 2, 0},
              {2, 0, 0},
              {0, 0, 0}});
    cout << "Disjointed  : " << g5.isDisjointed() << "\n";
    try {
        g5.printMST();
    } catch (const logic_error& e) {
        cout << "Error: " << e.what() << "\n";
    }

    return 0;
}