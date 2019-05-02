#pragma once
#include <chrono>
#include <map>
#include <string>

class Timer {
   public:
    // Starts a timer with the given name
    static void StartTimer(const std::string& timerName);

    // Ends the given timer and prints the number of milliseconds it took to run to cout
    static void EndTimingAndPrintResult(const std::string& timerName);

    // End the timer with the given name and returns the number of milliseconds it ran for
    static double EndTimer(const std::string& timerName);

   private:
    static std::map<std::string, std::chrono::steady_clock::time_point> _startTimes;
};
