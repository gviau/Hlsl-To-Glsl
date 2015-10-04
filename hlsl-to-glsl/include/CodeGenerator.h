#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_h

#include "Tokenizer.h"

#include <vector>
#include <string>
using namespace std;

namespace HlslToGlsl
{

void ConvertLexemesIntoGlsl(const vector<Lexeme>& lexemes, const string& entryFunctionName, bool isVertexShader, string& outputGlsl);

vector<string> PreprocessTextures(const vector<Lexeme>& lexemes, string& outputGlsl);
void InterpretLexeme(const vector<Lexeme>& lexemes, const string& entryFunctionName, const vector<string>& originalTextureNames, size_t& lexemeIndex, bool isVertexShader, string& outputGlsl);

}

#endif