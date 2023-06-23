#ifndef _EVENTHELPER_INCLUDED_
#define _EVENTHELPER_INCLUDED_

#include <fstream>
#include <string>

#include "opencv2/core/types.hpp"

namespace cv{
    class FontFace;
}

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
    const char *weekdayToString(events::Weekdays weekday);
    
    std::string wrapString(const std::string &text, cv::FontFace font, int fontSize, int maxWidth, int *largestWordWidth = nullptr);

    cv::Scalar rgbaToScalar(float colourIn[3]);

}

#endif