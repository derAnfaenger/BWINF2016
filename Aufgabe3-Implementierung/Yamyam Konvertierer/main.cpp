#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

ifstream fin ("yamyam.in");
ofstream fout ("yamyam.out");

vector <vector <int> > AdjM;
vector <vector <int> > numM;
vector <vector <int> > dirs {{0, 1},{1, 0},{0, -1},{-1, 0}};
int width, height;

int toAdd (int dir) {
    switch (dir) {
    case 0: return 1;
    case 1: return width-2;
    case 2: return -1;
    case 3: return -(width-2);
    }
}

int main()
{
    fin >> width >> height;
   // fout << (width-2)*(height-2) << endl;
    AdjM.resize (height, vector <int> (width));
    numM.resize (height, vector <int> (width));

    int counter = -1;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            fin >> AdjM[i][j];
            if (AdjM[i][j] != 1) {
                ++counter;
            }
            numM[i][j] = counter;
        }
    }
    fout << counter+1 << endl;

    for (int i = 1; i < height-1; ++i) {
        for (int j = 1; j < width-1; ++j) {
           // fout << "position : " << i << ' ' << j << endl;
            if (AdjM[i][j] != 1) {
                vector <vector <int> > cons;
                vector <int> nums;
                for (int k = 0; k < 4; ++k) {
                    int obj = AdjM[i+dirs[k][0]][j+dirs[k][1]],
                        mirrorObj = AdjM[i+dirs[(k+2)%4][0]][j+dirs[(k+2)%4][1]];
               //     fout << "pos: " << pos << endl;
                    if (obj != 1 && mirrorObj != 1) {
                     //   fout << "toAddM: " <<  toAdd ((k+2)%4) << endl;
                        cons.push_back (vector <int> {numM[i+dirs[(k+2)%4][0]][j+dirs[(k+2)%4][1]]});
                        nums.push_back (numM[i+dirs[k][0]][j+dirs[k][1]]);
                    }
                    else if (obj != 1) {
                        vector <int> nextTiles;
                        for (int l = 0; l < 4; ++l) {
                            if (AdjM[i+dirs[l][0]][j+dirs[l][1]] != 1) {
                            //    fout << "toAddN: " <<  toAdd (l) << endl;
                                nextTiles.push_back (numM[i+dirs[l][0]][j+dirs[l][1]]);
                            }
                        }
                        cons.push_back (nextTiles);
                        nums.push_back (numM[i+dirs[k][0]][j+dirs[k][1]]);
                    }
                }
                cons.push_back(nums);
                nums.push_back(numM[i][j]);
                fout << (AdjM[i][j] == 2 ? 1 : 0) << ' ' << cons.size() << endl;;
                for (int i = 0; i < cons.size(); ++i) {
                    fout << nums[i] << ' ' << cons[i].size() << endl;
                    for (int j = 0; j < cons[i].size(); ++j) {
                        fout << cons[i][j] << ' ';
                    }
                    fout << endl;
                }
            }
        }
    }
}
