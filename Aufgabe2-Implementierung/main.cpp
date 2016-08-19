#include <iostream>
#include <string>
#include <fstream>
#include "simple_svg_1.0.0.hpp"
#include <sstream>
#include <utility>
#include <vector>
#include <queue>
#include <list>
#include <bitset>
#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <time.h>

using namespace std;
using namespace svg;

#define PII pair<int, int>

vector <vector <PII> > fields; //1,2 indizes; PII = wo will dieses Paket hin?
vector <vector < vector <PII> > > stepsV; //alle Schritte
vector <vector < vector <pair <unsigned char,unsigned char> > > > dp; // 1, 2: indizes, 3: welche reihe, 4: die felderkombination
vector <vector <string> > result; //1,2 indizes; string = Befehlsabfolge für die werfende Person an diser Stelle
list <vector <pair <int, int> > > displaced; //Die verrückten Packete
vector <vector <int> > isAdded; //Wird das Packet gerade verwendet?
ifstream fieldIn ("siedlung.txt");
int wLength ,
    tLength = 20;
int steps = 0;
int elC;
int ctr;
Dimensions dimensions(wLength*tLength, wLength*(tLength+10)*steps);
Document doc("result.svg", Layout(dimensions, Layout::TopLeft));

void paintArrow (Point a, Point b) {
    Stroke s = Stroke (1, Color::Orange);
    doc << Line (a, b, s);
    doc << Line (b, Point (b.x+(a.y-b.y)/3+(a.x-b.x)/3, b.y+(a.x-b.x)/3+(a.y-b.y)/3), s);
    doc << Line (b, Point (b.x-(a.y-b.y)/3+(a.x-b.x)/3, b.y-(a.x-b.x)/3+(a.y-b.y)/3), s);
}

bool improved () { //Testet, ob das DP eine Verbesserung gebracht hat.
    for (int i = 0; i < elC; ++i) {
            if (dp.back().back()[i].first > 8) {
                return true;
            }
    }
    return false;
}

bool isFinished () {
    for (int i = 0; i < wLength; ++i) {
        for (int j = 0; j < wLength; ++j) {
            if (fields[i][j].first != i || fields[i][j].second != j)
                return false;
        }
    }
    return true;
}

int mDist (pair <int, int> a, pair <int, int> b) {
    return abs(a.first-b.first) + abs(a.second-b.second);
}

pair <int, int> getPrev (int r, int c) { //Liefert den Vorfolger von [r, c] zurück
    return make_pair ((c==0?r-1:r), (c==0?wLength-1:c-1));
}

int getIndex (int num) { //liefert einen Zähler zurück, der über alle Kombinationen bis zu diesem hin geht
    return 1<<(num+1);
}

bool isZero (int idx, int num) { //Testet, ob das Feld num in idx 0 ist
    return !((idx >> num) & 1);
}

