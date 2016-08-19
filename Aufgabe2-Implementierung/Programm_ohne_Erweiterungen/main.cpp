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

using namespace std;
using namespace svg;

ifstream fieldIn ("siedlung.txt");
ofstream dOut ("debug.txt");
int wLength ,
    tLength = 20;
int steps = 0;

int elC;

Dimensions dimensions(wLength*tLength, wLength*(tLength+10)*steps);
Document doc("result.svg", Layout(dimensions, Layout::TopLeft));

void paintArrow (Point a, Point b) {
    Stroke s = Stroke (1, Color::Orange);
    doc << Line (a, b, s);
    doc << Line (b, Point (b.x+(a.y-b.y)/3+(a.x-b.x)/3, b.y+(a.x-b.x)/3+(a.y-b.y)/3), s);
    doc << Line (b, Point (b.x-(a.y-b.y)/3+(a.x-b.x)/3, b.y-(a.x-b.x)/3+(a.y-b.y)/3), s);
}

#define PII pair<int, int>

vector <vector <PII> > fields; //1,2 indizes; PII = wo will dieses Paket hin?
vector <vector < vector <PII> > > stepsV; //alle Schritte
vector <vector < vector <pair <int, char> > > > dp; // 1, 2: indizes, 3: welche reihe, 4: die felderkombination
vector <vector <string> > result; //1,2 indizes; string = Befehlsabfolge für die werfende Person an diser Stelle

bool improved () { //Testet, ob das DP eine Verbesserung gebracht hat.
    for (int i = 0; i < elC; ++i) {
//            dOut << "*" << dp.back().back()[i].first << " ";
            if (dp.back().back()[i].first > 200) { ///Wurde auf 200 gesetzt, um immert zu scheitern!!!
                return true;
            }
    }
    return false;
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
//    dOut << "Felder nach DP bewegen!" << endl;
    int conditionMask = elC;
    vector <vector <bool> > isHandled (wLength, vector <bool> (wLength, false));
    for (int r = wLength-1; r >= 0; --r) {
        for (int c = wLength-1; c >= 0; --c) {
//            dOut << "[" << r << "," << c << "]: ";
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
//            dOut << endl << "Zustand: " << dp[r][c][idxMax].second << endl;
            if (isHandled[r][c] == false) {
                switch (dp[r][c][idxMax].second) { //Abhängig vom besten Wert die Swaps ausführen
                case 0: conditionMask = elC; result[r][c] += '_'; break;
                case 1: swap (fields[r][c-1], fields[r][c]); conditionMask = ~(elC & (1 << c));  result[r][c] += 'W'; result[r][c-1] += 'O'; isHandled[r][c-1] = true; break;
                case 2: swap (fields[r-1][c], fields[r][c]); conditionMask  = ~(elC & (1 << (r+c-1))); result[r][c] += 'N'; result[r-1][c] += 'S'; isHandled[r-1][c] = true; break;
                }
            }
        }
    }
}

void printField () {
    for (vector <PII> vec : fields) {
        for (PII p : vec) {
//            dOut << p.first << '|' << p.second << ' ';
;
        }
//        dOut << endl;
    }
}

