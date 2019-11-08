#include "../common/sendService.h"



int main()
{
	signal(SIGPIPE,SIG_IGN);  
	SendService sendService;
	sendService.Start();
	return 0;
}


