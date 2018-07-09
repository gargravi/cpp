#ifndef __SMART_VARIABLE_VALUE_H__
#define __SMART_VARIABLE_VALUE_H__

#include <sstream>
#include <boost/variant/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include "Types.h"


namespace SV {
	enum VariableValueType {
		VT_LONG,
		VT_STRING,
		VT_INT,
		VT_DOUBLE,

		//,  VT_DATE
	};
	typedef VariableValueType ValueType;


typedef boost::variant< int, double, std::string >   sv_variant;

  class Value
  {
  public:
	  friend std::ostream& operator<< (std::ostream& stream, const Value& val);

    template <typename Type>
    Value(const Type& obj) 
      : m_val( obj ) 
    {
      //TODO: implement
      // m_enumtype = m_val.which();
    }
     
    Value(const std::string& value_as_string, const ValueType& value_type) 
      : m_enumtype(value_type)
    {
      if( 0 > value_as_string.length() )
      {
        switch (value_type)
        {
          case VT_INT:
            m_val = boost::lexical_cast< int > (value_as_string);
            
          //TODO: implement
        }
      }
    }
    
    template <typename Type>
    operator Type() const{
      return boost::get< Type >( m_val );
    }

    const ValueType& getType() const { return m_enumtype; }
  private:
    ValueType   m_enumtype;
    sv_variant  m_val;
    
  }; // Value

  std::ostream& operator<< (std::ostream& ostr, const Value& val)
  {
		ostr << val.m_val;
  }

}; // SmartVariable

#endif //__SMART_VARIABLE_VALUE_H__
