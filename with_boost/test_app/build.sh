#!/bin/bash



rm -f *.o
rm -f *.so
rm -f clstk
#g++ -m64 -O2 -c -g -Wall -Werror -fpic -I. function_call_metric.cpp
#g++ -m64 -O2 -shared -o libfunction_call_metric.so function_call_metric.o

#g++ -m64 -O2 -Wall -pthread -g -o clnt test_client.cpp -I./src/ -L./src -lfunction_call_metric -lboost_thread -lboost_timer
#g++ -m64 -Wall -pthread -g -o clnt test_bed.cpp -I/home/rg/tp/boost/boost_1_40_0
#g++ -m64 -fPIC -O2 -Wall -pthread -g -o clnt test_bed.cpp -I/home/rg/tp/boost/boost_1_40_0
#g++ -std=c++11 -m64 -fPIC -O2 -Wall -pthread -g -o clnt test_bed.cpp -I/home/rg/tp/boost/boost_1_40_0
#g++ -std=c++11 -m64 -fPIC -O2 -Wall -pthread -g -o clstk test_callstack.cpp -I/home/rg/tp/boost/boost_1_40_0
#g++ -std=c++11 -m64 -fPIC -Wall -pthread -g -o clstk test_callstack.cpp -I/home/rg/tp/boost/boost_1_65_1 -L/home/rg/tp/boost/boost_1_65_1_inst/lib -lboost_stacktrace_addr2line -lboost_stacktrace_basic -ldl
g++ -std=c++11 -m64 -fPIC -Wall -pthread -g -o clstk test_callstack.cpp -I$CPP_BOOST_HOME -lboost_stacktrace_addr2line -lboost_stacktrace_basic -ldl
chmod +x clstk

#./clstk 4


