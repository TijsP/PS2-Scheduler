#ifndef _EVENTCONTAINER_INCLUDED_
#define _EVENTCONTAINER_INCLUDED_

#include <vector>
#include "OpsEvent.hpp"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"

namespace events{

    class EventContainer{
        private:
        int width, height;
        bool isUnique;      //  a flag indicating whether this instance uses the same settings as other EventContainers

        public:
        Weekdays weekday;
        std::vector<OpsEvent*> scheduledEvents;     //  At the moment, event containers only support a single OpsEvent
        cv::Mat renderfield;
        int pos[2];
        int horizontalSpacing, verticalSpacing;

        EventContainer(int renderfieldWidth, int renderfieldHeight);
        EventContainer();

        bool usesUniqueSettings();
        void UniqueSettings(bool unique);

        bool drawText(bool isPreview = true);

        void changeRenderfieldSize(int newWidth, int newHeight);
        void changeRenderfieldSize(int newWidth, int newHeight, cv::FontFace &newFont);
        int getWidth();
        int getHeight();

        private:
    };

}

#endif