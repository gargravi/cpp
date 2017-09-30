#ifndef __FUNCTION_CALL_METRIC_CLIENT_H__
#define __FUNCTION_CALL_METRIC_CLIENT_H__

#ifdef FUNCTION_CALL_METRIC_ENABLED

#include <string>
/*
identifier  = fileBaseNae_functionSig_line_Number;
collection
	Metrics
		hit count
		start time  (ns granularity)
		end time    (ns granularity)
	thread wise     (which Thread)
		low over head   
Reporting
	For each idetnfier
		Number of hits
		Max Time
		Min Time
		Average Time
	Frequency or trigger.
	Ouput Reporting format ?
*/

#define FCM_TOKENPASTE(FCM_ARG_1, FCM_ARG_2)		FCM_ARG_1##FCM_ARG_2 
#define FCM_TOKENPASTE2(FCM_ARG_1, FCM_ARG_2)		FCM_TOKENPASTE(FCM_ARG_1, FCM_ARG_2)
#define FCM_PREFIX_LINE(FCM_ARG_1)					FCM_TOKENPASTE2(FCM_ARG_1, __LINE__ )
#define FCM_TOEKN_AS_STRING_2(FCM_ARG_1)            # FCM_ARG_1
#define FCM_TOEKN_AS_STRING(FCM_ARG_1)              FCM_TOEKN_AS_STRING_2(FCM_ARG_1)
#define FCM_TYPE_PREFIX_LINE(FCM_ARG_IN_TYPE, FCM_ARG_IN_PREFIX)	FCM_ARG_IN_TYPE FCM_PREFIX_LINE(FCM_ARG_IN_PREFIX)
#define FCM_THIS_POINT_KEY(FCM_ARG_1,FCM_ARG_TAG)					static FCM_TYPE_PREFIX_LINE( function_call_metric::Identifier, FCM_ARG_1)(__FUNCTION__ , __BASE_FILE__ , FCM_TOEKN_AS_STRING(__LINE__),FCM_ARG_TAG );
#define FCM_PROFILE_THIS_POINT_TO_BLOCK_END(FCM_ARG_TAG)          FCM_THIS_POINT_KEY(fcm_this_place_idfr_,FCM_ARG_TAG); FCM_TYPE_PREFIX_LINE(function_call_metric::Sentinel, fcm_this_place_snti_)( FCM_PREFIX_LINE(fcm_this_place_idfr_) );
#define FCM_PROFILE_THIS_POINT_TO_BLOCK_END_LG(FCM_ARG_TAG, FCM_ARG_DOLOG)          FCM_THIS_POINT_KEY(fcm_this_place_idfr_,FCM_ARG_TAG); FCM_TYPE_PREFIX_LINE(function_call_metric::Sentinel, fcm_this_place_snti_)( FCM_PREFIX_LINE(fcm_this_place_idfr_), FCM_ARG_DOLOG );

namespace function_call_metric
{
	class Identifier
	{
		public:
			Identifier(const char* pszFunction, const char* pszFileName, const char* LineNumberAsString, const char* tag);
			inline const char* get_key() const { return m_unique_key.c_str(); }

			friend std::ostream& operator<<( std::ostream& ostr, const Identifier& obj ) ;
		private:
			std::string m_file;
			std::string m_function;
			std::string m_line;
			std::string m_tag;
			std::string m_unique_key;
	};

	std::ostream& operator<<( std::ostream& ostr, const Identifier& obj ) ;

	class Sentinel_imp;
	class Sentinel
	{
		public:
			Sentinel(const Identifier& obj);
			Sentinel(const Identifier& obj, bool doLog);
			~Sentinel();
		private:
			Sentinel_imp* m_pimpl;
	};
	void keep_reporting(const char* pszOutFilePath, size_t dump_freq_in_seconds, size_t iterations);

	typedef void (*fcn_fn_logCallBack)(const char* pszString);
	void register_logCallBack(fcn_fn_logCallBack inCallback);
}

#else


#define FCM_PROFILE_THIS_POINT_TO_BLOCK_END(FCM_ARG_TAG)          ;
#define FCM_PROFILE_THIS_POINT_TO_BLOCK_END_LG(FCM_ARG_TAG, FCM_ARG_DOLOG)          ;

namespace function_call_metric
{

	inline void keep_reporting(const char* pszOutFilePath, int dump_freq_in_seconds, 
			int iterations){}

	typedef void (*fcn_fn_logCallBack)(const char* pszString);
	inline void register_logCallBack(fcn_fn_logCallBack inCallback){}
}


#endif //FUNCTION_CALL_METRIC_ENABLED

#endif //__FUNCTION_CALL_METRIC_CLIENT_H__

