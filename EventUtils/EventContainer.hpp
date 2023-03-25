#ifndef _EVENTCONTAINER_INCLUDED_
#define _EVENTCONTAINER_INCLUDED_

#include <vector>
#include "OpsEvent.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"

// namespace cv{
//     class Mat;
//     class FontFace;
// }

namespace events{

    class EventContainer{
        private:
        cv::FontFace font;
        int fontSize;
        int width, height;

        public:
        Weekdays weekday;
        std::vector<OpsEvent*> scheduledEvents;     //  At the moment, event containers only support a single OpsEvent
        cv::Mat renderfield;
        int pos[2];
        int horizontalSpacing, verticalSpacing;

        EventContainer();
        EventContainer(int renderfieldWidth = 75, int renderfieldHeight = 300);

        void setFontSize(int newFontSize);
        int getFontSize();
        void setFont(cv::FontFace &newFont);
        bool setGlobalFontSize(int &globalFontSize, bool unifyFontSize);
        void setFont(cv::FontFace &newFont, int size);

        bool drawText(bool isPreview = true);

        void changeRenderfieldSize(int newWidth, int newHeight);
        void changeRenderfieldSize(int newWidth, int newHeight, cv::FontFace &newFont);
        int getWidth();
        int getHeight();

        private:
        std::string wrapString(const std::string &text, int *largestWordWidth = nullptr);
    };

}

#endif