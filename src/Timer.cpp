#include <iostream>
#include "Timer.h"

std::map<std::string, std::chrono::steady_clock::time_point> Timer::_startTimes;

using namespace std;
using namespace std::chrono;

void Timer::StartTimer(const std::string& timerName) {
    if (_startTimes.find(timerName) != _startTimes.end()) {  // The timer was already started
        std::string message = "Cannot start a timer again before it is ended! This was attempted with a timer of name \"" + timerName + ".";
        throw std::exception(message.c_str());
    }

    _startTimes[timerName] = high_resolution_clock::now();
}

void Timer::EndTimingAndPrintResult(const std::string& timerName) {
    auto runtime = EndTimer(timerName);

    std::cout << "->TIMER: \"" << timerName << "\" took " << runtime << "ms to run." << std::endl;
}

double Timer::EndTimer(const std::string& timerName) {
    auto endTime = high_resolution_clock::now();

    duration<double, std::milli> duration = endTime - _startTimes[timerName];
    auto runtime = duration.count();

    _startTimes.erase(timerName);

    return runtime;
}
