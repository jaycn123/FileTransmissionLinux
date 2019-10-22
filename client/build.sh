rm *.o
g++ -O3 client.cpp MyMd5.h -l crypto -lpthread
