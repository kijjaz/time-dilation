#include "RelativisticExpressionParser.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace time_dilation
{

class MathExprEvaluator
{
public:
    static double parse (const std::string& expr,
                         const std::map<std::string, double>& variables,
                         TapResolver tapResolver)
    {
        if (expr.empty()) return 0.0;

        std::string s = expr;

        // Process tap("...") calls
        size_t tapPos = 0;
        while ((tapPos = s.find ("tap(", tapPos)) != std::string::npos)
        {
            size_t closeParen = s.find (")", tapPos);
            if (closeParen != std::string::npos)
            {
                std::string argStr = s.substr (tapPos + 4, closeParen - (tapPos + 4));
                if (argStr.length() >= 2 && (argStr.front() == '"' || argStr.front() == '\''))
                    argStr = argStr.substr (1, argStr.length() - 2);

                double tappedVal = 0.0;
                if (tapResolver) tappedVal = tapResolver (argStr);

                std::string valStr = std::to_string (tappedVal);
                s.replace (tapPos, closeParen + 1 - tapPos, valStr);
                tapPos += valStr.length();
            }
            else break;
        }

        // Substitute variables ($v1, $t, $gamma, $bpm, etc.)
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
            pos = 0;
            while ((pos = s.find (kv.first, pos)) != std::string::npos)
            {
                bool leftOk = (pos == 0 || !std::isalnum (static_cast<unsigned char>(s[pos - 1])));
                bool rightOk = (pos + kv.first.length() == s.length() || !std::isalnum (static_cast<unsigned char>(s[pos + kv.first.length()])));
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

        size_t pos = 0;
        try {
            return parseExpression (s, pos);
        } catch (...) {
            return 0.0;
        }
    }

private:
    static void skipWhitespace (const std::string& s, size_t& pos)
    {
        while (pos < s.length() && std::isspace (static_cast<unsigned char>(s[pos])))
            pos++;
    }

    static double parseExpression (const std::string& s, size_t& pos)
    {
        return parseAddSub (s, pos);
    }

    static double parseAddSub (const std::string& s, size_t& pos)
    {
        double left = parseMulDiv (s, pos);
        while (pos < s.length())
        {
            skipWhitespace (s, pos);
            if (pos >= s.length()) break;
            char op = s[pos];
            if (op == '+' || op == '-')
            {
                pos++;
                double right = parseMulDiv (s, pos);
                if (op == '+') left += right;
                else left -= right;
            }
            else break;
        }
        return left;
    }

    static double parseMulDiv (const std::string& s, size_t& pos)
    {
        double left = parsePow (s, pos);
        while (pos < s.length())
        {
            skipWhitespace (s, pos);
            if (pos >= s.length()) break;
            char op = s[pos];
            if (op == '*' || op == '/' || op == '%')
            {
                pos++;
                double right = parsePow (s, pos);
                if (op == '*') left *= right;
                else if (op == '/') { if (std::abs (right) > 1e-12) left /= right; else left = 0.0; }
                else if (op == '%') { if (std::abs (right) > 1e-12) left = std::fmod (left, right); else left = 0.0; }
            }
            else break;
        }
        return left;
    }

    static double parsePow (const std::string& s, size_t& pos)
    {
        double left = parseUnary (s, pos);
        skipWhitespace (s, pos);
        if (pos < s.length() && s[pos] == '^')
        {
            pos++;
            double right = parsePow (s, pos);
            left = std::pow (left, right);
        }
        return left;
    }

    static double parseUnary (const std::string& s, size_t& pos)
    {
        skipWhitespace (s, pos);
        if (pos < s.length())
        {
            if (s[pos] == '+')
            {
                pos++;
                return parseUnary (s, pos);
            }
            if (s[pos] == '-')
            {
                pos++;
                return -parseUnary (s, pos);
            }
        }
        return parsePrimary (s, pos);
    }

    static double parsePrimary (const std::string& s, size_t& pos)
    {
        skipWhitespace (s, pos);
        if (pos >= s.length()) return 0.0;

        if (s[pos] == '(')
        {
            pos++;
            double val = parseExpression (s, pos);
            skipWhitespace (s, pos);
            if (pos < s.length() && s[pos] == ')') pos++;
            return val;
        }

        if (std::isalpha (static_cast<unsigned char>(s[pos])))
        {
            size_t start = pos;
            while (pos < s.length() && (std::isalnum (static_cast<unsigned char>(s[pos])) || s[pos] == '_'))
                pos++;
            std::string ident = s.substr (start, pos - start);
            std::transform (ident.begin(), ident.end(), ident.begin(), ::tolower);

            skipWhitespace (s, pos);
            if (pos < s.length() && s[pos] == '(')
            {
                pos++;
                std::vector<double> args;
                if (pos < s.length() && s[pos] != ')')
                {
                    while (true)
                    {
                        args.push_back (parseExpression (s, pos));
                        skipWhitespace (s, pos);
                        if (pos < s.length() && s[pos] == ',') pos++;
                        else break;
                    }
                }
                if (pos < s.length() && s[pos] == ')') pos++;

                if (ident == "sin") return args.size() > 0 ? std::sin (args[0]) : 0.0;
                if (ident == "cos") return args.size() > 0 ? std::cos (args[0]) : 0.0;
                if (ident == "tan") return args.size() > 0 ? std::tan (args[0]) : 0.0;
                if (ident == "asin") return args.size() > 0 ? std::asin (args[0]) : 0.0;
                if (ident == "acos") return args.size() > 0 ? std::acos (args[0]) : 0.0;
                if (ident == "atan") return args.size() > 0 ? std::atan (args[0]) : 0.0;
                if (ident == "sinh") return args.size() > 0 ? std::sinh (args[0]) : 0.0;
                if (ident == "cosh") return args.size() > 0 ? std::cosh (args[0]) : 0.0;
                if (ident == "tanh") return args.size() > 0 ? std::tanh (args[0]) : 0.0;
                if (ident == "sqrt") return args.size() > 0 ? std::sqrt (std::max (0.0, args[0])) : 0.0;
                if (ident == "abs") return args.size() > 0 ? std::abs (args[0]) : 0.0;
                if (ident == "exp") return args.size() > 0 ? std::exp (args[0]) : 0.0;
                if (ident == "log") return args.size() > 0 ? std::log (std::max (1e-12, args[0])) : 0.0;
                if (ident == "log10") return args.size() > 0 ? std::log10 (std::max (1e-12, args[0])) : 0.0;
                if (ident == "log2") return args.size() > 0 ? std::log2 (std::max (1e-12, args[0])) : 0.0;
                if (ident == "pow") return args.size() > 1 ? std::pow (args[0], args[1]) : 0.0;
                if (ident == "round") return args.size() > 0 ? std::round (args[0]) : 0.0;
                if (ident == "floor") return args.size() > 0 ? std::floor (args[0]) : 0.0;
                if (ident == "ceil") return args.size() > 0 ? std::ceil (args[0]) : 0.0;
                if (ident == "min") return args.size() > 1 ? std::min (args[0], args[1]) : (args.size() > 0 ? args[0] : 0.0);
                if (ident == "max") return args.size() > 1 ? std::max (args[0], args[1]) : (args.size() > 0 ? args[0] : 0.0);
                if (ident == "clamp") return args.size() > 2 ? std::clamp (args[0], args[1], args[2]) : 0.0;
                return 0.0;
            }

            if (ident == "pi") return 3.14159265358979323846;
            if (ident == "tau") return 6.28318530717958647692;
            if (ident == "e") return 2.71828182845904523536;

            return 0.0;
        }

        if (std::isdigit (static_cast<unsigned char>(s[pos])) || s[pos] == '.')
        {
            size_t start = pos;
            while (pos < s.length() && (std::isdigit (static_cast<unsigned char>(s[pos])) || s[pos] == '.'))
                pos++;
            try {
                return std::stod (s.substr (start, pos - start));
            } catch (...) {
                return 0.0;
            }
        }

        pos++;
        return 0.0;
    }
};

double RelativisticExpressionParser::evaluateExpression (const std::string& expr,
                                                         const std::map<std::string, double>& variables,
                                                         TapResolver tapResolver)
{
    return MathExprEvaluator::parse (expr, variables, tapResolver);
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
