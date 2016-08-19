#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

typedef vector <vector <vector <int> > > VVVI;
typedef vector <vector <int> > VVI;
typedef vector <int> VI;

ifstream fin ("ty.in");
ofstream fout ("ty.out");

VVVI nodeState; //erste Stufe: y, zweite Stufe: x,
                //dritte Stufe: Richtung,
                //Integer = Zustand (0 = unbesucht, 1 = besucht)
VVVI isSicher; //0 = unbesucht, 1 = unsicher, 2 = sicher;
VVI sicherN; //sichere Subknoten
VVI unsicherN; //unsichere Subknoten

VVVI unsicherM; //0 = nein, 1 = ja
VVI AdjM; //0 = frei, 1 = Mauer,
        //2 = Ausgang, 3 = unfrei, 4 = sicher
VVI dirs {{0, 1},{1, 0},{0, -1},{-1, 0}}; // y,x
        //Richtungen in die der Yamyam gehen kann

int width, height; // Höhe und Breite des Feldes

//dsf: findet alle Subknoten, die Subknoten yxd besuchen können
int dfs (int y, int x, int d, int state, int depth) {
  //  cout << depth << endl;
    nodeState[y][x][d] = 1; //siehe Status nodeState
    if (state == 1) {
        isSicher[y][x][d] = 1;
    }
    int d1 = (d-1) < 0 ? 3 : (d-1); //Die Richtungen, in die man gehen müsste, um potentiell yxd zu erreichen
    int d2 = (d+1) > 3 ? 0 : (d+1);
    int d3 = (d+2) > 3 ? (d-2) : (d+2);
    int nextX = x-dirs[d][1]; //Eins der Felder, von denen man potentiell yxd erreichen könnte
    int nextY = y-dirs[d][0];
    int nextX1 = x-dirs[d1][1];  //auch die Richtung im Array davor untersuchen
    int nextY1 = y-dirs[d1][0];
    int nextX2 = x-dirs[d2][1]; //und von danach
    int nextY2 = y-dirs[d2][0];
    if (AdjM[nextY][nextX] == 0 && nodeState[nextY][nextX][d] == 0) { //wenn das Yamyam von dort kommen kann, untersuche das Feld
        dfs (nextY, nextX, d, state, depth+1);
    }
    if (AdjM[nextY][nextX] == 1 && nodeState[y][x][d3] == 0) { //oder von dort
        dfs (y, x, d3, state, depth+1);
    }
    if (AdjM[nextY1][nextX1] == 0 && AdjM[nextY2][nextX2] == 1
        && nodeState[nextY1][nextX1][d1] == 0) { //wenn das Yamyam von dort kommen kann, untersuche das Feld
        dfs (nextY1, nextX1, d1, state, depth+1);
    }
    if (AdjM[nextY2][nextX2] == 0 && AdjM[nextY1][nextX1] == 1
        && nodeState[nextY2][nextX2][d2] == 0) { //wenn das Yamyam von dort kommen kann, untersuche das Feld
        dfs (nextY2, nextX2, d2, state, depth+1);
    }
    return 0;
}

int main()
{
    //Eingabe
    fin >> width >> height;
    AdjM.resize (height, VI (width));
    unsicherM.resize (height, VVI (width, VI (4, 0)));
    nodeState.resize (height, VVI (width, VI (4, 0)));
    isSicher.resize (height, VVI (width, VI (4, 0)));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            fin >> AdjM[i][j];
            if (AdjM[i][j] == 2) { //sichere Felder markieren
                sicherN.push_back (vector <int> {i, j});
                for (int d = 0; d < 4; ++d)
                    isSicher[i][j][d] = 2;
            }
        }
    }
    cout << "initialisiert!" << endl;
    //Felder markieren, von denen man nie ein sicheres Feld erreichen wird.
    for (int i = 0; i < sicherN.size(); ++i) {
        for (int d = 0; d < 4; ++d) {
            //alle nicht unsicheren Subknoten markieren
            dfs (sicherN[i][0], sicherN[i][1], d, 0, 0);
        }
    }
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            for (int d = 0; d < 4; ++d) {
                if (nodeState[i][j][d] == 0 && AdjM[i][j] == 0 && AdjM[i+dirs[d][0]][j+dirs[d][1]] != 1) {
                    //aus Ergebniss der rDFS alle unsicheren Subknoten ermitteln
                    unsicherN.push_back (vector <int> {i, j, d});
                    unsicherM[i][j][d] = 1;
                    isSicher[i][j][d] = 1;
                }
            }
        }
    }
    nodeState.clear();
    nodeState.resize (height, VVI (width, VI (4, 0)));
    //Felder markieren, von denen man 100%-ig ein sicheres Feld erreichen wird.
    for (int i = 0; i < unsicherN.size(); ++i) {
        for (int d = 0; d < 4; ++d) {
            dfs (unsicherN[i][0], unsicherN[i][1], unsicherN[i][2], 1, 0); //markiere alle als sicher
        }
    }
    //Ausgabe
    //0 = Wand, 1 = unsicher, 2 = fraglich, 3 = sicher, 4 = ausgang, 5 = würde zu Wand torkeln, nicht möglich
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (AdjM[i][j] == 1) { //Wenn Feld Wand, dann so ausgeben
                fout << "0 0 0 0 ";
            } else if (AdjM[i][j] == 2) { //Wenn Feld Ausgang, dann so ausgeben
                fout << "4 4 4 4 ";
            } else {
                for (int d = 0; d < 4; ++d) {
                    if (AdjM[i+dirs[d][0]][j+dirs[d][1]] == 1) { //Liegt das Feld an einer Wand, soll der dahin verbundene Subknoten als nicht eindeutig markiert werden
                        fout << "5 ";
                        continue;
                    }
                    if (isSicher[i][j][d] == 0) { //Wenn ein Subknoten nicht als unsicher markiert wurde, soll er als sicher gehandhabt werden
                        isSicher[i][j][d] = 2;
                    }
                    if (unsicherM[i][j][d] == 1) {
                        fout << "1 ";
                    } else {
                        fout << (isSicher[i][j][d] == 2 ? '3' : '2') << ' ';
                    }
                }
            }
            fout << ' ';
        }
        fout << endl;
    }
    //char c;
    //cin >> c;
}
