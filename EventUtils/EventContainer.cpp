#include "EventContainer.hpp"
#include "EventHelper.hpp"

events::EventContainer::EventContainer() : EventContainer(75, 300){ }

events::EventContainer::EventContainer(int renderfieldWidth, int renderfieldHeight) : 
    width(75), height(300),
    isUnique(false),

    weekday(tbd),
    renderfield(cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 255, 0, 255))),
    pos{ 10, 20 },
    horizontalSpacing(36), verticalSpacing(0)
{
    changeRenderfieldSize(renderfieldWidth, renderfieldHeight);
}

bool events::EventContainer::usesUniqueSettings(){ return isUnique; }
void events::EventContainer::UniqueSettings(bool unique){ isUnique = unique; }

bool events::EventContainer::drawText(bool isPreview){
    renderfield = cv::Scalar(0, 0, 0, 0);
    if(isPreview)
        cv::rectangle(renderfield, cv::Rect(0, 0, width, height), cv::Scalar(0, 255, 0, 255), 4);
    if(scheduledEvents.size() == 0){
#ifdef DEBUG
        std::cout << "No events found!" << std::endl;
#endif
        return false;
        }

    std::string opsText = "";
    int textStartHeight = 0;
    for (auto &&event : scheduledEvents)
    {
        textStartHeight += event->fontSize;
        cv::Scalar fontColour =  events::rgbaToScalar(event->fontColour);
        opsText =   events::wrapString(event->title, event->font, event->fontSize, width)        + "\n" +
                    events::wrapString(event->description, event->font, event->fontSize, width)  + "\n" +
                    events::wrapString(event->time, event->font, event->fontSize, width)         + "\n" +
                    events::wrapString(event->leader, event->font, event->fontSize, width)       + " ";
        cv::putText(renderfield, opsText, cv::Point(0, textStartHeight), fontColour, event->font, event->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        textStartHeight += cv::getTextSize(cv::Size(), opsText, cv::Point(0, textStartHeight), event->font, event->fontSize, 390).height;
    }
    
    return true;
}

void events::EventContainer::changeRenderfieldSize(int newWidth, int newHeight){
    width = newWidth; height = newHeight;
    if(!renderfield.empty()){
        renderfield = cv::Scalar(0, 0, 0, 255);
        cv::resize(renderfield, renderfield, cv::Size(newWidth, newHeight));
    }
}
void events::EventContainer::changeRenderfieldSize(int newWidth, int newHeight, cv::FontFace &newFont){
    width = newWidth; height = newHeight;
    renderfield = cv::Scalar(0, 0, 0, 255);
    cv::resize(renderfield, renderfield, cv::Size(newWidth, newHeight));
}
int events::EventContainer::getWidth(){
    return width;
}
int events::EventContainer::getHeight(){
    return height;
}