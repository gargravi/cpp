
#include <function_call_metric_client.h>

#ifdef FUNCTION_CALL_METRIC_ENABLED

#include <iostream>
#include <sstream> 
#include <cmath>
#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time.hpp> 

using namespace boost::timer;

namespace function_call_metric
{

	typedef std::pair< const char* , cpu_times >    SingleInvocation;

	class Counters
	{
		public:
			Counters() 
			{
				m_hitCount = 0;
				m_total.clear();
				m_min.clear();
				m_max.clear();
			}

			void add(const cpu_times& src) 
			{
				++m_hitCount;

				m_total.wall	+= src.wall;
				m_total.user	+= src.user;
				m_total.system  += src.system;

				if( (0 == m_min.wall) || (m_min.wall > src.wall) ) {
					m_min.wall    = src.wall;
					m_min.user    = src.user;
					m_min.system  = src.system;
				}

				if(m_max.wall < src.wall) {
					m_max.wall    = src.wall;
					m_max.user    = src.user;
					m_max.system  = src.system;
				}
			}

			cpu_times get_averate() const
			{
				cpu_times  avg;
				avg.clear();
				if( m_hitCount > 0 )
				{
					avg.wall    = ( m_total.wall   / m_hitCount );
					avg.user    = ( m_total.user   / m_hitCount );
					avg.system  = ( m_total.system / m_hitCount );
				}
				return avg;
			}


			friend std::ostream& operator<<( std::ostream& ostr, const Counters& obj ) ;

		public:
			int64_t     m_hitCount;
			cpu_times   m_total;
			cpu_times   m_min;
			cpu_times   m_max;
	};
	
	std::ostream& operator<<( std::ostream& ostr, const Counters& obj ) 
	{ 
		static const std::string frmt("%w,%u,%s,%t,%p");
		static const int places = 8;
		cpu_times avg =  obj.get_averate();
		ostr << obj.m_hitCount 
			 << "," << format(obj.m_total,places,frmt) 
			 << "," << format(avg,places,frmt) 
			 << "," << format(obj.m_min,places,frmt) 
			 << "," << format(obj.m_max,places,frmt) ;
		return ostr;
	}
	std::ostream& operator<<( std::ostream& ostr, const SingleInvocation& obj ) 
	{ 
		static const std::string frmt("%w,%t");
		static const int places = 10;
		ostr << "HIT_LOG," << obj.first << "," << format(obj.second,places,frmt) ;
		return ostr;
	}

	//std::ostream& operator<<( std::ostream& ostr, const Counters& obj ) 
	//{ 
	//	static const std::string frmt("%ws %us %ss %ts (%p%)");
	//	static const int places = 5;
	//	cpu_times avg =  obj.get_averate();
	//	ostr << obj.m_hitCount 
	//		 << "," << format(obj.m_total,places,frmt) 
	//		 << "," << format(avg,places,frmt) 
	//		 << "," << format(obj.m_min,places,frmt) 
	//		 << "," << format(obj.m_max,places,frmt) ;
	//	return ostr;
	//}

	class IfaceInvoactionStream
	{
		public:
			virtual ~IfaceInvoactionStream(){}
			virtual void pluck_all( std::vector<SingleInvocation*>& dest) = 0;
	};

	class All_Stream_Counters
	{
		public:

			void register_logCallBack(fcn_fn_logCallBack inCallback)
			{
				m_logCallBack = inCallback;
			}

			void register_stream( IfaceInvoactionStream* stream)
			{
				if(stream) {
					boost::mutex::scoped_lock lock(m_mtx);
					m_streams.push_back(stream);
				}
			}

			void unregister_stream( IfaceInvoactionStream* stream)
			{
				if(stream) {
					boost::mutex::scoped_lock lock(m_mtx);
					m_streams.remove(stream);
					reduce(stream);
				}
			}

			static All_Stream_Counters& get_instance()
			{
				static All_Stream_Counters s_inst;
				return s_inst;
			}

			void getSnapshot(std::map<std::string, Counters>& dest)
			{
				reduce_all();
				boost::mutex::scoped_lock lock(m_mtx);
				if( 0 < m_countersForIdentifers.size() )
				{
					dest = m_countersForIdentifers;
					m_countersForIdentifers.clear(); // prepare for next snapshot
				}
			}

			void getCumulative(std::map<std::string, Counters>& dest)
			{
				reduce_all();
				boost::mutex::scoped_lock lock(m_mtx);
				if( 0 < m_countersForIdentifers.size() )
				{
					dest = m_countersForIdentifers;
				}
			}

