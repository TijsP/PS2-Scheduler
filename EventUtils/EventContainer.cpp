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

    cv::Scalar fontColour =  events::rgbaToScalar(scheduledEvents[0]->fontColour);
    std::string opsText =   events::wrapString(scheduledEvents[0]->title, scheduledEvents[0]->font, scheduledEvents[0]->fontSize, width)        + "\n" +
                            events::wrapString(scheduledEvents[0]->description, scheduledEvents[0]->font, scheduledEvents[0]->fontSize, width)  + "\n" +
                            events::wrapString(scheduledEvents[0]->time, scheduledEvents[0]->font, scheduledEvents[0]->fontSize, width)         + "\n" +
                            events::wrapString(scheduledEvents[0]->leader, scheduledEvents[0]->font, scheduledEvents[0]->fontSize, width)       + " ";
    cv::putText(renderfield, opsText, cv::Point(0, scheduledEvents[0]->fontSize), fontColour, scheduledEvents[0]->font, scheduledEvents[0]->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
    
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