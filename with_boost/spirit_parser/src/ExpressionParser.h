#ifndef __SMART_VARIABLE_EXPRESSION_PARSER_H__
#define __SMART_VARIABLE_EXPRESSION_PARSER_H__

#include <riskfort/smartvariable/PredicateFactory.h> //TODO: bifercate this

namespace SV { 

class ExpressionParser
{
public:
  static bool parse(const SV_String& strIDExpression, VtrTokenTagVal& out_parsedTokens);

}; // ExpressionBuilder

}; // SmartVariable

#endif //__SMART_VARIABLE_EXPRESSION_PARSER_H__