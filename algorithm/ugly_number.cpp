//Question:If all factors of a number are 2 or 3 or 5,
//then this number is called a ugly number.
//Usually we treat 1 as the first ugly number.
//Find out the 1500th ugly number in an ascent order.

#include <iostream>

int get_min_number(int a, int b, int c)
{
	int min_ab = std::min(a,b);
	return std::min(min_ab, c);
}
int find_ugly_number(int target)
{
	if (target < 1)
		return 0;
	int* array = new int[target];
	array[0] = 1;
	int *pos2 = array;
	int *pos3 = array;
	int *pos5 = array;
	int index = 1;

	while (index < target)
	{
		int cur_max = get_min_number(*pos2*2, *pos3*3,*pos5*5);
		array[index++] = cur_max;

		while (*pos2*2 <= cur_max)
			pos2++;
		while (*pos3*3 <= cur_max)
			pos3++;
		while (*pos5*5 <= cur_max)
			pos5++;
	}

	int ret = array[target-1];
	delete [] array;
	return ret;
}


int main()
{
	int index = 1500;
	int result = find_ugly_number(index);
	std::cout << result << std::endl;
	return 0;
}
