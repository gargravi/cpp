#!/bin/bash
# yum install boost-devel (1.53.0)
cd ../src/

cd fcm
rm -f *.o
rm -f *.so
#g++ -m64 -O2 -c -g -Wall -Werror -fpic -I. -I"$CPP_BOOST_HOME" function_call_metric.cpp
#g++ -m64 -O2 -c -g -Wall -Werror -fpic -DFUNCTION_CALL_METRIC_ENABLED -I. -I$CPP_BOOST_HOME/ function_call_metric.cpp
g++ -m64 -O2 -c -g -Wall -fpic -DFUNCTION_CALL_METRIC_ENABLED -I. -I$CPP_BOOST_HOME/ function_call_metric.cpp
g++ -m64 -O2 -shared -o libfunction_call_metric.so function_call_metric.o

cd ..
rm -f clnt
#g++ -m64 -fPIC -O2 -Wall -pthread -g -o clnt test_client.cpp -I./src/ -L./src -lfunction_call_metric -lboost_thread -lboost_timer
g++ -m64 -fPIC -O2 -Wall -pthread -DFUNCTION_CALL_METRIC_ENABLED -g -o clnt test_client.cpp -I./fcm -I$CPP_BOOST_HOME -L./fcm -lfunction_call_metric -lboost_thread -lboost_timer

chmod +x clnt


