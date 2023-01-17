#include<Windows.h>
#include<iostream>
#include<conio.h>
#include<thread>
using namespace std;
using namespace std::chrono_literals;

#define tab "\t"
#define delimiter "\n--------------------------------------\n"

#define MIN_TANK_VOLUME 20
#define MAX_TANK_VOLUME 80

class Tank
{
	const unsigned int VOLUME;	//Характеристика объекта
	double fuel;				//Состояние объекта
public:
	unsigned int get_VOLUME()const{	return VOLUME;}
	double get_fuel_level()const  { return fuel;}
	double give_fuel(double amount)
	{
		fuel -= amount;
		if (fuel < 0)fuel = 0;
		return fuel;
	}
	void fill(double fuel)
	{
		if (fuel < 0)return;
		if (this->fuel + fuel < VOLUME)this->fuel += fuel;
		else this->fuel = VOLUME;
	}
	Tank(unsigned int volume)
		:VOLUME(volume<MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
			volume>MAX_TANK_VOLUME ? MAX_TANK_VOLUME : volume)
	{
		this->fuel = 0;
		cout << "Tank is ready" << endl;
	}
	~Tank()
	{
		cout << "Tank is over" << endl;
	}

	void info()const
	{
		cout << "Tank volume:" << VOLUME << " liters" << endl;
		cout << "Fuel level: " << fuel << " liters" << endl;
	}
};

#define MIN_ENGINE_CONSUMPTION	 4
#define MAX_ENGINE_CONSUMPTION	20

class Engine
{
	double consumption;
	double consumption_per_second;
	bool is_started;
public:
	double get_consumption()const
	{
		return consumption;
	}
	double get_consumption_per_second()const
	{
		return consumption_per_second;
	}
	double get_consumption_per_second(int speed)const
	{
		double current_consumption = consumption_per_second;
		if (speed > 0 && speed <= 60)current_consumption = consumption_per_second * 20 / 3;
		else if (speed > 60 && speed <= 100)current_consumption = consumption_per_second * 10 / 2;
		else if (speed > 100 && speed <= 140)current_consumption = consumption_per_second * 20 / 3;
		else if (speed > 140 && speed <= 200)current_consumption = consumption_per_second * 8.333;
		else if (speed > 200 && speed <= 250)current_consumption = consumption_per_second * 10;
		if (!is_started)current_consumption = 0;
		return current_consumption;
	}
	Engine(double consumption)
		:consumption(consumption<MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
			consumption>MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION : consumption)
	{
		consumption_per_second = consumption * 3e-5;
		is_started = false;
		cout << "Engine is ready" << endl;
	}
	~Engine()
	{
		cout << "Engine is over" << endl;
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	void info()const
	{
		cout << "Consumption/100  km: " << consumption << " liters" << endl;
		cout << "Consumption/1sec km: " << consumption_per_second << " liters" << endl;
		cout << "Engine is " << (is_started ? "started" : "stopped") << endl;
	}
};

#define MAX_SPEED_LOW_LIMIT 50
#define MAX_SPEED_HIGHT_LIMIT 300

class Car
{
	Engine engine;
	Tank tank;
	bool driver_inside;
	int speed;
	const int MAX_SPEED;
	struct Control
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;
	}threads;
public:
	Car(double consumption, int volume, int max_speed) :engine(consumption), tank(volume),
		MAX_SPEED(max_speed < MAX_SPEED_LOW_LIMIT ? MAX_SPEED_LOW_LIMIT : max_speed>MAX_SPEED_HIGHT_LIMIT ? MAX_SPEED_HIGHT_LIMIT : max_speed)
	{
		driver_inside = false;
		speed = 0;
		cout << "Your car is ready to go" << endl;
	}
	~Car()
	{
		cout << "You car is over" << endl;
	}

	void get_in()
	{
		driver_inside = true;
		threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (threads.panel_thread.joinable()) threads.panel_thread.join();
		system("CLS");
		cout << "You are out of car" << endl;
	}
	void start_engine()
	{
		if (driver_inside && tank.get_fuel_level() > 0)
		{
			engine.start();
			threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}
	void stop_engine()
	{
		if (driver_inside && threads.engine_idle_thread.joinable())
		{
			engine.stop();
			threads.engine_idle_thread.join();
		}
	}

	void control()
	{
		char key;
		do
		{
			key = _getch();
			switch (key)
			{
			case 13:
				if (driver_inside)get_out();
				else get_in();
				break;
			case 'F':
			case 'f':
				if (driver_inside)
				{
					cout << "Чтобы заправиться, выйдите из машины" << endl;
				}
				else
				{
					double fuel;
					cout << "Введите объем топлива: "; cin >> fuel;
					tank.fill(fuel);
				}
				break;
			case 'I':	//Зажигание
			case 'i':
				if (engine.started())stop_engine();
				else start_engine();
				break;
			case 'W':
			case 'w':
				if (driver_inside && engine.started())
				{
					speed += 10;
					if (speed > MAX_SPEED)speed = MAX_SPEED;
					if (!threads.free_wheeling_thread.joinable())
						threads.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
					std::this_thread::sleep_for(1s);
				}
				break;
			case 'S':
			case 's':
				if (driver_inside)
				{
					speed -= 10;
					if (speed < 0)speed = 0;
					std::this_thread::sleep_for(1s);
				}
				break;
			case 27:
				stop_engine();
				get_out();
				speed = 0;
				cout << "Good buy;-)" << endl;
				break;
			}
			if (speed == 0 && threads.free_wheeling_thread.joinable())threads.free_wheeling_thread.join();
		} while (key != 27);
	}

	void panel()const
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		while (driver_inside)
		{
			system("CLS");
			cout << "Engine is " << (engine.started() ? "started" : "stopped") << endl;
			cout << "Fuel level: ";
			if (tank.get_fuel_level() < 5)SetConsoleTextAttribute(hConsole, 0xCF);
			cout.width(8);
			cout << tank.get_fuel_level();
			SetConsoleTextAttribute(hConsole, 0x07);
			cout << " liters" << endl;
			cout << "Current consumption: " << engine.get_consumption_per_second(speed) << " liters\n";
			for (int i = 0; i < MAX_SPEED / 2.2; i++)cout << (i < speed / 2.2 ? "|" : "."); cout << endl;
			cout << "Speed:\t" << speed << " km/h\n";

			std::this_thread::sleep_for(900ms);
		}
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second(speed)) > 0)
		{
			std::this_thread::sleep_for(1s);
		}
	}
	void free_wheeling()
	{
		while (speed-- > 0)
			std::this_thread::sleep_for(1s);
	}
	void info()const
	{
		engine.info();
		tank.info();
	}
};

//#define TANK_CHECK
//#define ENGINE_CHECK

void main()
{
	setlocale(LC_ALL, "");
#ifdef TANK_CHECK
	Tank tank(3);
	tank.info();
	cout << delimiter << endl;
	tank.fill(10);
	tank.fill(3);
	tank.fill(10);
	tank.info();
#endif // TANK_CHECK

#ifdef ENGINE_CHECK
	Engine engine(10);
	//engine.start();
	engine.info();
#endif // ENGINE_CHECK

	Car bmw(10, 60, 250);
	//bmw.info();
	bmw.control();
}