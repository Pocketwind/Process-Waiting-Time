#include<iostream>
#include<stdio.h>
#include<string>
#include<vector>
#pragma warning(disable:4996)

#define DEL "qazwsxedcrfvtgbyhnujmikolpQAZWSXEDCRFVTGBYHNUJMIKOLP:/ \t\n"

using namespace std;

enum State { CPU, IO };
enum Prev { Running, New, Ready, Waiting};
class Process
{
private:
	bool is_end;
	int num, arrival, cpu_index, io_index, waiting, cpu_current, io_current;
	State state;
	vector<int> cpu, io;
	void ChangeState();
public:
	void print()
	{
		printf("P%02d\tARRIVAL:%d\t", getNum(), getArrival());
		for (int k = 0; k < getIOsize(); ++k)
		{
			cout << "CPU:" << getCPU(k) << "\t";
			cout << "IO:" << getIO(k) << "\t";
		}
		cout << "CPU:" << getCPU(getCPUsize() - 1) << ' ' << is_end << endl;
	}
	Process(int, int, vector<int>, vector<int>);
	void TimeForward();
	int getArrival() { return arrival; }
	State getState() { return state; }
	int getNum() { return num; }
	int getCPU(int i) { return cpu[i]; }
	int getIO(int i){return io[i]; }
	int getCPUsize() { return cpu.size(); }
	int getIOsize() { return io.size(); }
	int getWaiting() { return waiting; }
	void Waiting() { waiting++; }
	int getCPUCurrent() { return cpu_current; }
	int getIOCurrent() { return io_current; }
	bool isEnd() { return is_end; }
};
Process::Process(int Num, int Arrival, vector<int> cpu_in, vector<int> io_in)
{
	is_end = false;
	cpu_index = io_index = waiting = 0;
	state = CPU;
	num = Num;
	arrival = Arrival;
	cpu = cpu_in;
	io = io_in;
	cpu_current = cpu[0];
	if (io.size() > 0)
		io_current = io[0];
}
void Process::ChangeState()
{
	if (state == IO)
		state = CPU;
	else
		state = IO;
}


void Process::TimeForward()
{
	switch (state)
	{
	case IO:
		io[io_index]--;
		io_current = io[io_index];
		if (io[io_index] == 0)
		{
			ChangeState();
			io_index++;
		}
		break;

	case CPU:
		cpu[cpu_index]--;
		cpu_current = cpu[cpu_index];
		if (cpu_index == cpu.size() - 1&&cpu_current==0)
		{
			is_end = true;
			break;
		}
		else if (cpu[cpu_index] == 0)
		{
			cpu_index++;
			ChangeState();
		}


		break;
	}
}

void Calculate(vector<Process>& data)
{
	vector<int> queue, waiting;
	queue.reserve(20);
	waiting.reserve(20);
	bool end;
	int current = -1, shortest;
	int time = 0;
	int temp, running;


	while (true)
	{
		/*cout << time << endl;
		for (int i = 0; i < queue.size(); ++i)
		{
			data[queue[i]].print();
		}
		if (current != -1)
		{
			cout << "current : ";
			data[current].print();
		}
		cout << endl;*/

		end = true;										//한개라도 false면 탈출x
		for (int i = 0; i < data.size(); ++i)
		{
			if (data[i].isEnd() == false)
				end = false;
		}
		if (end)
			break;


		for (int i = 0; i < data.size(); ++i)
		{
			if (data[i].getArrival() == time)
			{
				queue.push_back(data[i].getNum());			//add new state
			}
		}

		for (int i = 0; i < waiting.size(); ++i)			//waiting인거 io끝나면 queue로
		{
			if (data[waiting[i]].getState() == CPU)
			{
				queue.push_back(waiting[i]);
				waiting.erase(waiting.begin() + i);
			}
		}


		if (current != -1)
		{
			for (int i = 0; i < queue.size(); ++i)			//running인거 강제 ready
			{
				if (data[current].getCPUCurrent() > data[queue[i]].getCPUCurrent())         //running이 ready보다 cpu큼
				{
					current = queue[i];
				}
			}
		}

		if (current != -1)
		{
			if (data[current].isEnd() == true)		//terminated일때 종료
			{
				for (int i = 0; i < queue.size(); ++i)
				{
					if (queue[i] == current)
					{
						queue.erase(queue.begin() + i);
						break;
					}
				}
				current = -1;
			}
		}

		if (current != -1)
		{
			if (data[current].getState() == IO)
			{															//current에 할당되있을때 io 하러 waiting
				for (int i = 0; i < queue.size(); ++i)
				{
					if (queue[i] == current)
					{
						waiting.push_back(queue[i]);
						queue.erase(queue.begin() + i);
						break;
					}
				}
				current = -1;								//current 비움
			}
		}
		
		if (current == -1)			//running 비어있을때
		{
			if (queue.size() == 0)			//ready 비어있으면 그냥 넘어감
			{ }
			else							//ready에서 running으로
			{
				shortest = data[queue[0]].getCPUCurrent();
				for (int i = 0; i < queue.size(); ++i)
				{
					if (shortest >= data[queue[i]].getCPUCurrent())
					{
						current = queue[i];
						shortest = data[queue[i]].getCPUCurrent();
					}
				}

			}
		}


		for (int i = 0; i < waiting.size(); ++i)
		{
			data[waiting[i]].TimeForward();
		}
		if (!(current == -1))
		{
			running = current;
			data[current].TimeForward();
		}
		for (int i = 0; i < queue.size(); ++i)
		{
			if (i != current && data[queue[i]].getState() == CPU)
				data[queue[i]].Waiting();
		}
		
		++time;
	}
}

int main(int argc, char** argv)
{
	string file = argv[1];
	char line[1000];
	FILE* fp;
	vector<int> buffer;
	vector<int> cpu, io;
	vector<vector<int>> data;
	char* tok;
	vector<Process> process;

	if (!(fp = fopen(file.c_str(), "r")))
	{
		cout << "ERROR: file open" << endl;
		exit(0);
	}


	while (fgets(line, 1000, fp))
	{
		buffer.clear();
		tok = strtok(line, DEL);
		do
		{
			buffer.push_back(atoi(tok));
		} while (tok = strtok(NULL, DEL));
		data.push_back(buffer);
	}
	fclose(fp);

	for (int i = 0; i < data.size(); ++i)
	{
		vector<int> cpu, io;
		for (int k = 2; k < data[i].size(); k += 2)
			cpu.push_back(data[i][k]);
		for (int k = 3; k < data[i].size(); k += 2)
			io.push_back(data[i][k]);
		Process* temp = new Process(data[i][0], data[i][1], cpu, io);
		process.push_back(*temp);
		delete temp;
	}
	
	for (int i = 0; i < process.size(); ++i)
	{
		printf("P%02d\tARRIVAL:%d\t", process[i].getNum(), process[i].getArrival());
		for (int k = 0; k < process[i].getIOsize(); ++k)
		{
			cout << "CPU:" << process[i].getCPU(k) << "\t";
			cout << "IO:" << process[i].getIO(k) << "\t";
		}
		cout << "CPU:" << process[i].getCPU(process[i].getCPUsize() - 1) << endl;
	}

	Calculate(process);

	int sum = 0;
	cout << endl << endl << "Process Waiting Time" << endl;
	for (int i = 0; i < process.size(); ++i)
	{
		printf("P%02d:%d\n", process[i].getNum(), process[i].getWaiting());
		sum += process[i].getWaiting();
	}
	cout << endl;
	cout << "Average Waiting Time = " << (double)sum / process.size();
	return 0;
}