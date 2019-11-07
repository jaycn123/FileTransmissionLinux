g++ -c ./common/ThreadPool.cpp
g++ -o server_exe -O3 ./server/server.cpp ThreadPool.o -lpthread -l crypto
g++ -o client_exe -O3 ./client/client.cpp -l crypto -lpthread
rm ThreadPool.o
mv *exe bin
