#pragma once
#include <iostream>

static std::string SanitizeStringLiteral(const std::string& str)
{
    std::string result;
    for (char c : str)
    {
        if (c == '"')
        {
            result += "\"";
        }
        else if (c == '\\')
        {
            result += "\\\\";
        }
        else
        {
            result += c;
        }
    }
    return result;
}