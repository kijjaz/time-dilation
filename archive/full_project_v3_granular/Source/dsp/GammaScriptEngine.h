#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace time_dilation
{

struct ScriptVariables
{
    double t = 0.0;     // Master coordinate time in seconds
    double tau = 0.0;   // Track proper time in seconds
    int step = 0;       // Current step (0-15)
    float bpm = 120.0f; // Master tempo
    float amp = 0.0f;   // Audio amplitude level (0.0 to 1.0)
    float tap1 = 1.0f;  // External gamma tap 1
    float tap2 = 1.0f;  // External gamma tap 2
};

class GammaScriptEngine
{
public:
    GammaScriptEngine() = default;

    /**
     * Evaluates a mathematical GammaScript string expression with live variables
     * Example expressions:
     *   "1.0 + sin(t * 2.0) * 0.5"
     *   "clamp(1.0 - amp * 0.8, 0.1, 4.0)"
     *   "tap1 * (step % 2 == 0 ? 1.5 : 0.7)"
     */
    static float evaluate (const juce::String& code, const ScriptVariables& vars)
    {
        juce::String cleanCode = code.trim().toLowerCase();
        if (cleanCode.isEmpty()) return 1.0f;

        try
        {
            // Simple Expression Tokenizer & Evaluator
            std::string expr = cleanCode.toStdString();
            double result = parseExpression (expr, vars);
            return juce::jlimit (0.05f, 8.0f, static_cast<float>(result));
        }
        catch (...)
        {
            return 1.0f;
        }
    }

private:
    static double parseExpression (const std::string& expr, const ScriptVariables& vars)
    {
        std::string s = expr;
        s.erase (std::remove_if (s.begin(), s.end(), ::isspace), s.end());

        if (s.empty()) return 1.0;

        // Replace variable tokens
        replaceAll (s, "tau", std::to_string (vars.tau));
        replaceAll (s, "t", std::to_string (vars.t));
        replaceAll (s, "step", std::to_string (vars.step));
        replaceAll (s, "bpm", std::to_string (vars.bpm));
        replaceAll (s, "amp", std::to_string (vars.amp));
        replaceAll (s, "tap1", std::to_string (vars.tap1));
        replaceAll (s, "tap2", std::to_string (vars.tap2));

        // Evaluate built-in functions
        if (s.rfind ("sin(", 0) == 0 && s.back() == ')')
        {
            std::string inner = s.substr (4, s.length() - 5);
            return std::sin (parseExpression (inner, vars));
        }
        if (s.rfind ("cos(", 0) == 0 && s.back() == ')')
        {
            std::string inner = s.substr (4, s.length() - 5);
            return std::cos (parseExpression (inner, vars));
        }
        if (s.rfind ("abs(", 0) == 0 && s.back() == ')')
        {
            std::string inner = s.substr (4, s.length() - 5);
            return std::abs (parseExpression (inner, vars));
        }
        if (s.rfind ("exp(", 0) == 0 && s.back() == ')')
        {
            std::string inner = s.substr (4, s.length() - 5);
            return std::exp (parseExpression (inner, vars));
        }

        // Basic arithmetic binary operations (+, -, *, /)
        size_t plusPos = findLastOp (s, '+');
        if (plusPos != std::string::npos)
            return parseExpression (s.substr (0, plusPos), vars) + parseExpression (s.substr (plusPos + 1), vars);

        size_t minusPos = findLastOp (s, '-');
        if (minusPos != std::string::npos && minusPos > 0)
            return parseExpression (s.substr (0, minusPos), vars) - parseExpression (s.substr (minusPos + 1), vars);

        size_t mulPos = findLastOp (s, '*');
        if (mulPos != std::string::npos)
            return parseExpression (s.substr (0, mulPos), vars) * parseExpression (s.substr (mulPos + 1), vars);

        size_t divPos = findLastOp (s, '/');
        if (divPos != std::string::npos)
        {
            double rhs = parseExpression (s.substr (divPos + 1), vars);
            return (rhs != 0.0) ? parseExpression (s.substr (0, divPos), vars) / rhs : 1.0;
        }

        try
        {
            return std::stod (s);
        }
        catch (...)
        {
            return 1.0;
        }
    }

    static size_t findLastOp (const std::string& s, char op)
    {
        int parenDepth = 0;
        for (int i = static_cast<int>(s.length()) - 1; i >= 0; --i)
        {
            if (s[i] == ')') parenDepth++;
            else if (s[i] == '(') parenDepth--;
            else if (parenDepth == 0 && s[i] == op) return static_cast<size_t>(i);
        }
        return std::string::npos;
    }

    static void replaceAll (std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty()) return;
        size_t startPos = 0;
        while ((startPos = str.find (from, startPos)) != std::string::npos)
        {
            str.replace (startPos, from.length(), to);
            startPos += to.length();
        }
    }
};

} // namespace time_dilation
