

#include "SimpleThreadPool.h"

#include <iostream>
#include <vector>



int th(int a,int b)
{
	std::cout << a << " 1 get_id " << std::this_thread::get_id() << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << a << " 2 get_id " << std::this_thread::get_id() << std::endl;
	return a + b;
}



int main()
{
	SimpleThreadPool<int, decltype(th), int, int> stp;
	std::vector<std::future<int>> vf;
	for (int i = 0; i != 10; ++i)
	{
		std::cout << "async " << i << std::endl;
		vf.push_back(stp.async(th, i, 5));
	}
	for (auto &a : vf)
	{
		std::cout << a.get() << std::endl;
	}
	system("pause");
	return 0;
}





