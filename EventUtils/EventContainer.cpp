#include "EventContainer.hpp"
#include "EventHelper.hpp"

events::EventContainer::EventContainer() : EventContainer(75, 300){ }

events::EventContainer::EventContainer(int renderfieldWidth, int renderfieldHeight) : 
    width(75), height(300),
    isUnique(false),
    previousUnformattedTextStartHeight(0),

    weekday(tbd),
    renderfield(cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 255, 0, 255))),
    pos{ 10, 20 },
    horizontalSpacing(36), verticalSpacing(0)
{
    changeRenderfieldSize(renderfieldWidth, renderfieldHeight);
}

bool events::EventContainer::usesUniqueSettings(){ return isUnique; }
void events::EventContainer::UniqueSettings(bool unique){ isUnique = unique; }

bool events::EventContainer::drawText(bool syncTextHeight, bool isPreview){
    bool textNeedsRedraw = false;

    renderfield = cv::Scalar(0, 0, 0, 0);
    if(isPreview)
        cv::rectangle(renderfield, cv::Rect(0, 0, width, height), cv::Scalar(0, 255, 0, 255), 4);
    if(scheduledEvents.size() == 0){
#ifdef DEBUG
        std::cout << "No events found!" << std::endl;
#endif
    }

    std::string opsText = "";
    int textStartHeight = 0;
    int unformattedTextStartHeight = 0;
    int textStartOffset = 0;
    static int titleStartHeight = 0, descriptionStartHeight = 0, timeStartHeight = 0, leaderStartHeight = 0;
    for (auto &&event : scheduledEvents)
    {
        int singleLineHeight = cv::getTextSize(cv::Size(), " ", cv::Point(0, 100), event->font, event->fontSize, 390).height / 2;
        textStartHeight += event->fontSize;
        cv::Scalar fontColour =  events::rgbaToScalar(event->fontColour);

        std::string titleText = events::wrapString(event->title, event->font, event->fontSize, width) + "\n";
        cv::putText(renderfield, titleText, cv::Point(0, textStartHeight), fontColour, event->font, event->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        textStartOffset = (!event->title.empty() ? cv::getTextSize(cv::Size(), titleText, cv::Point(0, textStartHeight), event->font, event->fontSize, 390).height : singleLineHeight) + event->verticalPadding;
        unformattedTextStartHeight += textStartOffset;
        if(syncTextHeight){
            if(textStartOffset > descriptionStartHeight) {
                descriptionStartHeight = textStartOffset;
                textNeedsRedraw = true;
                }
            else textStartOffset = descriptionStartHeight;
        }
        textStartHeight += textStartOffset;
        
        std::string descriptionText = events::wrapString(event->description, event->font, event->fontSize, width) + "\n";
        cv::putText(renderfield, descriptionText, cv::Point(0, textStartHeight), fontColour, event->font, event->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        textStartOffset = (!event->description.empty() ? cv::getTextSize(cv::Size(), descriptionText, cv::Point(0, textStartHeight), event->font, event->fontSize, 390).height : singleLineHeight) + event->verticalPadding;
        unformattedTextStartHeight += textStartOffset;
        if(syncTextHeight){
            if(textStartOffset > timeStartHeight) {
                timeStartHeight = textStartOffset;
                textNeedsRedraw = true;
                }
            else textStartOffset = timeStartHeight;
        }
        textStartHeight += textStartOffset;
        
        std::string timeText = events::wrapString(event->time, event->font, event->fontSize, width) + "\n";
        cv::putText(renderfield, timeText, cv::Point(0, textStartHeight), fontColour, event->font, event->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        textStartOffset = (!event->time.empty() ? cv::getTextSize(cv::Size(), timeText, cv::Point(0, textStartHeight), event->font, event->fontSize, 390).height : singleLineHeight) + event->verticalPadding;
        unformattedTextStartHeight += textStartOffset;
        if(syncTextHeight){
            if(textStartOffset > leaderStartHeight) {
                leaderStartHeight = textStartOffset;
                textNeedsRedraw = true;
                }
            else textStartOffset = leaderStartHeight;
        }
        textStartHeight += textStartOffset;
        
        std::string leaderText = events::wrapString(event->leader, event->font, event->fontSize, width);
        cv::putText(renderfield, leaderText, cv::Point(0, textStartHeight), fontColour, event->font, event->fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        textStartOffset = (!event->leader.empty() ? cv::getTextSize(cv::Size(), leaderText, cv::Point(0, textStartHeight), event->font, event->fontSize, 390).height : singleLineHeight) + event->verticalPadding;
        unformattedTextStartHeight += textStartOffset;
        if(syncTextHeight){
            if(textStartOffset > titleStartHeight) {
                titleStartHeight = textStartOffset;
                textNeedsRedraw = true;
                }
            else textStartOffset = titleStartHeight;
        }
        textStartHeight += textStartOffset;
    }
    //  should not be independent from syncTextHeight?
    if(previousUnformattedTextStartHeight == unformattedTextStartHeight){
        textNeedsRedraw = false;
    }
    else{
        textNeedsRedraw = true;
        if(previousUnformattedTextStartHeight > unformattedTextStartHeight){
            titleStartHeight = 0; descriptionStartHeight = 0; timeStartHeight = 0; leaderStartHeight = 0;
        }
    }
    previousUnformattedTextStartHeight = unformattedTextStartHeight;
    
    return textNeedsRedraw;
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