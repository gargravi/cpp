
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/bind.hpp>
#include <riskfort/smartvariable/ExpressionParser.h>

namespace SV { 

#define SVEP_ENABLE_SELF_TEST   (1)

//typedef MutexObject 

enum SVEP_TOKEN_IDS
{
  TOK_ID_OPERAND_EXPRESSION_ID = 1000,
  TOK_ID_OPERATOR_AND,
  TOK_ID_OPERATOR_OR,
  TOK_ID_OPERATOR_NOT,
  TOK_ID_PARANTHESES_OPEN,
  TOK_ID_PARANTHESES_CLOS,
  TOK_ID_SPACE_CHARS
};

template <typename Lexer>
struct SVEP_ExpressionLex : lex::lexer<Lexer>
{
  SVEP_ExpressionLex()
  {
    this->self.add
    ("[0-9]+", TOK_ID_OPERAND_EXPRESSION_ID)
    ("AND", TOK_ID_OPERATOR_AND)
    ("OR", TOK_ID_OPERATOR_OR)
    ("NOT", TOK_ID_OPERATOR_NOT)
    ("\\(", TOK_ID_PARANTHESES_OPEN)
    ("\\)", TOK_ID_PARANTHESES_CLOS)
    ("[ \\t\\r\\n]+", TOK_ID_SPACE_CHARS);
  }
};

struct SVEP_TokenHandler
{
  SVEP_TokenHandler(VtrTokenTagVal& vtrparsedTokens, std::ostream* pOstr=NULL) 
          : m_VtrparsedTokens(vtrparsedTokens), m_pOstr(pOstr) {}

  VtrTokenTagVal&   m_VtrparsedTokens;
  std::ostream&     m_pOstr;

  typedef bool result_type;
  
  template <typename Token>
  bool operator()(Token const& t) const
  {
    switch (t.id())
    {
    case TOK_ID_OPERAND_EXPRESSION_ID :
      {
        const SV_ID expId = boost::lexical_cast<SV_ID>(t.value());
        if(NULL != m_pOstr){ m_pOstr << "Found Expression ID : " << expId << " From Parsed VAL: " << t.value() << std::endl;}
        m_VtrparsedTokens.push_back(std::make_pair(OPERAND, expId));
      }
      break;

    case TOK_ID_OPERATOR_AND :
      { if(NULL != m_pOstr) { m_pOstr << "Found Operand AND From Parsed VAL: " << t.value() << std::endl; }
        m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_AND, 0));
      }
      break;

    case TOK_ID_OPERATOR_OR :
      { if(NULL != m_pOstr) { m_pOstr << "Found Operand OR From Parsed VAL: " << t.value() << std::endl; }
        m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_OR, 0));
      }
      break;

    case TOK_ID_OPERATOR_NOT :
      { if(NULL != m_pOstr) { m_pOstr << "Found Operand NOT From Parsed VAL: " << t.value() << std::endl; }
        m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_NOT, 0));
      }
      break;

    case TOK_ID_PARANTHESES_OPEN :
      { if(NULL != m_pOstr) { m_pOstr << "Found Syntax Open Paranteses ( From Parsed VAL: " << t.value() << std::endl; }
        m_VtrparsedTokens.push_back(std::make_pair(PRECEDENCE_OPENING_BRACE, 0));
      }
      break;

    case TOK_ID_PARANTHESES_CLOS :
      { if(NULL != m_pOstr) { m_pOstr << "Found Syntax Open Paranteses ( From Parsed VAL: " << t.value() << std::endl;}
        m_VtrparsedTokens.push_back(std::make_pair(PRECEDENCE_CLOSING_BRACE, 0));
      }
      break;

    case TOK_ID_SPACE_CHARS :
      {
        ; //do nothing
      }
      break;

      default :
      {
        //TODO: Throw error.
        return false;
      };
      
    }
    return true;        // always continue to tokenize
  }
};

template <typename T_LOCK>
class SVLockGuard
{
public:
  SVLockGuard(MutexObject& objLock) : m_objLock(objLock){ m_objLock.lock(); }
  ~SVLockGuard()
  {
    m_objLock.unlock();
  }

private:
	T_LOCK& m_objLock
};

bool parseEx(VtrTokenTagVal& out_parsedTokens, const std::string& str_inStr, std::ostream& ostr)
{
  bool retValue = false;

  try {
    // Refer : http://www.boost.org/doc/libs/1_65_1/libs/spirit/example/lex/word_count_functor.cpp
    typedef lex::lexertl::token<char const*, lex::omit, boost::mpl::false_> token_type;
    typedef lex::lexertl::actor_lexer<token_type> lexer_type;
    char const* first = str_inStr.c_str();
    char const* last = &first[str_inStr.size()];

    static MutexObject objLock;
    
    SVLockGuard<MutexObject> guard(objLock);
    static SVEP_ExpressionLex<lex::lexertl::lexer<> >* pId_expression_lex = new SVEP_ExpressionLex<lex::lexertl::lexer<> >();
    if(NULL != pId_expression_lex)
    {
      retValue = lex::tokenize(first, last, *id_expression_lex, boost::bind(SVEP_TokenHandler(out_parsedTokens, ostr), _1));
      
      // In case of failure we are recreating the parser //TODO this is just to be on safer side.
      if(!retValue)
      {
        delete pId_expression_lex;
        pId_expression_lex = new SVEP_ExpressionLex<lex::lexertl::lexer<> >();
      }
    }
  }
  catch(...)
  {
    //TODO: log error
    out_parsedTokens.clear();
  }
  
  return retValue;
}

