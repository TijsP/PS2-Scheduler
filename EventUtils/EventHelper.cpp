#include "EventHelper.hpp"

#include "opencv2/opencv.hpp"

const char *events::weekdayToString(events::Weekdays weekday){
    switch (weekday)
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

//  returns the wrapped version of the provided string, as well as the width of the largest word (largestWordWidth)
std::string events::wrapString(const std::string &text, cv::FontFace font, int fontSize, int maxWidth, int *largestWordWidth){
    std::string wrappedText;
    std::string word = "";
    std::istringstream textStream(text);
    int largestWidth = 0;
    int currentLineWidth = 0;
    int spaceWidth = cv::getTextSize(cv::Size(), " ", cv::Point(0, fontSize), font, fontSize, 350).width;

    wrappedText.clear();
    while (textStream >> word)
    {
        int wordWidth = cv::getTextSize(cv::Size(), word, cv::Point(0, fontSize), font, fontSize, 350).width;
        if(wordWidth > largestWidth)
            largestWidth = wordWidth;
        
        if(currentLineWidth == 0){
            currentLineWidth += wordWidth;
            wrappedText.append(word);
        }else if(currentLineWidth + spaceWidth + wordWidth > maxWidth){
            wrappedText.append("\n" + word);
            currentLineWidth = wordWidth;
        }else{
            currentLineWidth += spaceWidth + wordWidth;
            wrappedText.append(" " + word);
        }
    }
    if(largestWordWidth)        //  if the pointer has been set
        *largestWordWidth = largestWidth;
    return wrappedText;
}

cv::Scalar events::rgbaToScalar(float colourIn[3]){
    for(int i = 0; i < 3; ++i){
        if(colourIn[i] < 0)
            colourIn = 0;
        if(colourIn[i] > 1)
            colourIn[i] = 1;
    }

    return cv::Scalar(colourIn[2] * 255, colourIn[1] * 255, colourIn[0] * 255);      //  OpenCV prefers BGR
}