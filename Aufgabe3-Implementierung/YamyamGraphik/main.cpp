#include "simple_svg_1.0.0.hpp"
#include <fstream>

using namespace svg;
using namespace std;

ifstream fin ("grid.in");

const int f = 12;

int main()
{
    int width, height;

    fin >> width >> height;

    Dimensions dimensions(width*f, height*f);
    Document doc("field.svg", Layout(dimensions, Layout::TopLeft));

    // Red image border.
    Polygon border(Stroke(1, Color::Red));
    border << Point(0, 0) << Point(dimensions.width, 0)
        << Point(dimensions.width, dimensions.height) << Point(0, dimensions.height);
    doc << border;

    for (int i = 0; i < height; ++i) {
        cout << i << endl;
        for (int j = 0; j < width; ++j) {
            vector <int> field(4);
            bool isExit = true;
            for (int d = 0; d < 4; ++d) {
                fin >> field[d];
                isExit &= (field[d] == 3 || field[d] == 5);
            }
            for (int d = 0; d < 4; ++d) {
                int x, y, width, height;
                switch (d) {
                case 2: x = j*f; y = i*f; width = f/4; height = f; break;
                case 0: x = j*f+(f/4)*3; y = i*f; width = f/4; height = f; break;
                case 3: x = j*f+(f/4); y = i*f; width = f/2; height = f/2; break;
                case 1: x = j*f+(f/4); y = i*f+(f/2); width = f/2; height = f/2; break;
                }
                auto color = (isExit == true ? Color::Green : Color::Black);
                auto stroke = (isExit == true ? 1.5 : .5);
                switch (field[d]) {
                case 0: doc << Rectangle(Point(x, y), width, height, Color::Black, Stroke(stroke, color)); break;
                case 1: doc << Rectangle(Point(x, y), width, height, Color::Red, Stroke(stroke, color)); break;
                case 2: doc << Rectangle(Point(x, y), width, height, Color::Orange, Stroke(stroke, color)); break;
                case 3: doc << Rectangle(Point(x, y), width, height, Color::Yellow, Stroke(stroke, color)); break;
                case 4: doc << Rectangle(Point(x, y), width, height, Color::Green, Stroke(stroke, color)); break;
                case 5: doc << Rectangle(Point(x, y), width, height, Color::Purple, Stroke(stroke, color)); break;
                }
            }
        }
    }

    doc.save();
}
