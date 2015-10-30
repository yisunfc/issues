#include <iostream>
#include <cstdlib>
#include <ctime>

//Q:given an ordered array of int, shuffle the array in O(n) time and least space.
//This solution is actually another type of "Fisher Yates shuffle" in reverse order.
using namespace std;

void random_shuffle(int *arr, int len)
{
	int temp,idx;
	for (int i = 0; i < len-1; ++i)
	{
		idx = rand() % (len-1-i) + (i+1);
		swap(arr[i], arr[idx]);
	}
}

int main()
{
	srand (time(NULL));
	int array[] = {1,2,3,4,5,6,7,8,9,10};
	int size = sizeof(array) / sizeof(int);
	random_shuffle(array, size);
	for (int i = 0; i < size; ++i)
	{
		cout << array[i] << " ";
	}
	cout  << endl;
	return 0;
}
