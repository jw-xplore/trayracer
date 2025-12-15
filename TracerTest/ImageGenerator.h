#pragma once
#include <vector>
#include <fstream>
#include "../color.h"

static void CreateImage(int w, int h, std::vector<Color>& renderResult)
{
    std::ofstream img("testImg.pgm", std::ios_base::out
        | std::ios_base::binary
        | std::ios_base::trunc
    );

    img << "P5\n" << w << " " << h << "\n" << 255 << "\n";

    char* line = new char[w];
    int pos = 0;

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            pos = w * y + x;
            int val = ((renderResult[pos].r + renderResult[pos].g + renderResult[pos].b) * 255) / 3;
            line[x] = val;
        }

        //std::cout << line << std::endl;

        img.write(reinterpret_cast<const char*>(line), w);
    }

    delete line;
    img.close();
}