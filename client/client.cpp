#include "../common/recvService.h"

int main(int argc, char* argv[])
{
    std::cout<<"argc[0] : "<<argv[0]<<std::endl;
    std::string serverip = "127.0.0.1";
    if (argc > 1)
    {
        serverip = argv[1];
    }

    RecvService recvservice(serverip);
    recvservice.Start();

    return 0;  
}
