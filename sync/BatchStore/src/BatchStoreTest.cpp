#include <iostream>
#include <thread>
#include "BatchStore.h"

std::mutex g_coutLock;

void thread_func(BatchStore<int>& bs, int id)
{
	while (1) {
		int itm = bs.get();
		//{
		//	std::lock_guard<std::mutex> lck(g_coutLock);
		//	std::cout << std::flush << "Thread ID: " << std::this_thread::get_id << " ,id: " << id << ", Item val :" << itm << std::endl << std::flush;
		//}
		//std::this_thread::sleep_for(std::chrono::milliseconds(90));
	}
}

int main(int argc, char* argv[])
{
	auto& producer = [](BatchStore<int>::NotifyElementCreate& callback) {
		static int iCtr = 0;
		int iStart = iCtr;
		for (int itr = 0; itr < 100; ++itr) {
			callback(++iCtr);
		}
		std::cout << "Thread ID: " << std::this_thread::get_id 
		         << "Created A batch [" << iStart << ":" << iCtr << "] " << std::endl;
	};
	BatchStore<int> bs(producer);


	std::thread  thArr[10];
	for (int iCntr = 0; iCntr < 10; ++iCntr) {
		thArr[iCntr] = std::thread(thread_func, std::ref(bs), iCntr);
	}

	for (int iCntr = 0; iCntr < 10; ++iCntr) {
		thArr[iCntr].join();
	}

	return 0;
}
