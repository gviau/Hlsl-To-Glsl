#include "Tokenizer.h"

#include <algorithm>

namespace HlslToGlsl
{

string hlslTypes[] = {
    "bool",
    "int",
    "uint",
    "dword",
    "half",
    "float",
    "double",

    "bool1",
    "bool2",
    "bool3",
    "bool4",

    "int1",
    "int2",
    "int3",
    "int4",

    "uint1",
    "uint2",
    "uint3",
    "uint4",

    "half1",
    "half2",
    "half3",
    "half4",

    "float1",
    "float2",
    "float3",
    "float4",

    "double1",
    "double2",
    "double3",
    "double4",

    "float2x2",
    "float3x3",
    "float4x4"
};

string hlslFunctions[] = {
    "abs",
    "acos",
    "all",
    "any",
    "asin",
    "atan",
    "atan2",
    "ceil",
    "clamp",
    "clip",
    "cos",
    "cosh",
    "cross",
    "ddx",
    "ddx_coarse",
    "ddx_fine",
    "ddy",
    "ddy_coarse",
    "ddy_fine",
    "degrees",
    "determinant",
    "distance",
    "dot",
    "exp",
    "exp2",
    "faceforward",
    "floor",
    "fma",
    "fmod",
    "frac",
    "frexp",
    "fwidth",
    "isfinite",
    "isinf",
    "isnan",
    "ldexp",
    "length",
    "lerp",
    "log",
    "log10",
    "log2",
    "max",
    "min",
    "modf",
    "mul",
    "noise",
    "normalize",
    "pow",
    "radians",
    "rcp",
    "reflect",
    "refract",
    "reversebits",
    "rsqrt",
    "saturate",
    "sign",
    "sin",
    "sinh",
    "smoothstep",
    "sqrt",
    "step",
    "tan",
    "tanh",
    "tex1D",
    "tex1Dbias",
    "tex1Dgrad",
    "tex1Dlod",
    "tex1Dproj",
    "tex2D",
    "tex2Dbias",
    "tex2Dgrad",
    "tex2Dlod",
    "tex2Dproj",
    "tex3D",
    "tex3Dbias",
    "tex3Dgrad",
    "tex3Dlod",
    "tex3Dproj",
    "texCUBE",
    "texCUBEbias",
    "texCUBEgrad",
    "texCUBElod",
    "texCUBEproj",
    "transpose",
    "trunc"
};

string hlslFlowControl[] = {
    "break",
    "continue",
    "discard",
    "do",
    "for",
    "if",
    "switch",
    "case",
    "while",
    "return",
    "else",
    "const",
};

string keywordsToIgnore[] = {
    "static"
};

bool IsHlslType(const string& token);
bool IsHlslFunction(const string& token);
bool IsHlslFlowControl(const string& token);
bool IsKeywordToIgnore(const string& token);
vector<string> Strip(const string& input);
void SeparateAdditionalTokens(const vector<char>& tokensToSeparate, vector<string>& tokens);

vector<Lexeme> ParseIntoLexemes(const string& input)
{
    vector<Lexeme> lexemes;
    
    vector<char> tokensToSeparate;
    tokensToSeparate.push_back('(');
    tokensToSeparate.push_back(')');
    tokensToSeparate.push_back('[');
    tokensToSeparate.push_back(']');
    tokensToSeparate.push_back('{');
    tokensToSeparate.push_back('}');
    tokensToSeparate.push_back(';');
    tokensToSeparate.push_back('+');
    tokensToSeparate.push_back('-');
    tokensToSeparate.push_back('*');
    tokensToSeparate.push_back('/');
    tokensToSeparate.push_back('=');
    tokensToSeparate.push_back('>');
    tokensToSeparate.push_back('<');
    tokensToSeparate.push_back(',');
    tokensToSeparate.push_back('!');
    tokensToSeparate.push_back('.');
    tokensToSeparate.push_back('%');
    tokensToSeparate.push_back('~');
    tokensToSeparate.push_back('&');
    tokensToSeparate.push_back('|');
    tokensToSeparate.push_back('^');
    tokensToSeparate.push_back('?');

    vector<string> tokens = Strip(input);
    SeparateAdditionalTokens(tokensToSeparate, tokens);

    for (size_t i = 0; i < tokens.size(); i++)
    {
        Lexeme lexeme;
        lexeme.m_Token = tokens[i];

        // Make sure there are no null characters at the end
        while (lexeme.m_Token.back() == '\0')
        {
            lexeme.m_Token.pop_back();
        }

        if (IsKeywordToIgnore(lexeme.m_Token))
        {
            continue;
        }

        if (lexeme.m_Token.substr(0, 2) == "//")
        {
            lexeme.m_TokenClass = TokenClass_t::COMMENT;
        }
        else if (IsHlslType(lexeme.m_Token))
        {
            lexeme.m_TokenClass = TokenClass_t::TYPE;
        }
        else if (IsHlslFunction(lexeme.m_Token))
        {
            // Must make sure that next lexeme is a paranthesis, otherwise it does't count
            if (tokens[i + 1] == "(")
            {
                lexeme.m_TokenClass = TokenClass_t::BUILTIN_FUNCTION;
            }
            else
            {
                lexeme.m_TokenClass = TokenClass_t::VARIABLE_NAME;
            }
        }
        else if (IsHlslFlowControl(lexeme.m_Token))
        {
            lexeme.m_TokenClass = TokenClass_t::FLOW_CONTROL;
        }
        else if (lexeme.m_Token == "cbuffer")
        {
            lexeme.m_TokenClass = TokenClass_t::CBUFFER;
        }
        else if (lexeme.m_Token == "register")
        {
            lexeme.m_TokenClass = TokenClass_t::REGISTER;
        }
        else if (lexeme.m_Token == "struct")
        {
            lexeme.m_TokenClass = TokenClass_t::STRUCT;
        }
        else if (lexeme.m_Token == "SamplerState")
        {
            lexeme.m_TokenClass = TokenClass_t::SAMPLER_STATE;
        }
        else if (lexeme.m_Token == "Texture1D" || lexeme.m_Token == "Texture2D" || lexeme.m_Token == "Texture3D")
        {
            lexeme.m_TokenClass = TokenClass_t::TEXTURE;
        }
        else if (lexeme.m_Token == "(")
        {
            lexeme.m_TokenClass = TokenClass_t::OPENED_PARANTHESIS;
        }
        else if (lexeme.m_Token == ")")
        {
            lexeme.m_TokenClass = TokenClass_t::CLOSED_PARANTHESIS;
        }
        else if (lexeme.m_Token == "[")
        {
            lexeme.m_TokenClass = TokenClass_t::OPENED_ANGLE_BRACKET;
        }
        else if (lexeme.m_Token == "]")
        {
            lexeme.m_TokenClass = TokenClass_t::CLOSED_ANGLE_BRACKET;
        }
        else if (lexeme.m_Token == "{")
        {
            lexeme.m_TokenClass = TokenClass_t::OPENED_CURLY_BRACKET;
        }
        else if (lexeme.m_Token == "}")
        {
            lexeme.m_TokenClass = TokenClass_t::CLOSED_CURLY_BRACKET;
        }
        else if (lexeme.m_Token == "=")
        {
            lexeme.m_TokenClass = TokenClass_t::ASSIGNATION;
        }
        else if (lexeme.m_Token == "+" || lexeme.m_Token == "-" || lexeme.m_Token == "*" || lexeme.m_Token == "/" || lexeme.m_Token == "%")
        {
            lexeme.m_TokenClass = TokenClass_t::ARITHMETIC_OPERATOR;
        }
        else if (lexeme.m_Token == ">" || lexeme.m_Token == "<" || lexeme.m_Token == ">=" || lexeme.m_Token == "<=" || lexeme.m_Token == "!")
        {
            lexeme.m_TokenClass = TokenClass_t::RELATIONAL_OPERATOR;
        }
        else if (lexeme.m_Token == ",")
        {
            lexeme.m_TokenClass = TokenClass_t::COMMA;
        }
        else if (lexeme.m_Token == ";")
        {
            lexeme.m_TokenClass = TokenClass_t::SEMICOLUMN;
        }
        else if (lexeme.m_Token == ":")
        {
            lexeme.m_TokenClass = TokenClass_t::COLON;
        }
        else if (lexeme.m_Token == ".")
        {
            lexeme.m_TokenClass = TokenClass_t::STRUCTURE_OPERATOR;
        }
        else if (lexeme.m_Token == "~" || lexeme.m_Token == "&" || lexeme.m_Token == "|" || lexeme.m_Token == "^")
        {
            lexeme.m_TokenClass = TokenClass_t::BITWISE_OPERATOR;
        }
        else if (lexeme.m_Token == "?")
        {
            lexeme.m_TokenClass = TokenClass_t::TERNARY_OPERATOR;
        }
        else
        {
            lexeme.m_TokenClass = TokenClass_t::VARIABLE_NAME;
        }

        lexemes.push_back(lexeme);
    }

    return lexemes;
}

bool IsHlslType(const string& token)
{
    const size_t numberOfHlslTypes = _countof(hlslTypes);

    for (size_t i = 0; i < numberOfHlslTypes; i++)
    {
        if (token == hlslTypes[i])
        {
            return true;
        }
    }

    return false;
}

bool IsHlslFunction(const string& token)
{
    const size_t numberOfHlslFunctions = _countof(hlslFunctions);

    for (size_t i = 0; i < numberOfHlslFunctions; i++)
    {
        if (token == hlslFunctions[i])
        {
            return true;
        }
    }

    return false;
}

bool IsHlslFlowControl(const string& token)
{
    const size_t numberOfHlslFlowControl = _countof(hlslFlowControl);

    for (size_t i = 0; i < numberOfHlslFlowControl; i++)
    {
        if (token == hlslFlowControl[i])
        {
            return true;
        }
    }

    return false;
}

bool IsKeywordToIgnore(const string& token)
{
    const size_t numberOfKeywords = _countof(keywordsToIgnore);

    for (size_t i = 0; i < numberOfKeywords; i++)
    {
        if (token == keywordsToIgnore[i])
        {
            return true;
        }
    }

    return false;
}

vector<string> Strip(const string& input)
{
    vector<string> tokens;

    vector<char> delims;
    delims.push_back(' ');
    delims.push_back('\n');
    delims.push_back('\t');

    bool isComment = false;
    bool mightBeComment = false;

    string current_string = "";
    for (char c : input)
    {
        // Special case for comments : don't strip
        if (isComment)
        {
            if (c == '\n')
            {
                isComment = false;
            }
        }
        else if (c == '/')
        {
            if (mightBeComment)
            {
                isComment = true;
                mightBeComment = false;
            }
            else
            {
                mightBeComment = true;
            }
        }
        else if (mightBeComment)
        {
            mightBeComment = false;
        }

        if (isComment || find(delims.begin(), delims.end(), c) == delims.end())
        {
            current_string += c;
        }
        else
        {
            if (current_string != "")
            {
                tokens.push_back(current_string);
                current_string = "";
            }
        }
    }

    if (current_string != "")
    {
        tokens.push_back(current_string);
    }

    return tokens;
}

void SeparateAdditionalTokens(const vector<char>& tokensToSeparate, vector<string>& tokens)
{
    for (size_t i = tokens.size(); i > 0; i--)
    {
        size_t index = i - 1;
        string token = tokens[index];

        // Special case for comments
        if (token.substr(0, 2) == "//")
        {
            continue;
        }

        size_t posInString = 0;

        auto it = find_if(tokensToSeparate.begin(), tokensToSeparate.end(), [&] (char c) {
            posInString = token.find(c);
            return (posInString != string::npos);
        });

        if (it == tokensToSeparate.end())
        {
            continue;
        }

        // From the position in the string, separate the token in 3: the left part, the actual token, and the right part
        // Then, insert the 3 at the current position in the array and remove the old token.
        string leftPart = token.substr(0, posInString);
        string tokenPart = token.substr(posInString, 1);
        string rightPart = token.substr(posInString + 1, string::npos);

        if ((leftPart == "" || leftPart[0] == '\0') && (rightPart == "" || rightPart[0] == '\0'))
        {
            continue;
        }

        tokens.erase(tokens.begin() + index);
        
        size_t insertionIndex = 0;
        if (leftPart != "")
        {
            tokens.insert(tokens.begin() + index + insertionIndex, leftPart);
            insertionIndex++;
        }

        tokens.insert(tokens.begin() + index + insertionIndex, tokenPart);
        insertionIndex++;

        if (rightPart != "")
        {
            tokens.insert(tokens.begin() + index + insertionIndex, rightPart);
            insertionIndex++;
        }

        i += insertionIndex;
    }
}

}