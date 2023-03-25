#include "OpsEvent.hpp"

std::ostream &events::operator<<(std::ostream &output, const Weekdays &weekday){
    int weekdayInt = weekday;
    output << weekdayInt;
    return output;
}
std::istream &events::operator>>(std::istream &input, Weekdays &weekday){
    int weekdayInt;
    input >> weekdayInt;
    weekday = (Weekdays)weekdayInt;
    return input;
}
events::OpsEvent::OpsEvent(std::string title, std::string leader, events::Weekdays weekday, std::string time, std::string description) : title(title), description(description), leader(leader), time(time), weekday(weekday) { }
const char *events::OpsEvent::returnWeekday(events::Weekdays weekdayOfInterest){
        switch (weekdayOfInterest)
        {
        case events::Monday:
            return "Monday";
            break;
        case events::Tuesday:
            return "Tuesday";
            break;
        case events::Wednesday:
            return "Wednesday";
            break;
        case events::Thursday:
            return "Thursday";
            break;
        case events::Friday:
            return "Friday";
            break;
        case events::Saturday:
            return "Saturday";
            break;
        case events::Sunday:
            return "Sunday";
            break;
        case events::tbd:
            return "t.b.d.";
            break;
        
        default:
            return "Invalid";
            break;
        }
    }
events::OpsEvent &events::OpsEvent::operator= (const events::OpsEvent &rhs){
        weekday = rhs.weekday;
        title = rhs.title;
        description = rhs.description;
        leader = rhs.leader;
        time = rhs.time;

        return *this;
    }
std::ostream &events::operator<<(std::ostream &output, const events::OpsEvent &opsevent){
    output << opsevent.title << "\t" << opsevent.description << "\t" << opsevent.leader << "\t" << opsevent.time << "\t" << opsevent.weekday;
    return output;
}
std::istream &events::operator>>(std::istream &input, events::OpsEvent &opsevent){
    std::getline(input, opsevent.title, '\t');
    std::getline(input, opsevent.description, '\t');
    std::getline(input, opsevent.leader, '\t');
    std::getline(input, opsevent.time, '\t');
    input >> opsevent.weekday;
    
    return input;
}
bool events::compareByWeekday (const events::OpsEvent &lhs, const events::OpsEvent &rhs){
    return lhs.weekday < rhs.weekday;
}