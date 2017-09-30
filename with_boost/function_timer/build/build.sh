#!/bin/bash
cd ../src/

cd fcm
rm -f *.o
rm -f *.so
g++ -m64 -O2 -c -g -Wall -Werror -fpic -I. function_call_metric.cpp
g++ -m64 -O2 -shared -o libfunction_call_metric.so function_call_metric.o

cd ..
rm -f clnt
g++ -m64 -fPIC -O2 -Wall -pthread -g -o clnt test_client.cpp -I./src/ -L./src -lfunction_call_metric -lboost_thread -lboost_timer

chmod +x clnt


