#include "fcm/function_call_metric_client.h"
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp> 

#define CPU_CYCLES(X_IN_VAL)		while(1) { std::string str; float flt=1.0; for(int i=0; i< X_IN_VAL; ++i) { str += "."; flt *= 2; } ;  std::cout << str.length()<<"," << flt << "\n";break; };
#define SLEEP_CYCLES(X_IN_VAL)		while(1) { for(int i=0; i< X_IN_VAL; ++i) { boost::this_thread::sleep(boost::posix_time::milliseconds(100)); } break; };

//#define CPU_CYCLES(X_IN_VAL)     ;
//#define SLEEP_CYCLES(X_IN_VAL)   ;

void function_inner(int factor, char* argv)
{
	std::cout << "func_inner\n";
	FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");

	CPU_CYCLES(factor);

	SLEEP_CYCLES(factor);
}

int function_outer(int factor)
{
	std::cout << "func_outter\n";
	FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
	for(int i=0; i< factor; ++i) 
	{ 
		CPU_CYCLES(2);
		function_inner(2,NULL);
		SLEEP_CYCLES(4);
		function_inner(3,NULL);
	}
	return 0;
}

void* run1(void* parg)
{
	while(1) {
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
		function_inner(20,NULL);
	}
	return NULL;
}

void* run2(void* parg)
{
	while(1) {
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
		function_outer(3);
	}
	return NULL;
}


int test_multiThread()
{
	pthread_t  th1, th2;

	(void) pthread_create(&th1, NULL, &run1, NULL);
	(void) pthread_create(&th2, NULL, &run2, NULL);

	function_call_metric::keep_reporting( "./report1.csv" , 10 , 5000 );

	(void) pthread_join(th1, NULL);
	(void) pthread_join(th2, NULL);
	return 0;

}

float cpu_var(int factor)
{
	FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
	std::vector<float> vtr;
	std::string str; 
	float flt=1.0; 
	for(int i=0; i < factor; ++i) 
	{ 
		str += "_x_y_z"; 
		flt *= 2; 
		vtr.push_back(flt);
	} 
	flt += str.length();
	flt += vtr.size();
	return flt;
}

#include <boost/timer/timer.hpp>
using namespace boost::timer;

cpu_times total_noOp_sleep;
cpu_times total_cpu_alu;

void add_times(cpu_times& dest, const cpu_times& src)
{
	dest.wall		+= src.wall;
	dest.user		+= src.user;
	dest.system		+= src.system;
}

float cpu_alu(int factor)
{
	cpu_timer	t_timer;
	FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
	float flt=1.0; 
	int it = 0;
	for(int i=0; i < factor; ++i) 
	{ 
		flt *= 2; 
		it += (int)flt;
	} 
	add_times(total_cpu_alu,t_timer.elapsed());
	return flt+it;
}


void noOp_sleep(int factor)
{
	cpu_timer	t_timer;
	FCM_PROFILE_THIS_POINT_TO_BLOCK_END("test");
	for(int i=0; i < factor; ++i) 
	{ 
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	} 
	add_times(total_noOp_sleep,t_timer.elapsed());
}

void* test_cpu_var(void* parg)
{
	while(1) 
	{
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END("warp");
		cpu_var(10*1024);
	}
	return NULL;
}

void* test_cpu_alu(void* parg)
{
	while(1) 
	{
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END("warp");
		cpu_alu(100*1024);
	}
	return NULL;
}

void* test_sleep(void* parg)
{
	while(1) 
	{
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END_LG("warp", true);
		noOp_sleep(20);
	}
	return NULL;
}

void* test_conter_coverage(void* parg)
{
	while(1) 
	{
		FCM_PROFILE_THIS_POINT_TO_BLOCK_END("warp");
		cpu_var(1024);
		noOp_sleep(10);
	}
	return NULL;
}

void printCout(const char* pszStr)
{
	if(NULL != pszStr)
	{
		std::cout << pszStr << std::endl;
	}
}

int test_counters()
{
	total_noOp_sleep.clear();
	total_cpu_alu.clear();
	pthread_t  th2, th3;
	//pthread_t  th1, th2, th3, th4;
	function_call_metric::register_logCallBack(printCout);

	//(void) pthread_create(&th1, NULL, &test_cpu_var, NULL);
	(void) pthread_create(&th2, NULL, &test_cpu_alu, NULL);
	(void) pthread_create(&th3, NULL, &test_sleep, NULL);
	//(void) pthread_create(&th4, NULL, &test_conter_coverage, NULL);

	function_call_metric::keep_reporting( "./report_imp_chk.csv" , 10 , 5000 );

	//(void) pthread_join(th1, NULL);
	//(void) pthread_join(th2, NULL);
	//(void) pthread_join(th3, NULL);
	boost::this_thread::sleep(boost::posix_time::milliseconds(60*1000));

	static const std::string frmt("%w,%u,%s,%t,%p");
	static const int places = 8;
	std::cout << std::endl 
		 << "total_noOp_sleep: " << format(total_noOp_sleep,places,frmt) << std::endl
		 << "total_cpu_alu: " << format(total_cpu_alu,places,frmt) << std::endl;

	//(void) pthread_join(th4, NULL);
	return 0;

}

void* test_thd(void* parg)
{
	
	function_call_metric::keep_reporting( "./report1.csv" , 5 , 5000 );
	for( int i=0; i <= 30; i++)
	{
		noOp_sleep(2*10);//2 sec
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(9000));

	return NULL;
}

int test_thread()
{
	pthread_t  th1;

	(void) pthread_create(&th1, NULL, &test_thd, NULL);

	function_call_metric::keep_reporting( "./report_test_thread.csv" , 10 , 5000 );

	(void) pthread_join(th1, NULL);
	return 0;
}



int main(int argv, char* argc[])
{
	//FCM_PROFILE_THIS_POINT_TO_BLOCK_END("testxxx");
	//FCM_PROFILE_THIS_POINT_TO_BLOCK_END_LG("testxxx", false);
	//FCM_PROFILE_THIS_POINT_TO_BLOCK_END_LG("testxxx", true);
	// return test_thread();
	 return test_counters();
	// return test_multiThread();
	//
	
	function_call_metric::keep_reporting( "./report1.csv" , 5 , 5000 );
	for( int i=0; i <= 30; i++)
	{
		noOp_sleep(2*10);//2 sec
	}

	boost::this_thread::sleep(boost::posix_time::milliseconds(9000));
	return 0;
}
