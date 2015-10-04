#ifndef HLSL_TO_GLSL_H
#define HLSL_TO_GLSL_H

#include <string>
using namespace std;

namespace HlslToGlsl
{
bool ConvertHlslToGlslFromFile(const string& filename, const string& entryFunctionName, bool isVertexShader, string& outputGlsl);
bool ConvertHlslToGlslFromSource(const string& hlslSource, const string& entryFunctionName, bool isVertexShader, string& outputGlsl);
}

#endif