int main()
{
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
//    dOut << "input finished" << endl;
    printField();
    //DP Beginn
    dp.resize (wLength, vector <vector <pair <int, char> > > (wLength, vector <pair <int, char> > (elC, make_pair (0,0)))); //Neu für jede neue Zeiteinheit
    do {
        if (steps > 0) {
            moveField();
            printField();
//            dOut <<"printed" << endl;
//            dOut << "steps: " << steps << endl;
        }
        stepsV.push_back(fields);
        cout << steps << endl;
        ++steps;
//        dp.clear();
        for (int i = 0; i < wLength; ++i) {
            for (int j = 0; j < wLength; ++j) {
                fill(dp[i][j].begin(), dp[i][j].end(), make_pair (0,0));
            }
        }

        for (int r = 0; r < wLength; ++r) {
            cout << r << ' ' << dp.size() << endl;
            for (int c = 0; c < wLength; ++c) {
                dOut << "field [" << r << ',' << c << "]: " << endl;
                if (r == 0 && c == 0)   continue; //Das erste Feld unbeachtet lassen, wurde schon initialisiert
                pair <int, int> prevP = getPrev (r, c), //Das vorherige Packet
                                prevV = fields[prevP.first][prevP.second], //Das Ziel des vorherigen Packets
                                nowV = fields[r][c]; //Das Ziel des jetzigen Packets
                int elsToCheck = getIndex(r > 0 ? wLength+c : c); //Alle Bindungsmöglichkeiten mit dem aktuellen Packet berechnen

                for (int comb = 0; comb < elsToCheck; ++comb) {
//                    dOut << "prevV: " << prevV.first << '|' << prevV.second << "\tnowV: " << nowV.first << '|' << nowV.second << " ";

                    int offset = r > 0 ? wLength : 0;

                    if (c == 0) {
                        int combPrev = (comb & ~(1 << (wLength))); //Die vorherige Kombination äquivalent zu dieser Kombination
                        if (r>1)
                             combPrev <<= wLength;
//                        dOut << "combPrev: " << bitset<32>(combPrev) << endl;
                        int maxVAll = 0;
                        for (int cToChk = combPrev; cToChk < (combPrev+ (r > 1 ? getIndex(wLength) : 1)); ++cToChk) {
                            int bindingNothing = dp[prevP.first][prevP.second].at(cToChk).first;//Der Wert des vorherigen Feldes zu dieser Kombination
                            int maxV = bindingNothing;
                            int predecessor = 0; //0 = unbesetzt, 1 = bund mit links, 2 = bund mit oben
                            int oldMax = maxV;
                            if (isZero(comb, offset) == false) { //wenn das eigene Feld besetzt werden darf
                                if (r > 0 && isZero (comb, c) == false) { //wenn das oben angrenzende Feld besetzt werden darf
                                    pair <int, int> upV = fields[r-1][c];
                                    int bindingUp = (upV.first >= r ? 1 : -1) + (nowV.first <= (r-1) ? 1 : -1);
//                                    dOut << " bindingUp: " << bindingUp << "." << dp[r-1][c].at(cToChk & ~(1 << (wLength))).first << ", ";
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
//                                dOut << " bindingLeft: " << bindingLeft << "." << dp[r][c-1][combPrev & ~(1 << (c-1+offset))].first << ", ";
                                maxV = max (maxV, bindingLeft + dp[r][c-1][combPrev & ~(1 << (c-1+offset))].first);
                                predecessor = oldMax == maxV ? predecessor : 1;
                                oldMax = maxV;
                            }
                            if (r > 0 && isZero (comb, c) == false) { //wenn das oben angrenzende Feld besetzt werden darf
                                pair <int, int> upV = fields[r-1][c];
                                int bindingUp = (upV.first >= r ? 1 : -1) + (nowV.first <= (r-1) ? 1 : -1);
//                                dOut << " bindingUp: " << bindingUp << "." << dp[r-1][c][combPrev & ~(1 << (c))].first << ", ";
                                maxV = max (maxV, bindingUp + dp[r-1][c][combPrev & ~(1 << (c))].first);
                                predecessor = oldMax == maxV ? predecessor : 2;
                            }
                        }
                        dp[r][c][comb] = make_pair (maxV, predecessor);
                    }
                    bitset<32> nB (comb);
//                    dOut << nB << ": " << dp[r][c][comb].first << "|" << dp[r][c][comb].second << endl;;
                }
//                dOut << endl;
            }
        }
//        dOut << "dp finished: " << endl;
        /*for (int i = 0; i < wLength; ++i) {
            for (int j = 0; j < wLength; ++j) {
                for (int k = 0; k < elC; ++k) {
                    dOut << dp[i][j][k].first << ',' << dp[i][j][k].second << ' ';
                }
                dOut << "| ";
            }
            dOut << endl;
        }*/
    } while (improved() == true);
    --steps;
    dOut << "finished DP!" << endl;


    list <vector <pair <int, int> > > displaced; //Die verrückten Packete
    vector <vector <bool> > isAdded (wLength, vector <bool> (wLength, false)); //Wird das Packet gerade verwendet?
    bool isFinished = false; //Sind alle Packete auf dem richtigen Platz?
    do { //Jeder Durchgang platziert displaced.size()/2 Packete korrekt
        printField();
        displaced.clear();
        isAdded.clear();
        isAdded.resize (wLength, vector <bool> (wLength, false));
        isFinished = true;
        int ctr = 0;
        for (int r = 0; r < wLength; ++r) {
            for (int c = 0; c < wLength; ++c) {
//                dOut << "field [" << r << '|' << c << ']' << endl;
                pair <int, int> goTo = fields[r][c];
                if (goTo.first != r || goTo.second != c) { //Wenn das Packet noch nicht an der richtigen Stelle ist...
                    //Und wenn sein Verteilungsweg noch nicht verwendet wird...
                    bool fieldOcc = false;
                    for (int i = min (r, goTo.first); i <= max (r, goTo.first); ++i) {
                        fieldOcc |= isAdded[i][c];
                    }
                    for (int i = min (c, goTo.second); i <= max (c, goTo.second); ++i) {
                        fieldOcc |= isAdded[goTo.first][i];
                    }
                    //Dann markiere seinen Verteilungsweg und füge das Packet in die Platzierungsliste ein
                    if (fieldOcc == false) {
//                        dOut << "added!" << endl;
                        for (int i = min (r, goTo.first); i <= max (r, goTo.first); ++i) {
                            isAdded[i][c] = true;
                        }
                        for (int i = min (c, goTo.second); i <= max (c, goTo.second); ++i) {
                            isAdded[goTo.first][i] = true;
                        }
                        isFinished = false;
                        displaced.push_back (vector <pair <int, int> > {make_pair (r, c), goTo, make_pair (ctr, 0)});
                        displaced.push_back (vector <pair <int, int> > {goTo, make_pair (r, c), make_pair (ctr, 0)});
                        ++ctr;
                    }
                }
            }
        }
//        dOut << "finished search for displaced packets" << endl;
        for (auto it = displaced.begin(); it != displaced.end(); ++it) {
            dOut << "field " << (*it)[0].first << '|' << (*it)[0].second << " needs to go to " << (*it)[1].first << '|' << (*it)[1].second << "v: " << (*it)[2].first << endl;
        }

        while (displaced.size() > 0) { //Alle hinzugefügten Packete platzieren, ein Swap pro Zeiteinheit
//            dOut << "displaced has " << displaced.size() << " elements" << endl;
            for (auto it = displaced.begin(); it != displaced.end();) {
                pair <int, int> pNow = (*it)[0],
                                goTo = (*it)[1];
                dOut << "pNow: " << pNow.first << '|' << pNow.second << " goTo: " << goTo.first << '|' << goTo.second << endl;

                //In der Richtigen Reihenfolge swappen, damit sowohl Packet als auch Umkehrpacket den gleichen Weg verwenden
                if (goTo.first > pNow.first) {
                    dOut << "S!" << endl;
                    swap (fields[pNow.first][pNow.second], fields[pNow.first+1][pNow.second]);
                    result[pNow.first][pNow.second] += 'S';
                    result[pNow.first+1][pNow.second] += 'N';
                    ++(*it)[0].first;
                } else if (goTo.second < pNow.second) {
                    dOut << "W!" << endl;
                    swap (fields[pNow.first][pNow.second], fields[pNow.first][pNow.second-1]);
                    result[pNow.first][pNow.second] += 'W';
                    result[pNow.first][pNow.second-1] += 'O';
                    --(*it)[0].second;
                } else if (goTo.first < pNow.first && (goTo.second <= pNow.second)) {
                    dOut << "N!" << endl;
                    swap (fields[pNow.first][pNow.second], fields[pNow.first-1][pNow.second]);
                    result[pNow.first][pNow.second] += 'N';
                    result[pNow.first-1][pNow.second] += 'S';
                    --(*it)[0].first;
                } else if (goTo.second > pNow.second) {
                    dOut << "O!" << endl;
                    swap (fields[pNow.first][pNow.second], fields[pNow.first][pNow.second+1]);
                    result[pNow.first][pNow.second] += 'O';
                    result[pNow.first][pNow.second+1] += 'W';
                    ++(*it)[0].second;
                } else {
                    it = displaced.erase(it);
                    dOut << "erased!" << endl;
                    continue;
                }
                dOut << "itNow: " << (*it)[0].first << '|' << (*it)[0].second << endl;
                if (next(it) != displaced.end() && (*it)[2].first == (*next(it))[2].first && (*it)[2].second == 0) {
                    dOut << "itNext: " << (*next(it))[0].first << '|' << (*next(it))[0].second << endl;
                    int dist = (abs ((*next(it))[0].first-pNow.first)+abs ((*next(it))[0].second-pNow.second));
                    if (dist <= 2) {
                        if (dist == 1) {
                            assert ((*next(it))[0].second < 10 && (*next(it))[0].first < 10);
                            switch (result[pNow.first][pNow.second].back()) {
                            case 'S':  --(*next(it))[0].first; break;
                            case 'N':  ++(*next(it))[0].first; break;
                            case 'W':  ++(*next(it))[0].second; break;
                            case 'O':  --(*next(it))[0].second; break;
                            }
                            assert ((*next(it))[0].second < 10 && (*next(it))[0].first < 10);
                            ++(*it)[2].second;
                            ++(*next(it))[2].second;
                            assert ((*it)[2].second < 2 && (*next(it))[2].second < 2);
                        }
                        dOut << "skipped!" << endl;
                        ++it;
                    }
                }
                //dOut << "counter: " << counter << " distance: " <<  (abs ((*next(it))[0].first-pNow.first)+abs ((*next(it))[0].second-pNow.second) <= 2) << endl;
                ++it;
            }
            if (displaced.size() > 0) {
                ++steps;
                cout << steps << endl;
                stepsV.push_back(fields);
            }
            dOut << "steps: " << steps << endl;
            for (int r = 0; r < wLength; ++r) {
                for (int c = 0; c < wLength; ++c) {
                    if (result[r][c].length() < steps)
                        result[r][c] +=  '_';
                    dOut << r << '|' << c << " " << result[r][c] << endl;
                }
            }
        }
        dOut << "finished while-loop" << endl;
    } while (isFinished == false);
    dOut << "finished final placing" << endl;
    ofstream fieldOut ("aOut.txt");
    for (int r = 0; r < wLength; ++r) {
        for (int c = 0; c < wLength; ++c) {
            fieldOut << r << ' ' << c << ' ' << result[r][c] << endl;
        }
    }
    fieldOut.close();

    ///GRAPHIKERZEUGUNG!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    cout << stepsV.size() << ' ' << steps << endl;
    ifstream fin ("aOut.txt");
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
//            cout << "i: " << i << " " << x << " " << y << " " << s << endl;
                int xA = c*tLength+(tLength/2),
                    yA = j*(tLength*wLength+10)+r*tLength+(tLength/2);
                switch (result[r][c][j]) {
                case 'S': paintArrow (Point (xA, yA), Point (xA, yA+tLength)); break;
                case 'N': paintArrow (Point (xA, yA), Point (xA, yA-tLength)); break;
                case 'W': paintArrow (Point (xA, yA), Point (xA-tLength,yA)); break;
                case 'O': paintArrow (Point (xA, yA), Point (xA+tLength,yA)); break;
                }
//                cout << "x: " << x << "y: " << y << endl;
            }
        }
    }
    doc.save();
}
