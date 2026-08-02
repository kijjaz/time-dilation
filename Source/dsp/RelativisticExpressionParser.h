#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>

namespace time_dilation
{

using TapResolver = std::function<double(const std::string& target)>;

class RelativisticExpressionParser
{
public:
    RelativisticExpressionParser() = default;
    ~RelativisticExpressionParser() = default;

    static double evaluateExpression (const std::string& expressionStr,
                                     const std::map<std::string, double>& variables,
                                     TapResolver tapResolver = nullptr);

    static std::string evaluateStringExpression (const std::string& expressionStr,
                                                 const std::map<std::string, std::string>& stringVars,
                                                 const std::map<std::string, double>& numericVars);
};

} // namespace time_dilation
