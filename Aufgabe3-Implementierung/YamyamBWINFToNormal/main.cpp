#include <iostream>
#include <fstream>

using namespace std;

ifstream fin ("toConvert.in");
ofstream fout ("toConvert.out");

int main()
{
    char c;
    while (fin.get (c)) {
        switch (c) {
        case '#': fout << "1 "; break;
        case ' ': fout << "0 "; break;
        case 'E': fout << "2 "; break;
        case '\n': fout << "\n"; break;
        }

    }
}
