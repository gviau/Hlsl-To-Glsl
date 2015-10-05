#include "CodeGenerator.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
using namespace std;

namespace HlslToGlsl
{

// State variable
size_t currentIndentationLevel = 0;
bool startOfLine = true;
bool insideOfStruct = false;
bool hadAnySemanticsInStruct = false;
vector<string> structNames;
bool isOutputSemanticStruct = false;

string structBufferIfNoSemanticsInStruct;
vector<string> semantics;

vector<string> semanticStructNameToIgnore;
vector<string> semanticStructVariableToIgnore;
bool mightAddSemanticStructNameToIgnore = false;
bool isInEntryFunction = false;
size_t entryFunctionLevel = 0;

vector<string> samplerStateTextureNames;
vector<string> samplerStateTextureNamesToUse;

string glPositionName = "";

bool IsStructName(const string& name)
{
    for (const string& structName : structNames)
    {
        if (name == structName)
        {
            return true;
        }
    }

    return false;
}

bool IsSemanticStructName(const string& name)
{
    for (const string& structName : semanticStructNameToIgnore)
    {
        if (name == structName)
        {
            return true;
        }
    }

    return false;
}

bool IsSemanticStructVariable(const string& name)
{
    for (const string& structName : semanticStructVariableToIgnore)
    {
        if (name == structName)
        {
            return true;
        }
    }

    return false;
}

string GetSamplerStateTextureName(const string& samplerStateName, const string& textureName)
{
    for (size_t i = 0; i < samplerStateTextureNames.size(); i++)
    {
        string comp = samplerStateName + textureName;
        if (samplerStateTextureNames[i].substr(0, samplerStateTextureNames[i].size() - 5) == comp)
        {
            return samplerStateTextureNamesToUse[i];
        }
    }

    return "";
}

void InterpretArithmeticOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretAssignation(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretBitwiseOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretBuiltinFunction(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretCbuffer(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void IntrepretClosedAngleBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretClosedCurlyBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, bool isVertexShader, string& outputGlsl);
void IntrepretClosedParanthesis(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretColon(const vector<Lexeme>& lexemes, size_t& lexemeIndex, bool isVertexShader, string& outputGlsl);
void InterpretComma(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretComment(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretFlowControl(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretOpenedAngleBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretOpenedCurlyBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretOpenedParanthesis(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretRegister(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretRelationalOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretSamplerState(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretSemiColumn(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretStruct(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretStructureOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretTernaryOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretTexture(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretType(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl);
void InterpretVariableName(const vector<Lexeme>& lexemes, const string& entryFunctionName, const vector<string>& originalTextureNames, size_t& lexemeIndex, string& outputGlsl);

string hlslTypesMappingToGlsl[] = {
    "bool", "bool",
    "int", "int",
    "uint", "uint",
    "dword", "uint",
    "half", "float",
    "float", "float",
    "double", "double",

    "bool1", "bool",
    "bool2", "bvec2",
    "bool3", "bvec3",
    "bool4", "bvec4",

    "int1", "int",
    "int2", "ivec2",
    "int3", "ivec3",
    "int4", "ivec4",

    "uint1", "uint",
    "uint2", "uvec2",
    "uint3", "uvec3",
    "uint4", "uvec4",

    "half1", "float",
    "half2", "vec2",
    "half3", "vec3",
    "half4", "vec4",

    "float1", "float",
    "float2", "vec2",
    "float3", "vec3",
    "float4", "vec4",

    "double1", "double",
    "double2", "dvec2",
    "double3", "dvec3",
    "double4", "dvec4",

    "float2x2", "mat2",
    "float3x3", "mat3",
    "float4x4", "mat4",
};

string hlslBuiltinFunctionsToGlsl[] = {
    "abs",              "abs",
    "acos",             "acos",
    "all",              "all",
    "any",              "any",
    "asin",             "asin",
    "atan",             "atan",
    "atan2",            "atan",
    "ceil",             "ceil",
    "clamp",            "clamp",
    "clip",             "clip",         // Custom function for GLSL
    "cos",              "cos",
    "cosh",             "cosh",
    "cross",            "cross",
    "ddx",              "dFdx",
    "ddx_coarse",       "dFdxCoarse",
    "ddx_fine",         "dFdxFine",
    "ddy",              "dFdy",
    "ddy_coarse",       "dFdyCoarse",
    "ddy_fine",         "dFdyFine",
    "degrees",          "degreese",
    "determinant",      "determinant",
    "distance",         "distance",
    "dot",              "dot",
    "exp",              "exp",
    "exp2",             "exp2",
    "faceforward",      "facefoward",
    "floor",            "floor",
    "fma",              "fma",
    "fmod",             "fmod",         // Custom function for GLSL
    "frac",             "frac",
    "frexp",            "frexp",
    "fwidth",           "fwidth",
    "isinf",            "isinf",
    "isnan",            "isnan",
    "ldexp",            "ldexp",
    "length",           "length",
    "lerp",             "mix",
    "log",              "log",
    "log10",            "log10",        // Custon function for GLSL
    "log2",             "log2",
    "max",              "max",
    "min",              "min",
    "modf",             "modf",
    "mul",              "*",            // Special case, mul in HLSL is simply the * operator in GLSL
    "noise",            "noise",
    "normalize",        "normalize",
    "pow",              "pow",
    "radians",          "radians",
    "rcp",              "sqrt",
    "reflect",          "reflect",
    "refract",          "refract",
    "round",            "round",
    "rsqrt",            "1.0 / sqrt",
    "saturate",         "min(1.0, max(0.0, ",
    "sign",             "sign",
    "sin",              "sin",
    "sinh",             "sinh",
    "smoothstep",       "smoothstep",
    "sqrt",             "sqrt",
    "step",             "step",
    "tan",              "tan",
    "tanh",             "tanh",
    "tex1D",            "texture",
    "tex1Dbias",        "texture",
    "tex1Dgrad",        "textureGrad",
    "tex1Dlod",         "textureLod",
    "tex1Dproj",        "textureProj",
    "tex2D",            "texture",
    "tex2Dbias",        "texture",
    "tex2Dgrad",        "textureGrad",
    "tex2Dlod",         "textureLod",
    "tex2Dproj",        "textureProj",
    "tex3D",            "texture",
    "tex3Dbias",        "texture",
    "tex3Dgrad",        "textureGrad",
    "tex3Dlod",         "textureLod",
    "tex3Dproj",        "textureProj",
    "texCUBE",          "texture",
    "texCUBEbias",      "texture",
    "texCUBEgrad",      "textureGrad",
    "texCUBElod",       "textureLod",
    "texCUBEproj",      "textureProj",
    "transpose",        "transpose",
    "trunc",            "trunc"
};

void ConvertLexemesIntoGlsl(const vector<Lexeme>& lexemes, const string& entryFunctionName, bool isVertexShader, string& outputGlsl)
{
    vector<string> originalTextureNames = PreprocessTextures(lexemes, outputGlsl);

    for (size_t i = 0; i < lexemes.size(); i++)
    {
        if (startOfLine)
        {
            startOfLine = false;
        }

        InterpretLexeme(lexemes, entryFunctionName, originalTextureNames, i, isVertexShader, outputGlsl);
    }
}

vector<string> PreprocessTextures(const vector<Lexeme>& lexemes, string& outputGlsl)
{
    vector<string> originalTextureNames;

    // Pair of string and register slot. For textures, additionally store the dimension (1, 2 or 3)
    vector<pair<string, int>> samplerStateNames;
    vector<pair<pair<string, int>, int>> textureNames;

    for (size_t i = 0; i < lexemes.size(); i++)
    {
        const Lexeme& lexeme = lexemes[i];
        Lexeme registerLexeme;

        size_t dimension = 0;
        pair<string, int> nameRegister;

        if (lexeme.m_TokenClass == TokenClass_t::SAMPLER_STATE)
        {
            registerLexeme = lexemes[i + 5];
            nameRegister = pair<string, int>(lexemes[i + 1].m_Token, atoi(registerLexeme.m_Token.substr(1, string::npos).c_str()));
            samplerStateNames.push_back(nameRegister);
            i += 1;
        }
        else if (lexeme.m_TokenClass == TokenClass_t::TEXTURE)
        {
            originalTextureNames.push_back(lexemes[i + 1].m_Token);

            registerLexeme = lexemes[i + 5];

            dimension = (size_t)(lexeme.m_Token[lexeme.m_Token.size() - 2] - '0');

            nameRegister = pair<string, int>(lexemes[i + 1].m_Token, atoi(registerLexeme.m_Token.substr(1, string::npos).c_str()));
            textureNames.push_back(pair<pair<string, int>, int>(nameRegister, dimension));
            i += 1;
        }
        else if (lexeme.m_TokenClass ==  TokenClass_t::STRUCTURE_OPERATOR)
        {
            // Check if the keyword after that is Sample and if the keyword before that is a texture name
            const Lexeme& previousLexeme = lexemes[i - 1];
            const Lexeme& nextLexeme = lexemes[i + 1];

            auto pred = [&] (pair<pair<string, int>, int> val) {
                return (val.first.first == previousLexeme.m_Token);
            };

            if ( !(find_if(textureNames.begin(), textureNames.end(), pred) != textureNames.end() && nextLexeme.m_Token == "Sample") )
            {
                continue;
            }

            size_t samplerStateIndex = 0;
            size_t textureIndex = 0;

            for (size_t j = 0; j < samplerStateNames.size(); j++)
            {
                if (samplerStateNames[j].first == lexemes[i + 3].m_Token)
                {
                    samplerStateIndex = samplerStateNames[j].second;
                    break;
                }
            }

            for (size_t j = 0; j < textureNames.size(); j++)
            {
                if (textureNames[j].first.first == previousLexeme.m_Token)
                {
                    textureIndex = textureNames[j].first.second;
                    break;
                }
            }

            // Add the new texture name if it's not already in the list, in order of sampler register and then texture register slot
            string samplerStateTextureIndex;
            stringstream ss;
            ss << "_" << samplerStateIndex << "_" << setfill('0') << setw(2) << textureIndex;
            samplerStateTextureIndex = ss.str();

            string samplerStateTextureName = lexemes[i + 3].m_Token + previousLexeme.m_Token + samplerStateTextureIndex;

            bool isInList = false;
            for (size_t j = 0; j < samplerStateTextureNames.size(); j++)
            {
                if (samplerStateTextureNames[j] == samplerStateTextureName)
                {
                    isInList = true;
                    break;
                }
            }

            if (isInList)
            {
                continue;
            }

            size_t index = 0;
            for (; index < samplerStateTextureNames.size(); index++)
            {
                const string& name = samplerStateTextureNames[index];
                size_t tIndex = atoi(name.substr(name.size() - 2, 2).c_str());
                size_t sIndex = atoi(name.substr(name.size() - 4, 1).c_str());

                if (samplerStateIndex <= sIndex && textureIndex < tIndex)
                {
                    break;
                }
            }

            samplerStateTextureNames.insert(samplerStateTextureNames.begin() + index, samplerStateTextureName);

        }
    }

    // Output the sampler states
    for (size_t i = 0; i < samplerStateTextureNames.size(); i++)
    {
        size_t samplerIndex = atoi(samplerStateTextureNames[i].substr(samplerStateTextureNames[i].size() - 4, 1).c_str());
        size_t textureIndex = atoi(samplerStateTextureNames[i].substr(samplerStateTextureNames[i].size() - 2, 2).c_str());
        size_t dimension = textureNames[textureIndex].second;

        size_t idx = samplerStateNames[samplerIndex].second * originalTextureNames.size() + textureNames[textureIndex].first.second;
        
        string nameIndex = "";
        stringstream ss;
        ss << "_" << samplerIndex << "_" << setfill('0') << setw(2) << textureIndex;
        nameIndex = ss.str();
        string nameToUse = "texture" + nameIndex;

        samplerStateTextureNamesToUse.push_back(nameToUse);

        outputGlsl += "uniform sampler" + to_string(dimension) + "D " + nameToUse + ";\n";
    }

    outputGlsl += "\n";

    return originalTextureNames;
}

void InterpretLexeme(const vector<Lexeme>& lexemes, const string& entryFunctionName, const vector<string>& originalTextureNames, size_t& lexemeIndex,
                     bool isVertexShader, string& outputGlsl)
{
    const Lexeme& lexeme = lexemes[lexemeIndex];

    switch (lexeme.m_TokenClass)
    {
    case TokenClass_t::ARITHMETIC_OPERATOR:     InterpretArithmeticOperator(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::ASSIGNATION:             InterpretAssignation(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::BITWISE_OPERATOR:        InterpretBitwiseOperator(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::BUILTIN_FUNCTION:        InterpretBuiltinFunction(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::CBUFFER:                 InterpretCbuffer(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::CLOSED_ANGLE_BRACKET:    IntrepretClosedAngleBracket(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::CLOSED_CURLY_BRACKET:    InterpretClosedCurlyBracket(lexemes, lexemeIndex, isVertexShader, outputGlsl); break;
    case TokenClass_t::CLOSED_PARANTHESIS:      IntrepretClosedParanthesis(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::COLON:                   InterpretColon(lexemes, lexemeIndex, isVertexShader, outputGlsl); break;
    case TokenClass_t::COMMA:                   InterpretComma(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::COMMENT:                 InterpretComment(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::FLOW_CONTROL:            InterpretFlowControl(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::OPENED_ANGLE_BRACKET:    InterpretOpenedAngleBracket(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::OPENED_CURLY_BRACKET:    InterpretOpenedCurlyBracket(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::OPENED_PARANTHESIS:      InterpretOpenedParanthesis(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::REGISTER:                InterpretRegister(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::RELATIONAL_OPERATOR:     InterpretRelationalOperator(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::SAMPLER_STATE:           InterpretSamplerState(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::SEMICOLUMN:              InterpretSemiColumn(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::STRUCT:                  InterpretStruct(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::STRUCTURE_OPERATOR:      InterpretStructureOperator(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::TEXTURE:                 InterpretTexture(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::TERNARY_OPERATOR:        InterpretTernaryOperator(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::TYPE:                    InterpretType(lexemes, lexemeIndex, outputGlsl); break;
    case TokenClass_t::VARIABLE_NAME:           InterpretVariableName(lexemes, entryFunctionName, originalTextureNames, lexemeIndex, outputGlsl); break;
    }
}

void InterpretArithmeticOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // +, -, *, /, %. Check for assignation on next lexeme, or if prefix increment/decrement
    const Lexeme& lexeme = lexemes[lexemeIndex];
    
    // Prefix increment/decrement?
    if (lexeme.m_Token == "+" || lexeme.m_Token == "-")
    {
        if (lexemeIndex < lexemes.size() - 1)
        {
            const Lexeme& nextLexeme = lexemes[lexemeIndex + 1];

            if (nextLexeme.m_Token == lexeme.m_Token)
            {
                outputGlsl += " " + lexeme.m_Token + lexeme.m_Token;
                lexemeIndex += 1;
                return;
            }
        }
    }

    // Simple arithmetic operator
    outputGlsl += " " + lexeme.m_Token + " ";
}

void InterpretAssignation(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    const Lexeme& lexeme = lexemes[lexemeIndex];

    // Simple assignation
    outputGlsl += " " + lexeme.m_Token + " ";
}

void InterpretBitwiseOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // ~, |, &, ^
    const Lexeme& lexeme = lexemes[lexemeIndex];

    // Have to check if it is a relationnal operator (for | and &)
    if (lexeme.m_Token == "|" || lexeme.m_Token == "&")
    {
        if (lexemeIndex < lexemes.size() - 1)
        {
            const Lexeme& nextLexeme = lexemes[lexemeIndex];

            if (nextLexeme.m_TokenClass == TokenClass_t::BITWISE_OPERATOR)
            {
                if (nextLexeme.m_Token == lexeme.m_Token)
                {
                    outputGlsl += " " + lexeme.m_Token + lexeme.m_Token + " ";
                    lexemeIndex += 1;
                    return;
                }
            }
        }
    }

    // Check for ~=, |=, &=, ^=
    if (lexemeIndex < lexemes.size() - 1)
    {
        const Lexeme& nextLexeme = lexemes[lexemeIndex];

        if (nextLexeme.m_TokenClass == TokenClass_t::ASSIGNATION)
        {
            outputGlsl += " " + lexeme.m_Token + nextLexeme.m_Token + " ";
            lexemeIndex += 1;
            return;
        }
    }

    outputGlsl += " " + lexeme.m_Token;
}

void InterpretMulOperation(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl, size_t levelToStartAt, size_t levelToReach)
{
    // Start is the paranthesis
    outputGlsl += "(";

    size_t idx = 0;
    size_t level = levelToStartAt;
    while (true)
    {
        const Lexeme& nextLexeme = lexemes[lexemeIndex + 2 + idx];
        if (nextLexeme.m_TokenClass == TokenClass_t::BUILTIN_FUNCTION)
        {
            if (nextLexeme.m_Token == "mul")
            {
                size_t tempIdx = lexemeIndex + 2 + idx;
                string mulOutput = "";

                InterpretMulOperation(lexemes, tempIdx, mulOutput, level + 1, level + 1);
                level += 1;

                outputGlsl += mulOutput;

                idx += (tempIdx - (lexemeIndex + 2 + idx)) + 1;
            }
            else
            {
                size_t index = 0;
                size_t size = _countof(hlslBuiltinFunctionsToGlsl);
                for (; index < size; index++)
                {
                    if (nextLexeme.m_Token == hlslBuiltinFunctionsToGlsl[index])
                    {
                        break;
                    }
                }

                // There are a few special cases
                //  1. mul in HLSL is simply the * operator in GLSL
                //  2. saturate in HLSL is min(1.0, max(0.0, x)) where x is the value to saturate
                if (nextLexeme.m_Token == "saturate")
                {
                    // TODO
                    // For now we assume that the three next lexemes are a opened paranthesis, a variable name and a closed paranthesis
                    outputGlsl += " " + hlslBuiltinFunctionsToGlsl[index + 1] + lexemes[lexemeIndex + 2 + idx + 2].m_Token + ")";
                    idx += 3;
                }
                else
                {
                    outputGlsl += " " + hlslBuiltinFunctionsToGlsl[index + 1];
                }

                idx += 1;
            }

            continue;
        }
        else if (nextLexeme.m_TokenClass == TokenClass_t::TYPE)
        {
            size_t index = 0;
            size_t size = _countof(hlslTypesMappingToGlsl);
            for (; index < size; index++)
            {
                if (nextLexeme.m_Token == hlslTypesMappingToGlsl[index])
                {
                    break;
                }
            }

            outputGlsl += hlslTypesMappingToGlsl[index + 1] + " ";
            idx += 1;
            continue;
        }
        else if (nextLexeme.m_TokenClass == TokenClass_t::CLOSED_PARANTHESIS)
        {
            level -= 1;
        }
        else if (nextLexeme.m_TokenClass == TokenClass_t::OPENED_PARANTHESIS)
        {
            level += 1;
        }
        else if (nextLexeme.m_TokenClass == TokenClass_t::COMMA && level == levelToReach)
        {
            break;
        }

        outputGlsl += nextLexeme.m_Token;

        idx += 1;
    }
        
    outputGlsl += " * ";
    lexemeIndex += 2 + idx;
}

void InterpretBuiltinFunction(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    const Lexeme& lexeme = lexemes[lexemeIndex];

    size_t index = 0;
    size_t size = _countof(hlslBuiltinFunctionsToGlsl);
    for (; index < size; index++)
    {
        if (lexeme.m_Token == hlslBuiltinFunctionsToGlsl[index])
        {
            break;
        }
    }

    // There are a few special cases
    //  1. mul in HLSL is simply the * operator in GLSL
    //  2. saturate in HLSL is min(1.0, max(0.0, x)) where x is the value to saturate
    if (lexeme.m_Token == "saturate")
    {
        // TODO
        // For now we assume that the three next lexemes are a opened paranthesis, a variable name and a closed paranthesis
        outputGlsl += " " + hlslBuiltinFunctionsToGlsl[index + 1] + lexemes[lexemeIndex + 2].m_Token + ")";
        lexemeIndex += 3;
    }
    else if (lexeme.m_Token == "mul")
    {
        InterpretMulOperation(lexemes, lexemeIndex, outputGlsl, 1, 1);
    }
    else
    {
        outputGlsl += " " + hlslBuiltinFunctionsToGlsl[index + 1];
    }
}

void InterpretCbuffer(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // A cbuffer is a uniform block. We first have to get the index of the register to properly set the layout index
    const Lexeme& lexeme = lexemes[lexemeIndex];

    string registerSlot = lexemes[lexemeIndex + 5].m_Token.substr(1, string::npos);

    outputGlsl += "layout(binding = " + registerSlot + ") uniform " + lexemes[lexemeIndex + 1].m_Token  + "\n";
    lexemeIndex += 6;
}

void IntrepretClosedAngleBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token;
}

void InterpretClosedCurlyBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, bool isVertexShader, string& outputGlsl)
{
    if (isInEntryFunction)
    {
        entryFunctionLevel -= 1;

        if (entryFunctionLevel == 0)
        {
            isInEntryFunction = false;
        }
    }

    // Check if we have to output the struct
    if (insideOfStruct)
    {
        if (hadAnySemanticsInStruct)
        {
            for (size_t i = 0; i < semantics.size(); i++)
            {
                if (isVertexShader && !isOutputSemanticStruct)
                {
                    outputGlsl += "layout (location=" + to_string(i) + ") ";
                }
                
                if (isOutputSemanticStruct)
                {
                    outputGlsl += "out ";
                }
                else
                {
                    outputGlsl += "in ";
                }

                outputGlsl += semantics[i];
            }

            semantics.clear();
            hadAnySemanticsInStruct = false;
            isOutputSemanticStruct = false;

            lexemeIndex += 1;
            outputGlsl += "\n";
        }
        else
        {
            outputGlsl += structBufferIfNoSemanticsInStruct;
            currentIndentationLevel -= 1;
        }

        structBufferIfNoSemanticsInStruct = "";
    }
    else
    {
        outputGlsl += lexemes[lexemeIndex].m_Token;
        currentIndentationLevel -= 1;
    }

    insideOfStruct = false;
    mightAddSemanticStructNameToIgnore = false;
}

void IntrepretClosedParanthesis(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token;
}

void InterpretColon(const vector<Lexeme>& lexemes, size_t& lexemeIndex, bool isVertexShader, string& outputGlsl)
{
    if (insideOfStruct)
    {
        hadAnySemanticsInStruct = true;

        size_t index = lexemeIndex - 2;
        string semanticType;
        string semanticName;

        insideOfStruct = false;

        InterpretType(lexemes, index, semanticType);
        index += 1;

        InterpretVariableName(lexemes, "", vector<string>(), index, semanticName);

        insideOfStruct = true;

        bool ignoreFollowingSemantic = false;

        if (isVertexShader)
        {
            if (lexemes[lexemeIndex + 1].m_Token == "SV_POSITION")
            {
                glPositionName = lexemes[lexemeIndex - 1].m_Token;
                isOutputSemanticStruct = true;

                ignoreFollowingSemantic = true;
            }
        }
        else
        {
            if (lexemes[lexemeIndex + 1].m_Token.find("SV_TARGET") != string::npos)
            {
                isOutputSemanticStruct = true;
            }
        }

        if (!ignoreFollowingSemantic)
        {
            semantics.push_back(semanticType + semanticName + ";\n");
        }

        lexemeIndex += 2;

        if (mightAddSemanticStructNameToIgnore)
        {
            semanticStructNameToIgnore.push_back(structNames.back());
            mightAddSemanticStructNameToIgnore = false;
        }
    }
    else
    {
        outputGlsl += lexemes[lexemeIndex].m_Token + " ";
    }
}

void InterpretComma(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token + " ";
}

void InterpretComment(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token + "\n";
}

void InterpretFlowControl(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    if (isInEntryFunction)
    {
        if (lexemes[lexemeIndex].m_Token == "return")
        {
            lexemeIndex += 2;
            return;
        }
    }

    outputGlsl += lexemes[lexemeIndex].m_Token + " ";
}

void InterpretOpenedAngleBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token + " ";
}

void InterpretOpenedCurlyBracket(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    if (isInEntryFunction)
    {
        entryFunctionLevel += 1;
    }

    if (!insideOfStruct)
    {
        outputGlsl += lexemes[lexemeIndex].m_Token + "\n";
        currentIndentationLevel += 1;
        startOfLine = true;
    }
    else
    {
        structBufferIfNoSemanticsInStruct += lexemes[lexemeIndex].m_Token + "\n";
    }
}

void InterpretOpenedParanthesis(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // Special case: casting. Casting can be in two forms:
    //  1. (float3)val;                     -> Simple cast of a single variable
    //  2. (float3)(mul(val + val2, ...))   -> more complex version
    const Lexeme& nextLexeme = lexemes[lexemeIndex + 1];
    const Lexeme& thirdLexeme = lexemes[lexemeIndex + 2];
    const Lexeme& variableLexeme = lexemes[lexemeIndex + 3];
    const Lexeme& semiColonLexeme = lexemes[lexemeIndex + 4];
    if (nextLexeme.m_TokenClass == TokenClass_t::TYPE && thirdLexeme.m_TokenClass == TokenClass_t::CLOSED_PARANTHESIS)
    {
        size_t index = 0;
        size_t size = _countof(hlslTypesMappingToGlsl);
        for (; index < size; index++)
        {
            if (nextLexeme.m_Token == hlslTypesMappingToGlsl[index])
            {
                break;
            }
        }

        if (variableLexeme.m_TokenClass == TokenClass_t::VARIABLE_NAME)
        {
            outputGlsl += hlslTypesMappingToGlsl[index + 1] + "(" + variableLexeme.m_Token + ")";
            lexemeIndex += 3;
        }
        else
        {
            outputGlsl += hlslTypesMappingToGlsl[index + 1];
            lexemeIndex += 2;
        }
    }
    else
    {
        outputGlsl += lexemes[lexemeIndex].m_Token + " ";
    }
}

void InterpretRegister(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
}

void InterpretRelationalOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // >, <, !
    const Lexeme& lexeme = lexemes[lexemeIndex];

    // Check for >=, <=, !=, or >>, <<, >>=, <<=
    if (lexemeIndex < lexemes.size() - 1)
    {
        const Lexeme& nextLexeme = lexemes[lexemeIndex + 1];

        if (nextLexeme.m_TokenClass == TokenClass_t::ASSIGNATION)
        {
            outputGlsl += " " + lexeme.m_Token + nextLexeme.m_Token + " ";
            lexemeIndex += 1;
            return;
        }
        else if (nextLexeme.m_TokenClass == TokenClass_t::RELATIONAL_OPERATOR)
        {
            if (nextLexeme.m_TokenClass == lexeme.m_TokenClass)
            {
                const Lexeme& thirdLexeme = lexemes[lexemeIndex + 2];

                if (thirdLexeme.m_TokenClass == TokenClass_t::ASSIGNATION)
                {
                    outputGlsl += " " + lexeme.m_Token + nextLexeme.m_Token + thirdLexeme.m_Token + " ";
                    lexemeIndex += 2;
                }
                else
                {
                    outputGlsl += " " + lexeme.m_Token + nextLexeme.m_Token + " " ;
                    lexemeIndex += 1;
                }

                return;
            }
        }
    }

    // Simple relational operator
    outputGlsl += " " + lexeme.m_Token + " ";
}

void InterpretSamplerState(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    lexemeIndex += 7;
}

void InterpretSemiColumn(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    if (insideOfStruct)
    {
        structBufferIfNoSemanticsInStruct += lexemes[lexemeIndex].m_Token + "\n";
    }
    else
    {
        outputGlsl += lexemes[lexemeIndex].m_Token + "\n";
        startOfLine = true;
    }
}

void InterpretStruct(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    // Assume that the next lexemes is the struct name
    const Lexeme& lexeme = lexemes[lexemeIndex];

    string structName = lexemes[lexemeIndex + 1].m_Token;
    structNames.push_back(structName);

    insideOfStruct = true;
    mightAddSemanticStructNameToIgnore = true;

    structBufferIfNoSemanticsInStruct += lexeme.m_Token + " " + structName + "\n";
    lexemeIndex += 1;
}

void InterpretStructureOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += lexemes[lexemeIndex].m_Token;
}

void InterpretTexture(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    lexemeIndex += 7;
}

void InterpretTernaryOperator(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    outputGlsl += " " + lexemes[lexemeIndex].m_Token;
}

void InterpretType(const vector<Lexeme>& lexemes, size_t& lexemeIndex, string& outputGlsl)
{
    const Lexeme& lexeme = lexemes[lexemeIndex];

    size_t index = 0;
    size_t size = _countof(hlslTypesMappingToGlsl);
    for (; index < size; index++)
    {
        if (lexeme.m_Token == hlslTypesMappingToGlsl[index])
        {
            break;
        }
    }

    if (insideOfStruct)
    {
        structBufferIfNoSemanticsInStruct += "    " + hlslTypesMappingToGlsl[index + 1] + " ";
    }
    else
    {
        outputGlsl += hlslTypesMappingToGlsl[index + 1] + " ";
    }
}

void InterpretVariableName(const vector<Lexeme>& lexemes, const string& entryFunctionName, const vector<string>& originalTextureNames, size_t& lexemeIndex, string& outputGlsl)
{
    const Lexeme& lexeme = lexemes[lexemeIndex];

    // Special case for the SV_POSITION semantic
    if (isInEntryFunction && glPositionName != "")
    {
        if (lexeme.m_Token == glPositionName)
        {
            outputGlsl += "gl_Position";
            return;
        }
    }

    // Special case for texture names : if we find one of the original texture names, check if there is a dot, then the Sample keyword followed by a (
    if (find(originalTextureNames.begin(), originalTextureNames.end(), lexeme.m_Token) != originalTextureNames.end())
    {
        if (lexemes[lexemeIndex + 1].m_TokenClass == TokenClass_t::STRUCTURE_OPERATOR &&
            lexemes[lexemeIndex + 2].m_Token == "Sample" && lexemes[lexemeIndex + 3].m_TokenClass == TokenClass_t::OPENED_PARANTHESIS)
        {
            string samplerStateTextureName = GetSamplerStateTextureName(lexemes[lexemeIndex + 4].m_Token, lexeme.m_Token);
            if (samplerStateTextureName != "")
            {
                outputGlsl += "texture(" + samplerStateTextureName;
                lexemeIndex += 4;
                return;
            }
        }
    }

    if (insideOfStruct)
    {
        structBufferIfNoSemanticsInStruct += lexeme.m_Token + (IsStructName(lexeme.m_Token) ? " " : "");
    }
    else
    {
        // Special case for semantic variable names to ignore. We want to remove the name and the dot after it
        if (IsSemanticStructVariable(lexeme.m_Token))
        {
            lexemeIndex += 1;
            return;
        }

        // Special case for entry function. If this variable name is a struct name, check if the next lexeme is the entry function.
        if (IsSemanticStructName(lexeme.m_Token))
        {
            const Lexeme& nextLexeme = lexemes[lexemeIndex + 1];
            if (nextLexeme.m_Token == entryFunctionName)
            {
                // Don't forget to get the semantic variable declaration inside of the entry function
                semanticStructVariableToIgnore.push_back(lexemes[lexemeIndex + 4].m_Token);

                outputGlsl += "void " + entryFunctionName + "()\n";
                lexemeIndex += 5;
                
                isInEntryFunction = true;
            }
            else if (nextLexeme.m_TokenClass == TokenClass_t::VARIABLE_NAME)
            {
                // This means that it is a semantic variable declaration. Add it to the ignore list
                semanticStructVariableToIgnore.push_back(nextLexeme.m_Token);
                lexemeIndex += 2;
            }
        }
        else
        {
            outputGlsl += lexeme.m_Token + (IsStructName(lexeme.m_Token) ? " " : "");
        }
    }
}

}