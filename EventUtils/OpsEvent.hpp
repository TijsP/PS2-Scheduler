#ifndef _OPSEVENT_INCLUDED_
#define _OPSEVENT_INCLUDED_

#include <fstream>
#include <string>

namespace events{

    enum Weekdays {
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday,
        tbd
    };
    std::ostream &operator<<(std::ostream &output, const Weekdays &weekday);
    std::istream &operator>>(std::istream &input, Weekdays &weekday);

    struct OpsEvent{
        Weekdays weekday = Monday;
        std::string title;
        std::string renderedTitle;
        std::string description;
        std::string renderedDescription;
        std::string leader;
        std::string renderedLeader;
        std::string time;
        std::string renderedTime;

        OpsEvent(std::string title, std::string leader, Weekdays weekday, std::string time = "", std::string description = "");

        OpsEvent &operator= (const OpsEvent &rhs);

        const static char *returnWeekday(Weekdays weekdayOfInterest);
    };
    std::ostream &operator<<(std::ostream &output, const OpsEvent &opsevent);
    std::istream &operator>>(std::istream &input, OpsEvent &opsevent);
    bool compareByWeekday (const OpsEvent &lhs, const OpsEvent &rhs);

}

#endif