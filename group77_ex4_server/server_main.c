

#include "../Shared/hardCodedData.h"
int main()
{
	char test[5] = { 'a', 'b','c','d', '\n' };
	int len = 0, ret_val = TRUE;
	Messege msg;
	ret_val = initMessege(&msg, "Type", "Param1", "Param2", NULL, NULL, NULL);
	printMessege(&msg);
	freeMessege(&msg);
	//len = calcCharLstLen(test);
	//printf("messege length: %d\n", len);
	//printf("%c\n", *test);
}