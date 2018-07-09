
#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>

int main()
{
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	std::string s = "  ( 11 OR 22) AND 33)";
	//boost::char_separator<char> sep{" ", "+", boost::keep_empty_tokens};
	//boost::char_separator<char> sep{" ", "+"};
	boost::char_separator<char> sep{")", "("};
	tokenizer tok{s, sep};
	for (const auto &t : tok)
		std::cout << t << '\n';
}

/*
#include <iostream>
#include <boost/function.hpp>
#include "Value.h"

template <typename Type , typename Arg1>
class DataFetcher_1argFunc 
{
	public:
		typedef boost::function< Type (Arg1 in_arg) > Callback;

	private:
		Callback m_callback;
		Arg1 m_arg;

	public:
		DataFetcher_1argFunc( const Callback& callback, const Arg1& arg) : m_callback(callback), m_arg(arg) {}

		Type fetch() const { return m_callback(m_arg); }
}; 

template <typename Type>
Type getMeType(const std::string& str)
{
	return boost::lexical_cast<Type> ( str ) ;
}


int main(int argc, char* argv[])
{
	SV::Value val(20);
	std::cout << " val(20) : " << val << std::endl;
	{	
	SV::Value val("Hello");
	std::cout << " val(Hello) : " << val << std::endl;
	}

	std::string str_40("40");
	DataFetcher_1argFunc<int,std::string> fnobj(  &getMeType<int>, str_40 );
	std::cout << " DataFetcher : " << fnobj.fetch() << std::endl;

	return 0;
}
*/
