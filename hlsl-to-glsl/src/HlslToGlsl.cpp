#include "HlslToGlsl.h"

#include "CodeGenerator.h"
#include "Tokenizer.h"

#include <fstream>
using namespace std;

namespace HlslToGlsl
{

void WriteHeaderOfGlsl(string& outputGlsl);

bool ConvertHlslToGlslFromFile(const string& filename, const string& entryFunctionName, bool isVertexShader, string& outputGlsl)
{
    outputGlsl = "";

    ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        return false;
    }

    string inputHlsl;
    inputFile.seekg(0, std::ios::end);
    inputHlsl.reserve((size_t) inputFile.tellg());
    inputHlsl.resize((size_t) inputFile.tellg());
    inputFile.seekg(0, std::ios::beg);

    inputFile.read(&inputHlsl[0], inputHlsl.length());

    return ConvertHlslToGlslFromSource(inputHlsl, entryFunctionName, isVertexShader, outputGlsl);
}

bool ConvertHlslToGlslFromSource(const string& hlslSource, const string& entryFunctionName, bool isVertexShader, string& outputGlsl)
{
    vector<Lexeme> lexemes = ParseIntoLexemes(hlslSource);

    WriteHeaderOfGlsl(outputGlsl);
    ConvertLexemesIntoGlsl(lexemes, entryFunctionName, isVertexShader, outputGlsl);

    return true;
}

void WriteHeaderOfGlsl(string& outputGlsl)
{
    outputGlsl += "#version 420\n";   
}

}