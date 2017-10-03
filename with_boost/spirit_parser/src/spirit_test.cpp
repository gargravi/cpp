
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <string>
#include <iostream>
//#include <boost/timer/timer.hpp>
#//include <boost/chrono.hpp>
//#include <boost/date_time.hpp> 

#include <chrono>

namespace lex = boost::spirit::lex;
//using namespace boost::timer;
//#define USE_OSTR_TO_OUT (0)

#define  SP_HAS_STATIC_PARSER (1)

// ============================= SV Known =======================================================
//typedef unsigned long long SV_ID;
typedef uint64_t  SV_ID;
enum TokenTag
{
  OPERATOR_NOT,
  OPERATOR_AND,
  OPERATOR_OR,
  //OPERATOR_ADD,
  //OPERATOR_SUBT,
  //OPERATOR_MULT,
  //OPERATOR_DEVI,
  PRECEDENCE_OPENING_BRACE,
  PRECEDENCE_CLOSING_BRACE,
  OPERAND
};

typedef std::pair< TokenTag, SV_ID >    TokenTagVal;
typedef std::vector< TokenTagVal >      VtrTokenTagVal;
typedef VtrTokenTagVal::iterator        VtrTokenTagVal_Itr;
// ============================= SV Known =======================================================

enum token_ids
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
struct ID_Expression_Lex : lex::lexer<Lexer>
{
  ID_Expression_Lex()
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

struct Expression_Tokens_Handler
{
  Expression_Tokens_Handler(VtrTokenTagVal& vtrparsedTokens, std::ostream& ostr) : m_VtrparsedTokens(vtrparsedTokens), m_ostr(ostr) {}

  VtrTokenTagVal&   m_VtrparsedTokens;
  std::ostream&		m_ostr;

  //<- this is an implementation detail specific to boost::bind and doesn't show 
  //   up in the documentation
  typedef bool result_type;
  //->
  // the function operator gets called for each of the matched tokens
  // c, l, w are references to the counters used to keep track of the numbers
  template <typename Token>
  bool operator()(Token const& t) const
  {
    switch (t.id())
    {
    case TOK_ID_OPERAND_EXPRESSION_ID :
      {
        //std::cout << "Found Expression ID : " << t.value().size() << " VAL: " << t.value() << std::endl;
        // std::cout << "Found OPEN PARA : " << t.value().size() << std::endl;
        const SV_ID expId = boost::lexical_cast<SV_ID>(t.value());
#if USE_OSTR_TO_OUT
		m_ostr << "Found Expression ID : " << expId << " From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
        m_VtrparsedTokens.push_back(std::make_pair(OPERAND, expId));
      }
      break;

    case TOK_ID_OPERATOR_AND :
      {
#if USE_OSTR_TO_OUT
		m_ostr << "Found Operand AND From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
		m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_AND, 0));
      }
      break;

    case TOK_ID_OPERATOR_OR :
      {
#if USE_OSTR_TO_OUT
		m_ostr << "Found Operand OR From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
		m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_OR, 0));
      }
      break;

    case TOK_ID_OPERATOR_NOT :
      {
#if USE_OSTR_TO_OUT
		m_ostr << "Found Operand NOT From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
		m_VtrparsedTokens.push_back(std::make_pair(OPERATOR_NOT, 0));
      }
      break;

    case TOK_ID_PARANTHESES_OPEN :
      {
#if USE_OSTR_TO_OUT
		m_ostr << "Found Syntax Open Paranteses ( From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
		m_VtrparsedTokens.push_back(std::make_pair(PRECEDENCE_OPENING_BRACE, 0));
      }
      break;

    case TOK_ID_PARANTHESES_CLOS :
      {
#if USE_OSTR_TO_OUT
		m_ostr << "Found Syntax Open Paranteses ( From Parsed VAL: " << t.value() << std::endl;
#endif //USE_OSTR_TO_OUT
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
      };
      
    }
    return true;        // always continue to tokenize
  }
};




