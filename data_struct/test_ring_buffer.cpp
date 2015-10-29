#include <iostream>
#include <string>
#include "RingBuffer.h"

using namespace std;

int main()
{
	RingBuffer rb(64);
	string str("hello,test!");//12
	for (int i = 0; i < 3; i++)
	{
		rb.write(str.data(), str.size());
	}
	char tmp[128];
	memset(tmp, 0 ,128);
	rb.read(tmp, 20);

	cout << tmp << endl;;

	str = "continue!";//10
	for (int i = 0; i < 13; i++)
	{
		size_t ret = rb.write(str.data(), str.size());
		if (ret != str.size())
			cout << "write fail:" << i << endl;
	}
	rb.read(tmp,128);
	cout << tmp;
	return 0;
}
