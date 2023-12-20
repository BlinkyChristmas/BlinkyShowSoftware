//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef timeutil_hpp
#define timeutil_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

//======================================================================
namespace util {
    //=========================================================
    // Time/String conversions
    //=========================================================
    using ourclock = std::chrono::system_clock ;
    //=======================================================================
    /// Converts a system clock time point to a string value
    /// - Parameters:
    ///     - t: the time point
    ///     - format: the format string (defaults to Thu Dec 30 14:13:28 2021)
    /// - Returns: string value of the time point
    inline auto sysTimeToString(const std::chrono::system_clock::time_point &t, const std::string &format = "%a %b %d %H:%M:%S %Y") -> std::string {
        std::stringstream output;
        auto time = std::chrono::system_clock::to_time_t(t);
        tm myvalue;
#if defined(_MSC_VER)
        auto status = ::localtime_s(&myvalue, &time);
#else
        ::localtime_r(&time, &myvalue);
#endif
        output << std::put_time(&myvalue, format.c_str());
        
        return output.str();
    }
    
    //=======================================================================
    /// Converts a string value of time to a system time point
    /// - Parameters:
    ///     - str: the string value for the time point
    ///     - format: the string value representing the format (Thu Dec 30 14:13:28 2021)
    /// - Returns: a system time point
    inline auto stringToSysTime(const std::string &str, const std::string &format = "%a %b %d %H:%M:%S %Y") -> std::chrono::system_clock::time_point {
        std::stringstream timbuf(str);
        tm converted;
        timbuf >> std::get_time(&converted, format.c_str());
        converted.tm_isdst = -1;
        auto ntime = mktime(&converted);
        return std::chrono::system_clock::from_time_t(ntime);
    }
    
    //=======================================================================
    /// Returns time now in a string
    /// - Parameters:
    ///     - format: the format string (defaults to Thu Dec 30 14:13:28 2021)
    /// - Returns: string value of the time point
    inline auto timeNow(const std::string &format = "%a %b %d %H:%M:%S %Y") -> std::string {
        return sysTimeToString(std::chrono::system_clock::now(), format);
    }
    
    //======================================================================
    class HourMinute {
        
        int hour ;
        int minute ;
        
    public:
         
        HourMinute() ;
        HourMinute(int hour, int minute);
        HourMinute(const std::chrono::system_clock::time_point &timePoint) ;
        HourMinute(const std::string &timeString) ;

        auto load(const std::string &timeString) -> void ;
        auto load(const std::chrono::system_clock::time_point &timePoint) ->void  ;

        auto describe() const -> std::string ;
        
        // So we can compare HourMinute values
        auto operator==(const HourMinute &value) const ->bool ;
        auto operator<(const HourMinute &value) const ->bool ;
        auto operator>(const HourMinute &value) const -> bool ;
        auto operator<=(const HourMinute &value) const ->bool ;
        auto operator>=(const HourMinute &value) const ->bool ;
        
        // So we can add/subtract minutes from the time
        auto operator+(int minutes) const -> HourMinute ;
        auto operator-(int minutes) const -> HourMinute ;
        auto operator-=(int minutes) ->HourMinute& ;
        auto operator+=(int minutes) ->HourMinute& ;
        // So we can set it with a "= 02:48" type expression
        auto operator=(const std::string& value) -> HourMinute& ;
        
        // So we can understand the amount of minutes between two times
        auto operator-(const HourMinute &value) const -> int ;
        
        // We want to get time now in an HourMinute structure
        static auto now() -> HourMinute ;

    } ;

}

#endif /* timeutil_hpp */