#if SVEP_ENABLE_SELF_TEST

void singeParseTest(const std::string inVal, const bool expected_result, const bool is_perfTest = false)
{
  VtrTokenTagVal out_parsedTokens;
  std::stringstream* pLog_ostr = ( (is_perfTest) ? NULL : new std::stringstream );
  if(NULL != pLog_ostr) { *pLog_ostr << "Going for Test of  : " << inVal << "  "; }
  bool compareResult = true;
  if( expected_result == parseEx(out_parsedTokens, inVal, pLog_ostr) )
  {
    if(NULL != pLog_ostr) { *pLog_ostr << "   TEST PASS " << inVal << std::endl ; }
    compareResult = !is_perfTest;
  }
  else
  {
    if(NULL != pLog_ostr) { *pLog_ostr << "   TEST FAIL !!! " << inVal << std::endl; }
  }
  
  if(compareResult)
  {
    std::string in_copy_without_spaces = str_inStr;
    in_copy_without_spaces.erase(remove_if(in_copy_without_spaces.begin(), in_copy_without_spaces.end(), isspace), in_copy_without_spaces.end());
    
    std::stringstream ss;
    if(NULL != pLog_ostr) { *pLog_ostr << std::endl << "Done paring following is the vector." << std::endl; }
    BOOST_FOREACH(const TokenTagVal& tknPair, vtrparsedTokens)
    {
      if(NULL != pLog_ostr) { *pLog_ostr << tknPair.first << "-" << tknPair.second << std::endl; }
      switch (tknPair.first)
      {
      case OPERATOR_NOT: {   ss << "NOT";   }  break;
      case OPERATOR_AND: {   ss << "AND";   }  break;
      case OPERATOR_OR: {   ss << "OR";   }  break;
      case PRECEDENCE_OPENING_BRACE: {   ss << "(";   }  break;
      case PRECEDENCE_CLOSING_BRACE: {   ss << ")";   }  break;
      case OPERAND: {   ss << tknPair.second;   }  break;
      default:
        ss << "ERROR TOKEN";
        ;
      }
      assert( 0 == strcmp(ss.str().c_str(), in_copy_without_spaces.c_str()) );
    }
  }
}

void functionalTest()
{
  // Invalid Input
  singeParseTest(" ( NT 111 ( 2)33(44) 55555(66 ) ", false);
  
  //Valid Input
  singeParseTest(" ( 111 ( 2)33(44) 55555(66 ) ", true);
  singeParseTest(" NOT(1AND2) ", true);
  singeParseTest(" NOT(1OR2) ", true);
  singeParseTest("88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666", true);
  singeParseTest(" NOT( 1111111111AND2222222222) OR 33333 ", true);
  
  //Invllid Input
  singeParseTest(" asdfsadf sdf csd  ", false);
  singeParseTest("asd123", false);

  //Valid Input
  singeParseTest("NOTANDOR NOTORAND ANDORNOT ANDNOTOR ORANDNOT ORNOTAND (111AND222) (111OR222)  (111NOT222) (())()", true);
  singeParseTest("88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666", true);
  singeParseTest("12345678901234567890"); //Test Large number
  
  // Invalid Input // Only whitespaces
  singeParseTest("", false);
  singeParseTest("  ", false);
  
  //Valid Input // Test other whitespaces
  singeParseTest("NOTANDOR\nNOTORAND\tANDORNOT ANDNOTOR ORANDNOT ORNOTAND (\t111AND222) (111OR222\t)  (\n111NOT222) (())()", true);
  
}

void performanceTest()
{

  chrono::steady_clock sc;   // create an object of `steady_clock` class
  auto start = sc.now();
  const int thrshold = 1000 * 1000; //test for a million
  for (int iCntr = 0; iCntr < thrshold; ++iCntr)
  {
    singeParseTest("88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666", true, true);
  } 
  auto end = sc.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
  auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end
  std::cout << "Operation took: " << time_span.count() << " seconds or " << thrshold << " operations";
}

struct performSelfTest() {
  performSelfTest() {
    functionalTest();
    performanceTest();
  }
}
#endif // SVEP_ENABLE_SELF_TEST

bool ExpressionParser::parse(const SV_String& strIDExpression, VtrTokenTagVal& out_parsedTokens)
{
  #if SVEP_ENABLE_SELF_TEST
    static performSelfTest slftest;
  #endif // SVEP_ENABLE_SELF_TEST

  return parseEx(out_parsedTokens, strIDExpression, NULL);

}

};