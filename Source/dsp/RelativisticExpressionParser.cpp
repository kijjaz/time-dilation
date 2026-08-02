#include "RelativisticExpressionParser.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace time_dilation
{

double RelativisticExpressionParser::evaluateExpression (const std::string& expr,
                                                         const std::map<std::string, double>& variables,
                                                         TapResolver tapResolver)
{
    if (expr.empty()) return 0.0;

    std::string s = expr;

    // Process tap(...) Wireless Signal Tapping Functions
    size_t tapPos = 0;
    while ((tapPos = s.find ("tap(", tapPos)) != std::string::npos)
    {
        size_t closeParen = s.find (")", tapPos);
        if (closeParen != std::string::npos)
        {
            std::string argStr = s.substr (tapPos + 4, closeParen - (tapPos + 4));
            // Strip quotes if present (e.g. tap("osc1") or tap('osc1'))
            if (argStr.length() >= 2 && (argStr.front() == '"' || argStr.front() == '\''))
                argStr = argStr.substr (1, argStr.length() - 2);

            double tappedVal = 0.0;
            if (tapResolver)
            {
                tappedVal = tapResolver (argStr);
            }

            std::string valStr = std::to_string (tappedVal);
            s.replace (tapPos, closeParen + 1 - tapPos, valStr);
            tapPos += valStr.length();
        }
        else
        {
            break;
        }
    }

    // Substitute variables (e.g. $t, $gt, $gamma, $v1, $bpm)
    for (const auto& kv : variables)
    {
        std::string varName = "$" + kv.first;
        size_t pos = 0;
        std::string valStr = std::to_string (kv.second);
        while ((pos = s.find (varName, pos)) != std::string::npos)
        {
            s.replace (pos, varName.length(), valStr);
            pos += valStr.length();
        }

        // Also check without $ prefix
        pos = 0;
        while ((pos = s.find (kv.first, pos)) != std::string::npos)
        {
            // Check word boundary
            bool leftOk = (pos == 0 || !std::isalnum (s[pos - 1]));
            bool rightOk = (pos + kv.first.length() == s.length() || !std::isalnum (s[pos + kv.first.length()]));
            if (leftOk && rightOk)
            {
                s.replace (pos, kv.first.length(), valStr);
                pos += valStr.length();
            }
            else
            {
                pos += kv.first.length();
            }
        }
    }

    // Mathematical Functions
    if (s.rfind ("sin(", 0) == 0) return std::sin (evaluateExpression (s.substr (4, s.length() - 5), variables));
    if (s.rfind ("cos(", 0) == 0) return std::cos (evaluateExpression (s.substr (4, s.length() - 5), variables));
    if (s.rfind ("abs(", 0) == 0) return std::abs (evaluateExpression (s.substr (4, s.length() - 5), variables));
    if (s.rfind ("exp(", 0) == 0) return std::exp (evaluateExpression (s.substr (4, s.length() - 5), variables));
    if (s.rfind ("log(", 0) == 0) return std::log (evaluateExpression (s.substr (4, s.length() - 5), variables));
    if (s.rfind ("round(", 0) == 0) return std::round (evaluateExpression (s.substr (6, s.length() - 7), variables));
    if (s.rfind ("floor(", 0) == 0) return std::floor (evaluateExpression (s.substr (6, s.length() - 7), variables));

    // Try direct string-to-double parsing
    try
    {
        return std::stod (s);
    }
    catch (...)
    {
        return 0.0;
    }
}

std::string RelativisticExpressionParser::evaluateStringExpression (const std::string& expr,
                                                                     const std::map<std::string, std::string>& stringVars,
                                                                     const std::map<std::string, double>& numericVars)
{
    std::string result = expr;

    for (const auto& kv : stringVars)
    {
        std::string varName = "$" + kv.first;
        size_t pos = 0;
        while ((pos = result.find (varName, pos)) != std::string::npos)
        {
            result.replace (pos, varName.length(), kv.second);
            pos += kv.second.length();
        }
    }

    for (const auto& kv : numericVars)
    {
        std::string varName = "$" + kv.first;
        size_t pos = 0;
        std::string valStr = std::to_string (kv.second);
        while ((pos = result.find (varName, pos)) != std::string::npos)
        {
            result.replace (pos, varName.length(), valStr);
            pos += valStr.length();
        }
    }

    return result;
}

} // namespace time_dilation