void moveField () { //Wirft die Packete nach dem durch DP vorgerechneten Plan
    int conditionMask = elC;
    vector <vector <bool> > isHandled (wLength, vector <bool> (wLength, false));
    for (int r = wLength-1; r >= 0; --r) {
        for (int c = wLength-1; c >= 0; --c) {
            int vMax = 0;
            int idxMax = 0;
            int elsToCheck = getIndex(r > 0 ? wLength+c : c);
            for (int comb = 0; comb < elsToCheck; ++comb) { //Die beste Kombination finden
                int combToUse = comb & conditionMask; // Unter der Vorbedingung, dass bestimmte Felder ungebunden sind
                int vNow = dp[r][c][combToUse].first;
                if (vNow > vMax) {
                    vMax = vNow;
                    idxMax = combToUse;
                }
            }
            if (isHandled[r][c] == false) {
                switch (dp[r][c][idxMax].second) { //Abhängig vom besten Wert die Swaps ausführen
                case 0: conditionMask = elC; result[r][c] += '_'; break;
                case 1: swap (fields[r][c-1], fields[r][c]);
                    conditionMask = ~(elC & (1 << c));  result[r][c] += 'W';
                    result[r][c-1] += 'O'; isHandled[r][c-1] = true; break;
                case 2: swap (fields[r-1][c], fields[r][c]);
                    conditionMask  = ~(elC & (1 << (r+c-1)));
                    result[r][c] += 'N'; result[r-1][c] += 'S'; isHandled[r-1][c] = true; break;
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    string resFileName = "result";
    srand(time(NULL));
    int randomFac = 0;
    if (argc > 1) {
        sscanf (argv[1], "%i", &randomFac);
        resFileName = argv[2];
    }
    doc = Document(resFileName+".svg", Layout(dimensions, Layout::TopLeft));
    //Eingabe Beginn
    fieldIn >> wLength;

    fields.resize (wLength, vector <PII>(wLength));
    result.resize (wLength, vector <string> (wLength));
    elC = getIndex (2*wLength)-1;

    for (int i = 0; i < wLength*wLength; ++i) {
        int col, row, colD, rowD;
        fieldIn >> row >> col >> rowD >> colD;
        fields[row][col] = make_pair (rowD, colD);
    }
    if (isFinished() == true) {
        stepsV.push_back(fields);
        goto alreadyFinished;
    }
    //DP Beginn
    dp.resize (wLength, vector <vector <pair <unsigned char,unsigned char> > > (wLength, vector <pair <unsigned char,unsigned char> > (elC, make_pair (0,0)))); //Neu für jede neue Zeiteinheit
    do {
        if (steps > 0) {
            moveField();
        }
        stepsV.push_back(fields);
        ++steps;
        for (int i = 0; i < wLength; ++i) {
            for (int j = 0; j < wLength; ++j) {
                fill(dp[i][j].begin(), dp[i][j].end(), make_pair (0,0));
            }
        }

        for (int r = 0; r < wLength; ++r) {
            for (int c = 0; c < wLength; ++c) {
                if (r == 0 && c == 0)   continue; //Das erste Feld unbeachtet lassen, wurde schon initialisiert
                pair <int, int> prevP = getPrev (r, c), //Das vorherige Packet
                                prevV = fields[prevP.first][prevP.second], //Das Ziel des vorherigen Packets
                                nowV = fields[r][c]; //Das Ziel des jetzigen Packets
                int elsToCheck = getIndex(r > 0 ? wLength+c : c); //Alle Bindungsmöglichkeiten mit dem aktuellen Packet berechnen

                for (int comb = 0; comb < elsToCheck; ++comb) {
                    int offset = r > 0 ? wLength : 0;
                    if (c == 0) {
                        int combPrev = (comb & ~(1 << (wLength))); //Die vorherige Kombination äquivalent zu dieser Kombination
                        if (r>1)
                             combPrev <<= wLength;
                        int maxVAll = 0;
                        for (int cToChk = combPrev; cToChk < (combPrev+ (r > 1 ? getIndex(wLength) : 1));
                        ++cToChk) {
                            int bindingNothing = dp[prevP.first][prevP.second].at(cToChk).first;//Der Wert des vorherigen Feldes zu dieser Kombination
                            int maxV = bindingNothing;
                            int predecessor = 0; //0 = unbesetzt, 1 = bund mit links, 2 = bund mit oben
                            int oldMax = maxV;
                            if (isZero(comb, offset) == false) { //wenn das eigene Feld besetzt werden darf
                                if (r > 0 && isZero (comb, c) == false) { //wenn das oben angrenzende Feld besetzt werden darf
                                    pair <int, int> upV = fields[r-1][c];
                                    int bindingUp = (upV.first >= r ? 1 : -1) + (nowV.first <= (r-1) ? 1 : -1);
                                    maxV = max (maxV, bindingUp + dp[r-1][c][cToChk & ~(1 << (wLength))].first);
                                    predecessor = oldMax == maxV ? predecessor : 2;
                                }
                            }
                            if (maxV > maxVAll) {
                                dp[r][c][comb] = make_pair (maxV, predecessor);
                                maxVAll = maxV;
                            }
                        }

                    } else {
                        int combPrev = comb & ~(1 << (c+offset)); //Die vorherige Kombination äquivalent zu dieser Kombination
                        int bindingNothing = dp[prevP.first][prevP.second][combPrev].first;//Der Wert des vorherigen Feldes zu dieser Kombination

                        int maxV = bindingNothing;
                        int predecessor = 0; //0 = unbesetzt, 1 = bund mit links, 2 = bund mit oben
                        int oldMax = maxV;
                        if (isZero(comb, c+offset) == false) { //wenn das eigene Feld besetzt werden darf
                            if (c > 0 && isZero (comb, c-1+offset) == false) { //wenn das links angrenzende Feld besetzt werden darf
                                int bindingLeft = (prevV.second >= c ? 1 : -1) + (nowV.second <= prevP.second ? 1 : -1);
                                maxV = max (maxV, bindingLeft + dp[r][c-1][combPrev & ~(1 << (c-1+offset))].first);
                                predecessor = oldMax == maxV ? predecessor : 1;
                                oldMax = maxV;
                            }
                            if (r > 0 && isZero (comb, c) == false) { //wenn das oben angrenzende Feld besetzt werden darf
                                pair <int, int> upV = fields[r-1][c];
                                int bindingUp = (upV.first >= r ? 1 : -1) + (nowV.first <= (r-1) ? 1 : -1);
                                maxV = max (maxV, bindingUp + dp[r-1][c][combPrev & ~(1 << (c))].first);
                                predecessor = oldMax == maxV ? predecessor : 2;
                            }
                        }
                        dp[r][c][comb] = make_pair (maxV, predecessor);
                    }
                }
            }
        }
    } while (improved() == true);
    --steps;

    ctr = 0;
    isAdded = vector <vector <int> >(wLength, vector <int> (wLength, 0));
    while (isFinished() == false) { //Alle hinzugefügten Packete platzieren, ein Swap pro Zeiteinheit
        for (int r = 0; r < wLength; ++r) {
            for (int c = 0; c < wLength; ++c) {
                pair <int, int> goTo = fields[r][c];
                if (goTo.first != r || goTo.second != c) { //Wenn das Packet noch nicht an der richtigen Stelle ist...
                    //Und wenn sein Verteilungsweg noch nicht verwendet wird...
                    bool fieldOcc = false;
                    for (int i = min (r, goTo.first); i <= max (r, goTo.first); ++i) {
                        fieldOcc |= isAdded[i][min(goTo,make_pair (r,c)).second] > 0;
                    }
                    for (int i = min (c, goTo.second); i <= max (c, goTo.second); ++i) {
                        fieldOcc |= isAdded[max(goTo,make_pair (r,c)).first][i] > 0;
                    }
                    //Dann markiere seinen Verteilungsweg und füge das Packet in die Platzierungsliste ein
                    if (fieldOcc == false) {
                        for (int i = min (r, goTo.first); i <= max (r, goTo.first); ++i) {
                            isAdded[i][min(goTo,make_pair (r,c)).second] = 2;
                        }
                        for (int i = min (c, goTo.second); i <= max (c, goTo.second); ++i) {
                            isAdded[max(goTo,make_pair (r,c)).first][i] = 2;
                        }
                        displaced.push_back (vector <pair <int, int> >
                                {make_pair (r, c), goTo, make_pair (ctr, 0), make_pair (r, c)});
                        displaced.push_back (vector <pair <int, int> >
                                {goTo, make_pair (r, c), make_pair (ctr, 0), goTo});
                        ++ctr;
                    }
                }
            }
        }
        for (auto it = displaced.begin(); it != displaced.end();) {
            pair <int, int> pNow = (*it)[0],
                            goTo = (*it)[1];
            //In der Richtigen Reihenfolge swappen, damit sowohl Packet als auch Umkehrpacket den gleichen Weg verwenden
            if (goTo.first > pNow.first) {
                swap (fields[pNow.first][pNow.second], fields[pNow.first+1][pNow.second]);
                result[pNow.first][pNow.second] += 'S';
                result[pNow.first+1][pNow.second] += 'N';
                ++(*it)[0].first;
            } else if (goTo.second < pNow.second) {
                swap (fields[pNow.first][pNow.second], fields[pNow.first][pNow.second-1]);
                result[pNow.first][pNow.second] += 'W';
                result[pNow.first][pNow.second-1] += 'O';
                --(*it)[0].second;
            } else if (goTo.first < pNow.first && (goTo.second <= pNow.second)) {
                swap (fields[pNow.first][pNow.second], fields[pNow.first-1][pNow.second]);
                result[pNow.first][pNow.second] += 'N';
                result[pNow.first-1][pNow.second] += 'S';
                --(*it)[0].first;
            } else if (goTo.second > pNow.second) {
                swap (fields[pNow.first][pNow.second], fields[pNow.first][pNow.second+1]);
                result[pNow.first][pNow.second] += 'O';
                result[pNow.first][pNow.second+1] += 'W';
                ++(*it)[0].second;
            } else {
                //unadd element
                pair <int, int> origin = (*it)[3];
                int subtracted = isAdded[origin.first][origin.second]-1;
                for (int i = min (origin.first, goTo.first); i <= max (origin.first, goTo.first); ++i) {
                    isAdded[i][min(goTo,origin).second] = subtracted;
                }
                for (int i = min (origin.second, goTo.second); i <= max (origin.second, goTo.second); ++i) {
                    isAdded[max(goTo,origin).first][i] = subtracted;
                }
                it = displaced.erase(it);
                continue;
            }
            if (next(it) != displaced.end() && (*it)[2].first == (*next(it))[2].first &&
                (*it)[2].second == 0) {
                int dist = (abs ((*next(it))[0].first-pNow.first)+
                            abs ((*next(it))[0].second-pNow.second));
                if (dist <= 2) {
                    if (dist == 1) {
                        assert ((*next(it))[0].second < 10 && (*next(it))[0].first < 10);
                        switch (result[pNow.first][pNow.second].back()) {
                        case 'S':  --(*next(it))[0].first; break;
                        case 'N':  ++(*next(it))[0].first; break;
                        case 'W':  ++(*next(it))[0].second; break;
                        case 'O':  --(*next(it))[0].second; break;
                        }
                        ++(*it)[2].second;
                        ++(*next(it))[2].second;
                    }
                    ++it;
                }
            }
            ++it;
        }
        if (displaced.size() > 0) {
            ++steps;
            stepsV.push_back(fields);
        }

        for (int r = 0; r < wLength; ++r) {
            for (int c = 0; c < wLength; ++c) {
                if (result[r][c].length() < steps)
                    result[r][c] +=  '_';
            }
        }

        vector <pair <int, int> > throwM = {make_pair (0,1),
                                            make_pair (1,0), make_pair (0,-1), make_pair(-1,0)}; //Wurfmatrix
        vector <char> throwMS = {'O', 'S', 'W', 'N'};   //Wurfmatrix-Zeichen
        vector <vector <bool> > usedNow (wLength, vector <bool> (wLength, false));
        for (int i = 1; i < wLength-1; ++i) {
            for (int j = 1; j < wLength-1; ++j) {
                for (int k = 0; k < throwM.size(); ++k) {
                    int iN = i+throwM[k].first,
                        jN = j+throwM[k].second;
                     int sum = mDist (fields[i][j],
                            make_pair (i,j)) > mDist (fields[i][j], make_pair (iN, jN)) ? 1 : -1;
                     sum += mDist (fields[iN][jN],
                            make_pair (iN,jN)) > mDist (fields[iN][jN], make_pair (i, j)) ? 1 : -1;
                     if ((isAdded[i][j] == 0 && isAdded[iN][jN] == 0) &&
                         usedNow[i][j]==false && usedNow[iN][jN]==false &&
                         ((sum == 2 && randomFac > (rand()%10)) || (sum==0 && randomFac > (rand()%100))) &&
                         (fields[i][j].first != i || fields[i][j].second != j) &&
                         (fields[iN][jN].first != iN || fields[iN][jN].second != jN)
                         ) {
                        result[i][j].back() = throwMS[k];
                        result[iN][jN].back() = throwMS[(k+2)%4];
                        swap (fields[i][j], fields[iN][jN]);
                        usedNow[i][j] = true;
                        usedNow[iN][jN] = true;
                     }
                }
            }
        }
    }
    alreadyFinished:
    ofstream fieldOut (resFileName+".paketverteilung.txt");
    for (int r = 0; r < wLength; ++r) {
        for (int c = 0; c < wLength; ++c) {
            fieldOut << r << ' ' << c << ' ' << result[r][c] << endl;
        }
    }
    fieldOut.close();

    ///GRAPHIKERZEUGUNG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    cout << steps << endl;
    for (int i = 0; i <= steps; ++i) {
        for (int j = 0; j < wLength; ++j) {
            for (int k = 0; k < wLength; ++k) {
                doc << Rectangle(Point(j*tLength, i*(tLength*wLength+10)+k*tLength), tLength, tLength, Color::Green, Stroke(1, Color::Blue));
            }
        }
    }
    for (int i = 0; i <= steps; ++i) {
        for (int j = 0; j < wLength; ++j) {
            for (int k = 0; k < wLength; ++k) {
                ostringstream sX, sY;
                sX << stepsV[i][j][k].first;
                sY << stepsV[i][j][k].second;
                string sCoords = sX.str()+ "|" + sY.str();
                doc << Text(Point(k*tLength+1,  10+i*(tLength*wLength+10)+j*tLength), sCoords, Fill(Color::Yellow), Font (4));
            }
        }
    }

    // Red image border.
    Polygon border(Stroke(1, Color::Red));
    border << Point(0, 0) << Point(dimensions.width, 0)
        << Point(dimensions.width, dimensions.height) << Point(0, dimensions.height);
    doc << border;


    for (int j = 0; j < steps; ++j) {
        for (int r = 0; r < wLength; ++r) {
            for (int c = 0; c < wLength; ++c) {
                int xA = c*tLength+(tLength/2),
                    yA = j*(tLength*wLength+10)+r*tLength+(tLength/2);
                switch (result[r][c][j]) {
                case 'S': paintArrow (Point (xA, yA), Point (xA, yA+tLength)); break;
                case 'N': paintArrow (Point (xA, yA), Point (xA, yA-tLength)); break;
                case 'W': paintArrow (Point (xA, yA), Point (xA-tLength,yA)); break;
                case 'O': paintArrow (Point (xA, yA), Point (xA+tLength,yA)); break;
                }
            }
        }
    }
    doc.save();
}