bool parse_test(std::ostream& ostr, const std::string str_inStr, std::string& finalStr, bool minimal = false)
{
	std::string in_copy_without_spaces;
	if (!minimal)
	{
		in_copy_without_spaces = str_inStr;
		in_copy_without_spaces.erase(remove_if(in_copy_without_spaces.begin(), in_copy_without_spaces.end(), isspace), in_copy_without_spaces.end());
	}

	//<  Specifying `omit` as the token attribute type generates a token class
	//not holding any token attribute at all (not even the iterator range of the
	//matched input sequence), therefore optimizing the token, the lexer, and
	//possibly the parser implementation as much as possible. Specifying
	//`mpl::false_` as the 3rd template parameter generates a token
	//type and an iterator, both holding no lexer state, allowing for even more
	//aggressive optimizations. As a result the token instances contain the token
	//ids as the only data member.
	//>  
	typedef lex::lexertl::token<char const*, lex::omit, boost::mpl::false_> token_type;

	// This defines the lexer type to use
	typedef lex::lexertl::actor_lexer<token_type> lexer_type;


	//Read input from the given file, tokenize all the input, while discarding all generated tokens
	const std::string& str = str_inStr;
	//std::string str = "111   2 33 44  55555 66 ) ";
	//std::string str = "123";

	char const* first = str.c_str();
	char const* last = &first[str.size()];

#if SP_HAS_STATIC_PARSER
	static ID_Expression_Lex<lex::lexertl::lexer<> > id_expression_lex;
#else
	ID_Expression_Lex<lex::lexertl::lexer<> > id_expression_lex;
#endif //SP_HAS_STATIC_PARSER
	//bool r = lex::tokenize(first, last, id_expression_lex, Expression_Tokens_Handler());; // boost::bind(Expression_Tokens_Handler(), _1)  );
	//bool r = lex::tokenize(first, last, id_expression_lex,   boost::bind(Expression_Tokens_Handler(), _1)  );

	VtrTokenTagVal   vtrparsedTokens;
	bool r = lex::tokenize(first, last, id_expression_lex, boost::bind(Expression_Tokens_Handler(vtrparsedTokens, ostr), _1));

	//assert(true == r);
#if SP_HAS_STATIC_PARSER
	//id_expression_lex.initial_state(); // .clear();
#endif //SP_HAS_STATIC_PARSER

	//id_expression_lex.r
	if (!minimal)
	{
		std::stringstream ss;
		ostr << std::endl << "Done paring following is the vector." << std::endl;
		BOOST_FOREACH(const TokenTagVal& tknPair, vtrparsedTokens)
		{
			ostr << tknPair.first << "-" << tknPair.second << std::endl;
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
		}
		finalStr = ss.str();
		return (0 == strcmp(finalStr.c_str(), in_copy_without_spaces.c_str()));
	}
	else
	{
		return true;
	}
}


#define TEST_PARING_STRING(X_IN_STR)  {\
    std::stringstream this_st; \
	std::string finalStr; \
	std::cout << " Going for Testing : " << X_IN_STR ; \
	if ( parse_test(this_st, X_IN_STR, finalStr)) \
	{ \
		std::cout << " ==> SUCESS TO PARSE : PARSED STRING: " << finalStr << std::endl ; \
	} \
	else \
	{ \
		std::cout << " ==> !!!FAILED!!! TO PARSE : PARSED STRING: " << finalStr << std::endl << this_st.str() << std::endl; \
	} \
}

//assert(0); \

using namespace std;

int main(int argc, char* argv[])
{
	//TEST_PARING_STRING(" ( NT111 ( 2)33(44) 55555(66 ) ");

	//std::cout
	TEST_PARING_STRING(" ( 111 ( 2)33(44) 55555(66 ) ");
	
	//This should fail
	TEST_PARING_STRING(" ( NT111 ( 2)33(44) 55555(66 ) ");

	
	TEST_PARING_STRING(" NOT(1AND2) ");
	TEST_PARING_STRING(" NOT(1OR2) ");
	TEST_PARING_STRING("88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666");
	TEST_PARING_STRING(" NOT( 1111111111AND2222222222) OR 33333 ");
	TEST_PARING_STRING("NOTANDOR NOTORAND ANDORNOT ANDNOTOR ORANDNOT ORNOTAND (111AND222) (111OR222)  (111NOT222) (())()");
	TEST_PARING_STRING("88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666");
	TEST_PARING_STRING("12345678901234567890");
	//TEST_PARING_STRING("12345678901234567890123456789");
	TEST_PARING_STRING("NOTANDOR\nNOTORAND\tANDORNOT ANDNOTOR ORANDNOT ORNOTAND (\t111AND222) (111OR222\t)  (\n111NOT222) (())()");

	//cpu_timer cc;
	std::stringstream sst;
	//cc.start();
	std::string tmp;

	chrono::steady_clock sc;   // create an object of `steady_clock` class
	auto start = sc.now();
	const int thrshold = 100; // 100 * 1000;
	for (int iCntr = 0; iCntr < thrshold; ++iCntr)
	{
		parse_test(sst, "88(NOTANDOR) )NOTORAND( (111(222(333)444)555)666", tmp, true);
	}
	//cpu_times elp = cc.elapsed();
	auto end = sc.now();       // end timer (starting & ending is done by measuring the time at the moment the process started & ended respectively)
	auto time_span = static_cast<chrono::duration<double>>(end - start);   // measure time span between start & end
	cout << "Operation took: " << time_span.count() << " seconds or " << thrshold << " operations";

	//std::cout << 
	return 0;
}