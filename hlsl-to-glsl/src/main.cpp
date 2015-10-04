#include "HlslToGlsl.h"

#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " input_file.hlsl output_file.glsl isVertexShader {true|false}" << endl;
        return 1;
    }

    bool isVertexShader = false;
    if (strcmp(argv[3], "true") == 0)
    {
        isVertexShader = true;
    }
    else if (strcmp(argv[3], "false") == 0)
    {
        isVertexShader = false;
    }
    else
    {
        cerr << "Invalid third parameter: " << argv[3] << " ! Reconized values are true or false" << endl;
        return 1;
    }

    string outputGlsl;
    HlslToGlsl::ConvertHlslToGlslFromFile(argv[1], "main", isVertexShader, outputGlsl);

    ofstream outputFile(argv[2]);
    outputFile << outputGlsl;
    outputFile.close();

    return 0;
}