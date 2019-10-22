g++ -c ThreadPool.cpp
g++ -O3 server.cpp MyMd5.h netPack.h ThreadPool.o -lpthread -l crypto