			void logInvocation(SingleInvocation* snglInvoc) const
			{
				if( ( NULL != m_logCallBack ) && (NULL != snglInvoc) )
				{
					std::stringstream ss;
					ss << *snglInvoc;
					m_logCallBack( ss.str().c_str() );
				}
			}

		private:
			void reduce(IfaceInvoactionStream* stream) // should be called with lock
			{
				if(NULL != stream)
				{
					std::vector<SingleInvocation*> invocations;
					stream->pluck_all(invocations);
					for( std::vector<SingleInvocation*>::iterator itr = invocations.begin(); itr != invocations.end(); ++itr)
					{
						SingleInvocation* snglInvoc = *itr;
						if(NULL != snglInvoc)
						{
							Counters&  cntrs = m_countersForIdentifers[ std::string(snglInvoc->first) ];
							cntrs.add( snglInvoc->second );
							delete snglInvoc;
						}
					}

				}
			}

			void reduce_all()
			{
				boost::mutex::scoped_lock lock(m_mtx);
				for( std::list<IfaceInvoactionStream*>::iterator itr = m_streams.begin(); itr != m_streams.end(); ++itr)
				{
					reduce(*itr);
				}
			}

			All_Stream_Counters(): m_logCallBack(NULL) {}
			~All_Stream_Counters(){}

		private:
			fcn_fn_logCallBack					m_logCallBack;
			boost::mutex						m_mtx;
			std::list<IfaceInvoactionStream*>	m_streams;
			std::map<std::string, Counters>		m_countersForIdentifers;
	};

	class Invocation_Stream : public IfaceInvoactionStream
	{
		private:
			typedef boost::lockfree::queue<SingleInvocation*, boost::lockfree::fixed_sized<false>  >   InvStrmQueue;
		public:

			//void add(const char* pszIdentifier, const cpu_times& cpuTime) 
			//{
			//	if(NULL != pszIdentifier) {
			//		m_invokations->push( new SingleInvocation(pszIdentifier, cpuTime) );
			//	}
			//}

			cpu_timer* getTimer()
			{
				if( m_onDemandTimerList.empty() )
					return (new cpu_timer);

				cpu_timer* tmr = m_onDemandTimerList.front();
				m_onDemandTimerList.pop_front();
				tmr->start();
				return tmr;
			}

			void complete(const char* pszIdentifier, cpu_timer* ptimer, bool doLog)
			{
				if( (NULL != pszIdentifier) && ( NULL != ptimer) ) 
				{
					SingleInvocation* pSnglInvocation = new SingleInvocation(pszIdentifier, ptimer->elapsed()) ;
					m_invokations->push(pSnglInvocation); 
					ptimer->stop();
					m_onDemandTimerList.push_back(ptimer);
					if(doLog) 
					{
						All_Stream_Counters::get_instance().logInvocation(pSnglInvocation);
					}
				}
			}

			void pluck_all( std::vector<SingleInvocation*>& dest) 
			{
				SingleInvocation* pObj = NULL;
				while (m_invokations->pop(pObj)) {
					dest.push_back(pObj);
				}
			}

			Invocation_Stream()
			{
				m_invokations = new InvStrmQueue(0);
				m_invokations->reserve( 2*100*1024 ); //Aritary 200 K
				All_Stream_Counters::get_instance().register_stream(this);
			}

			~Invocation_Stream()
			{
				All_Stream_Counters::get_instance().unregister_stream(this);
				//just in case
				SingleInvocation* pObj = NULL;
				while (m_invokations->pop(pObj)) {
					if(NULL != pObj) {
						delete pObj;
					}
				}
			}

		private:
			//boost::lockfree::queue<SingleInvocation* >   m_invokations;
			//boost::lockfree::queue<SingleInvocation* , boost::lockfree::capacity<60*1024> >   m_invokations;
			InvStrmQueue*			m_invokations;
			std::list<cpu_timer*>   m_onDemandTimerList;
	};


	class Sentinel_imp 
	{
		public:
			Sentinel_imp(const char* pszIdentifier, bool doLog) : 
				m_pszIdentifier(pszIdentifier), m_invStrm(NULL), m_doLog(doLog)
			{
				m_ptimer = getInvocationStream()->getTimer();
			}

			~Sentinel_imp()
			{
				//m_invStrm->add( m_pszIdentifier, m_invStrm->elapsed(), m_doLog );
				m_invStrm->complete( m_pszIdentifier, m_ptimer, m_doLog );
			}

		private:
			Invocation_Stream* getInvocationStream()
			{
				if(NULL == m_invStrm)
				{
					static boost::thread_specific_ptr<Invocation_Stream> tls_inv_stream;
					Invocation_Stream* p_inv_stream = tls_inv_stream.get();
					if(NULL == p_inv_stream)
					{
						p_inv_stream = new Invocation_Stream();
						tls_inv_stream.reset(p_inv_stream);
					}
					m_invStrm = p_inv_stream;
				}
				return m_invStrm;
			}

