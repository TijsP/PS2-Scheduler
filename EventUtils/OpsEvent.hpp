#ifndef _OPSEVENT_INCLUDED_
#define _OPSEVENT_INCLUDED_

#include <fstream>
#include <string>

#include "EventHelper.hpp"

#include "opencv2/imgproc.hpp"

namespace cv{
    class FontFace;
}

namespace events{

    struct OpsEvent{
        //  usually unique to each OpsEvent. Rendered versions of the variable are meant for formatting (such as newline characters)
        std::string title;
        std::string description;
        std::string leader;
        std::string time;
        events::Weekdays weekday;

        //  usually shared between OpsEvents. Can be set on a per-event basis. In this case, isUnique should be set to true
        bool isUnique = false;
        cv::FontFace font;
        int fontSize;
        float fontColour[3];
        int verticalPadding;

        OpsEvent(std::string title, std::string leader, Weekdays weekday, std::string time = "", std::string description = "");

        OpsEvent &operator= (const OpsEvent &rhs);

        bool setGlobalFontSize(cv::FontFace font, int &globalFontSize, int maxWidth, bool unifyFontSize);
    };
    std::ostream &operator<<(std::ostream &output, const OpsEvent &opsevent);
    std::istream &operator>>(std::istream &input, OpsEvent &opsevent);
    bool compareByWeekday (const OpsEvent &lhs, const OpsEvent &rhs);

}

#endif