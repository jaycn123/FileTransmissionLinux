g++ -c ./common/ThreadPool.cpp
g++ -o server_exe -O3 server/server.cpp common/MyMd5.h common/netPack.h ThreadPool.o -lpthread -l crypto
g++ -o client_exe -O3 ./client/client.cpp ./common/MyMd5.h -l crypto -lpthread
rm ThreadPool.o
mv *exe bin
