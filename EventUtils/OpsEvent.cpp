#include "OpsEvent.hpp"

#include "opencv2/opencv.hpp"

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

events::OpsEvent::OpsEvent(std::string title, std::string leader, events::Weekdays weekday, std::string time, std::string description) : 
    title(title),
    renderedTitle(""),
    description(description),
    renderedDescription(""),
    leader(leader),
    renderedLeader(""),
    time(time),
    renderedTime(""),
    weekday(weekday),
    font(cv::FontFace("Times New Roman")),
    fontSize(60),
    fontColour{ 0.54901f, 1.0f, 0.98431f }      //  RGBA
    { }

bool events::OpsEvent::setGlobalFontSize(cv::FontFace font, int &globalFontSize, int maxWidth, bool unifyFontSize){
    bool fontSizeWasChanged = false;
    if(unifyFontSize){
        if(!isUnique)
            fontSize = globalFontSize;
        float fontScaleFactor = 1.0f;

        int largestWidth = 0;
        int titleTextWidth = 0;
        wrapString(title, font, fontSize, maxWidth, &titleTextWidth);
        int timeTextWidth = 0;
        wrapString(time, font, fontSize, maxWidth, &timeTextWidth);
        int leaderTextWidth = 0;
        wrapString(leader, font, fontSize, maxWidth, &leaderTextWidth);
        int descriptionTextWidth = 0;
        wrapString(description, font, fontSize, maxWidth, &descriptionTextWidth);

        largestWidth = (titleTextWidth > largestWidth) ? titleTextWidth : largestWidth;
        largestWidth = (timeTextWidth > largestWidth) ? timeTextWidth : largestWidth;
        largestWidth = (leaderTextWidth > largestWidth) ? leaderTextWidth : largestWidth;
        largestWidth = (descriptionTextWidth > largestWidth) ? descriptionTextWidth : largestWidth;

        if(largestWidth > maxWidth)
            fontScaleFactor = (float)maxWidth / largestWidth;
        
        fontSize *= fontScaleFactor;
        if(!isUnique){
            if(fontSize != globalFontSize)
                fontSizeWasChanged = true;
            globalFontSize = fontSize;
        }
#ifdef DEBUG
        std::cout << "font size set to: " << fontSize << std::endl;
#endif
    }

    return fontSizeWasChanged;
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