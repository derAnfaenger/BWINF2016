#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;

typedef vector <vector <vector <int> > > VVVI;
typedef vector <vector <int> > VVI;
typedef vector <int> VI;

#define fakeN 1000000000

ifstream fin ("ty.in");
ofstream fout ("ty.out");
ofstream graph ("graph.gv");

vector <map <int, int> > nodeState, nodeStateInit, //erste Stufe: Knoten: Richtung, Integer = Zustand (0 = unbesucht, 1 = besucht)
			 isSicher, isUnsicher; // 0 = unbesucht, 1 = unsicher, 2 = sicher
vector <map <int, VI> > AdjL, AdjLG; //1. Knoten, 2. verbundene Knoten (Richtung, wohin will ich danach), 3. wenn ich zu diesem Knoten gehen will, von welchen kann ich kommen?
vector <bool> isExit; //Ist Knoten x ein Ausgang?
VI sicherN;
VVI unsicherN;
int nSize; //Anzahl der Knoten

//dsf-status: 0 - nicht gefunden, 1 - gefunden
int dfs (int node, int d, int state) { // Herausfinden, von welchen Feldern aus man Knoten node erreichen kann. d = von welchem Knoten komme ich
    nodeState[node][d] = 1; //siehe Status nodeState
    if (state == 1) {
        isSicher[node][d] = 1;
    }
    for (int i = 0; i < AdjL[d][node].size(); ++i) {
        int formerN = AdjL[d][node][i];
        if (nodeState[d][formerN] == 0 && isExit[d] == false) {
            dfs (d, formerN, state);
        }
    }
    return 0;
}

int main()
{
    //Eingabe
    fin >> nSize;
    AdjL.resize (nSize);
    AdjLG.resize(nSize);
    nodeStateInit.resize (nSize);
    isExit.resize (nSize);
    for (int i = 0; i < nSize; ++i) {
        int cons, isE; //connections, isExit
        fin >> isE >> cons;
        isExit[i] = isE;
        if (isE == 1) {
            sicherN.push_back (i);
        }
        for (int j = 0; j < cons; ++j) {
            int rSize, tile; //number of reachable nodes when coming from node tile, state of
            fin >> tile >> rSize;
            vector <int> reachableV (rSize);
            for (int k = 0; k < rSize; ++k) {
                int reachable;
                fin >> reachable;
                AdjL[i][reachable].push_back (tile);
                reachableV[k] = reachable;
            }
            AdjLG[i].insert (make_pair(tile, reachableV));
            if (rSize == 0 && isE == 1) { //ist der Knoten ein Ausgang, sollen auch eingehende Kanten ohne weiterführende Verbindung durchlaufen werden
                AdjL[i][fakeN].push_back (tile);
            }
            nodeStateInit[i].insert (make_pair (tile, 0));
        }
    }
    isSicher = nodeStateInit;
    nodeState = nodeStateInit;
    for (int i = 0; i < isExit.size(); ++i) {
        if (isExit[i] == true) {
            for (auto it = isSicher[i].begin(); it != isSicher[i].end(); ++it) {
                (*it).second = 2;
            }
        }
    }
    //Für jeden Knoten und jede Richtung testen, ob sie sicher ist.
        for (int i = 0; i < sicherN.size(); ++i) {
            int uN = sicherN[i];
            for (auto d = AdjL[uN].begin(); d != AdjL[uN].end(); ++d) {
                for (int j = 0; j < (*d).second.size(); ++j) {
                    dfs (uN, (*d).second[j], 0);
                }
            }
        }
    for (int i = 0; i < nodeState.size(); ++i) {
        for (auto d = nodeState[i].begin(); d != nodeState[i].end(); ++d) {
            int state = (*d).second;
            int dir = (*d).first;
            if (state == 0) {
                unsicherN.push_back (vector <int> {i, dir});
                isSicher[i][dir] = 1;
            }
        }
    }
    isUnsicher = isSicher;
    nodeState = nodeStateInit;
    for (int i = 0; i < unsicherN.size(); ++i) {
        dfs (unsicherN[i][0], unsicherN[i][1], 1);
    }
    //Ausgabe
    for (int i = 0; i < nSize; ++i) {
        if (isSicher[i][i] != 1){
            fout << i << endl;
        }
    }
    graph << "digraph yamyam {" << endl;
    for (int i = 0; i < AdjLG.size(); ++i) {
        for (auto it = AdjLG[i].begin(); it != AdjLG[i].end(); ++it) {
                int con = (*it).first;
                graph << con << " -> " << i << " [label = \"";
                for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2) {
                    graph << "\\n" << *it2;
                }
                graph << "\"];" << endl;
        }
        bool unsicher = true;
        for (auto d = isUnsicher[i].begin(); d != isUnsicher[i].end(); ++d) {
            unsicher = (*d).second == 1 ? unsicher : false;
        }
        graph << i << "[label = \"" << i << "\"";
        if (isExit[i] == true) {
            graph << " color=green style = filled";
        }
        else if (isSicher[i][i] != 1) {
            graph << " color=yellow style = filled";
        } else if (unsicher == true) {
            graph << " color=red style = filled";
        } else {
            graph << " color=orange style = filled";
        }
        graph << "];" << endl;
    }
    graph << "}" << endl;
}
