#include<iostream>
#include<thread>
using namespace std;
using namespace std::chrono_literals;

bool finish=false;

void Plus()
{
	while (!finish)
	{
		cout << "+ ";
		std::this_thread::sleep_for(.8s);
	}
}
void Minus()
{
	while (!finish)
	{
		cout << "- ";
		std::this_thread::sleep_for(.7s);
	}
}


void main()
{
	setlocale(LC_ALL, "");
	//Plus();
	//Minus();

	std::thread plus_thread(Plus);
	std::thread minus_thread(Minus);

	cin.get();
	finish = true;

	plus_thread.join();
	minus_thread.join();
}