/*
    Assignment: Synchronization
    Course:     Operating Systems
    By:         Jelmer van Vugt
    Date:       14/10/2021
*/

#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include "Buffer.h"

Buffer<int> unboundedBuffer();
Buffer<int> boundedBuffer(10);

int number = 0;

void incrementNumber(int threadId)
{
  boundedBuffer.write(number, threadId);
  number++;

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void read(int threadId)
{
  for (int i = 0; i < number; i++)
  {
    std::cout << std::to_string(boundedBuffer.read(i, threadId)) << std::endl;
  }
}

int main(int argc, char *argv[])
{

    std::thread t1(incrementNumber, 1);
    std::thread t2(read, 2);

    t1.join();
    t2.join();


  return 1;
}
