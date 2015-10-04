#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
using namespace std;

namespace HlslToGlsl
{

enum TokenClass_t
{
    TYPE,
    VARIABLE_NAME,
    SEMICOLUMN,
    OPENED_CURLY_BRACKET,
    CLOSED_CURLY_BRACKET,
    COMMA,
    OPENED_ANGLE_BRACKET,
    CLOSED_ANGLE_BRACKET,
    OPENED_PARANTHESIS,
    CLOSED_PARANTHESIS,
    ARITHMETIC_OPERATOR,
    RELATIONAL_OPERATOR,
    ASSIGNATION,
    BUILTIN_FUNCTION,
    COLON,
    CBUFFER,
    REGISTER,
    FLOW_CONTROL,
    STRUCT,
    BITWISE_OPERATOR,
    STRUCTURE_OPERATOR,
    TERNARY_OPERATOR,
    COMMENT,
    SAMPLER_STATE,
    TEXTURE,
};

struct Lexeme
{
    TokenClass_t m_TokenClass;
    string m_Token;
};

vector<Lexeme> ParseIntoLexemes(const string& input);

}

#endif