		private:
			const char*			m_pszIdentifier;
			Invocation_Stream*	m_invStrm;
			cpu_timer*			m_ptimer;
			bool                m_doLog;
	};
	
	class Reporter
	{
		public:
			Reporter(const char* pszOutFilePath, size_t dump_freq_in_seconds, size_t iterations) :
				m_strOutFilePath(pszOutFilePath), 
				m_dump_freq_in_seconds(dump_freq_in_seconds),
				m_iterations(iterations)
		{}

		void operator()()
		{
			size_t report_ctr = 0;
			boost::posix_time::ptime report_start_time= boost::posix_time::second_clock::local_time();
			//std::string headers = "file,function,line,Hits,Total_time(unit=s|format=wall user system total(u+s) cpu%),Average_time(s),Minimum_time(s),Maximum_time(s),";
			std::string headers = "File,Line,Function,Tag,Hits,Total_wall(s),Total_user(s),Total_sys(s),Total_Util(s),Total_CPU(%),Avg_wall(s),Avg_user(s),Avg_sys(s),Avg_Util(s),Avg_CPU(%),Min_wall(s),Min_user(s),Min_sys(s),Min_Util(s),Min_CPU(%),Max_wall(s),Max_user(s),Max_sys(s),Max_Util(s),Max_CPU(%)";
			while(1)
			{
				boost::posix_time::ptime start_time= boost::posix_time::second_clock::local_time();
				boost::this_thread::sleep_for(boost::chrono::seconds(m_dump_freq_in_seconds));
				boost::posix_time::ptime end_time = boost::posix_time::second_clock::local_time();

				std::fstream reportfs;
				reportfs.open(m_strOutFilePath.c_str(),std::fstream::out | std::fstream::app);
				if (reportfs.is_open())
				{
					reportfs << headers << ",ReportCounter=" << ++report_ctr <<"|ReportStart="<<report_start_time <<"|ThisSnapStart=" << start_time << "|ThisSnapStop=" << end_time << std::endl;
					std::map<std::string, Counters > cntrs;
					All_Stream_Counters::get_instance().getCumulative(cntrs);
					if( 0 < cntrs.size() ) 
					{
						for( std::map<std::string, Counters>::iterator itr = cntrs.begin(); itr != cntrs.end(); ++itr)
						{
							reportfs << itr->first << "," << itr->second << std::endl;
						}
					}
					reportfs << std::endl << std::endl;
					reportfs.close();
				}
				if( report_ctr >= m_iterations)
					break;
			}//while
		}

		private:
			std::string		m_strOutFilePath;
			size_t			m_dump_freq_in_seconds;
			size_t			m_iterations;
	};

	void keep_reporting(const char* pszOutFilePath, size_t dump_freq_in_seconds, size_t iterations)
	{
		Reporter reportObj(pszOutFilePath, dump_freq_in_seconds, iterations);
		new boost::thread(reportObj); //TODO: how to kill
	}
	
	std::ostream& operator<<( std::ostream& ostr, const Identifier& obj ) 
	{ 
		ostr << obj.m_unique_key;
		return ostr; 
	}


	Identifier::Identifier(const char* pszFunction, const char* pszFileName, const char* LineNumberAsString,const char* pszTag ) : 
		m_file(pszFileName), m_function(pszFunction), m_line(LineNumberAsString), m_tag(pszTag)
	{
		std::stringstream ss;
		ss << m_file << "," << m_line << "," << m_function << "," << m_tag;
		m_unique_key = ss.str() ;
	}
	
	Sentinel::Sentinel(const Identifier& obj)
	{
		m_pimpl = new Sentinel_imp( obj.get_key(), false );
	}

	Sentinel::Sentinel(const Identifier& obj, bool doLog)
	{
		m_pimpl = new Sentinel_imp( obj.get_key(), doLog);
	}

	Sentinel::~Sentinel() {  delete m_pimpl; m_pimpl = NULL; }

	void register_logCallBack(fcn_fn_logCallBack inCallback)
	{
		 All_Stream_Counters::get_instance().register_logCallBack(inCallback);
	}

}

#else



//namespace function_call_metric
//{
//
//	void keep_reporting(const char* pszOutFilePath, size_t dump_freq_in_seconds, size_t iterations) {}
//
//	typedef void (*fcn_fn_logCallBack)(const char* pszString);
//	void register_logCallBack(fcn_fn_logCallBack inCallback);
//}



#endif //FUNCTION_CALL_METRIC_ENABLED

