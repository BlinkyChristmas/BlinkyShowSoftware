//Copyright © 2023 Charles Kerr. All rights reserved.

#include "timeutil.hpp"

#include <algorithm>
#include <stdexcept>

#include "strutil.hpp"

using namespace std::string_literals ;

//======================================================================
namespace util {
    //======================================================================
    //======================================================================
    HourMinute::HourMinute():hour(0),minute(0){
    }

    //======================================================================
    HourMinute::HourMinute(int hour, int minute):hour(hour),minute(minute){
    }

    //======================================================================
    HourMinute::HourMinute(const std::chrono::system_clock::time_point &timePoint):HourMinute() {
        this->load(timePoint) ;
    }

    //======================================================================
    HourMinute::HourMinute(const std::string &timeString):HourMinute() {
        this->load(timeString) ;
    }

    //======================================================================
    auto HourMinute::describe() const -> std::string {
        return util::format("%0.2i:%0.2i", hour,minute) ;
    }

    //======================================================================
    auto HourMinute::load(const std::string &timeString) ->void {
        auto [shour,sminute] = util::split(timeString,":") ;
        try {
            this->hour = std::stoi(shour,nullptr,10) ;
            this->minute = std::stoi(sminute,nullptr,10) ;
        }
        catch(...){
            throw std::runtime_error("Error converting time string: "s + timeString);
        }
        if (this->hour < 0 || this->hour > 23 || this->minute < 0 || this->minute > 59) {
            throw std::runtime_error("Error converting time string: "s + timeString);
        }
    }

    //======================================================================
    auto HourMinute::load(const std::chrono::system_clock::time_point &timePoint) -> void {
        std::stringstream output ;
        tm myvalue;
        auto format = "%H:%M"s;
        auto time = std::chrono::system_clock::to_time_t(timePoint);
    #if defined(_MSC_VER)
        auto status = ::localtime_s(&myvalue, &time);
    #else
        ::localtime_r(&time, &myvalue);
    #endif
        output << std::put_time(&myvalue, format.c_str());
        load(output.str()) ;
    }

    //====================================================================================
    auto HourMinute::operator==(const HourMinute &value) const ->bool {
        return this->hour == value.hour && this->minute == value.minute ;
    }

    //====================================================================================
    auto HourMinute::operator<(const HourMinute &value) const ->bool {
        if ( this->hour <= value.hour) {
            return this->minute < value.minute;
        }
        return false ;
    }

    //====================================================================================
    auto HourMinute::operator>(const HourMinute &value) const -> bool {
        if ( this->hour >= value.hour) {
            return this->minute > value.minute;
        }
        return false ;
    }

    //====================================================================================
    auto HourMinute::operator<=(const HourMinute &value) const ->bool {
        return operator==(value) || operator<(value) ;
    }

    //====================================================================================
    auto HourMinute::operator>=(const HourMinute &value) const ->bool {
        return operator==(value) || operator>(value) ;
    }

    // So we can add/subtract minutes from the time
    //====================================================================================
    auto HourMinute::operator+(int minutes) const -> HourMinute {
        auto newMinute = this->minute + minutes ;
        auto hoursToAdd = newMinute / 60 ; // 60 minutes to an hour
        newMinute = newMinute % 60 ;
        auto newHour = this->hour + hoursToAdd ;
        // We need to check for overflow
        if (newHour >= 24) {
            newHour -= 24 ;
        }
        return HourMinute(newHour,newMinute) ;
    }

    //====================================================================================
    auto HourMinute::operator-(int minutes) const -> HourMinute {
        auto newHour = this->hour ;
        auto newMinute = this->minute ;
        // Are we going to need to borrow an hour?
        if (newMinute < minutes) {
            newHour -= 1;
        }
        newMinute -= minutes ;
        
        if (newHour < 0) {
            // We need to do something
            newHour = 23 ;
        }
        if (newMinute < 0)  {
            newMinute = 60 + newMinute ;
        }
        return HourMinute(newHour,newMinute);
    }

    //====================================================================================
    auto HourMinute::operator-=(int minutes) ->HourMinute& {
        auto temp = this->operator-(minutes) ;
        this->hour = temp.hour ;
        this->minute = temp.minute ;
        return *this ;
    }

    //====================================================================================
    auto HourMinute::operator+=(int minutes) ->HourMinute& {
        auto temp = this->operator+(minutes) ;
        this->hour = temp.hour ;
        this->minute = temp.minute ;
        return *this ;
    }

    //====================================================================================
    auto HourMinute::operator=(const std::string& value) -> HourMinute& {
        this->load(value) ;
        return *this ;
    }
    //====================================================================================
    auto HourMinute::operator-(const HourMinute &value) const -> int {
        auto hours = this->hour - value.hour ;
        
        auto minutes = this->minute - value.minute ;
        if (minutes < 0) {
            hours -= 1 ;
            minutes += 60 ;
        }
        return (hours * 60 ) + minutes ;
    }
    
    //====================================================================================
    auto HourMinute::now() -> HourMinute {
        return HourMinute(std::chrono::system_clock::now()) ;
    }

}
