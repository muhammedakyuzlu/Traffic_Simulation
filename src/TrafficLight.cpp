#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <future>
#include <thread>

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a:DONE : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
	
	std::unique_lock<std::mutex> _lock(_mutex);
	_condition.wait(_lock, [this] { return !_queue.empty(); });
	T msg = std::move(_queue.back());
	_queue.clear();
	return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a:DONE : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> _lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}




/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b:DONE : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
	while (true){
        TrafficLightPhase _phase = message_queue.receive();
		if (_phase == TrafficLightPhase::green) return;		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b:DONE : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a:DONE  Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device _rand;
	std::mt19937 mt(_rand());
	std::uniform_real_distribution<double> _distribution(4.0, 6.0);
    double cycle = _distribution(mt);

    auto start_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> end_time;
	while (true)
	{
        end_time = std::chrono::high_resolution_clock::now() - start_time;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (end_time.count() >= cycle)
		{
            start_time = std::chrono::high_resolution_clock::now();
			cycle = _distribution(mt);
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? green :red;
			TrafficLightPhase msg = _currentPhase;
            message_queue.send(std::move(msg));
		}
	}
}