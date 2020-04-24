#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lck(_msgmtx);
    _condition.wait(lck, [this] { return !_queue.empty();});
    auto msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lck(_msgmtx);
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
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TrafficLightPhase current_status = _message.receive();
        if(current_status == TrafficLightPhase::green)
            return;
    }
}
TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases,this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(4,6);
    auto time_span = distribution(generator);

    auto start = std::chrono::system_clock::now();
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto measured_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count();
        if(measured_time > time_span) 
        {
            if(_currentPhase == TrafficLightPhase::red)
                _currentPhase = TrafficLightPhase::green;
            else
                _currentPhase = TrafficLightPhase::red;
            
            _message.send(std::move(_currentPhase));
            start = std::chrono::system_clock::now();
            time_span = distribution(generator);
        }
    }
}