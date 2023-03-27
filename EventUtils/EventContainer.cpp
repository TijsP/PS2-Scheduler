#include "EventContainer.hpp"
// #include "opencv2/opencv.hpp"

events::EventContainer::EventContainer() : EventContainer(75, 300){ }

events::EventContainer::EventContainer(int renderfieldWidth, int renderfieldHeight) : 
    font(cv::FontFace("Times New Roman")),
    fontSize(60),
    width(75), height(300),

    weekday(tbd),
    renderfield(cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 255, 0, 255))),
    pos{ 10, 20 },
    horizontalSpacing(36), verticalSpacing(0)
{
    changeRenderfieldSize(renderfieldWidth, renderfieldHeight);
}

void events::EventContainer::setFontSize(int newFontSize){
        fontSize = newFontSize;
        drawText();
    }
int events::EventContainer::getFontSize(){ return fontSize; }
void events::EventContainer::setFont(cv::FontFace &newFont){
    font = newFont;
    drawText();
}
bool events::EventContainer::setGlobalFontSize(int &globalFontSize, bool unifyFontSize){
    bool fontSizeWasChanged = false;
    fontSize = globalFontSize;
    if(unifyFontSize){
        for(auto &event : scheduledEvents){
            float fontScaleFactor = 1.0f;

            int largestWidth = 0;
            int titleTextWidth = 0;
            wrapString(event->title, &titleTextWidth);
            int timeTextWidth = 0;
            wrapString(event->time, &timeTextWidth);
            int leaderTextWidth = 0;
            wrapString(event->leader, &leaderTextWidth);

            largestWidth = (titleTextWidth > largestWidth) ? titleTextWidth : largestWidth;
            largestWidth = (timeTextWidth > largestWidth) ? timeTextWidth : largestWidth;
            largestWidth = (leaderTextWidth > largestWidth) ? leaderTextWidth : largestWidth;

            if(largestWidth > width)
                fontScaleFactor = (float)width / largestWidth;
            
            fontSize = globalFontSize * fontScaleFactor;
            if(fontSize != globalFontSize)
                fontSizeWasChanged = true;
            globalFontSize = fontSize;
#ifdef DEBUG
            std::cout << "font size set to: " << fontSize << std::endl;
#endif
        }
    }

    return fontSizeWasChanged;
}
void events::EventContainer::setFont(cv::FontFace &newFont, int size){
    setFontSize(size);
    setFont(newFont);
}

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

    std::string opsText = events::EventContainer::wrapString(scheduledEvents[0]->title) + "\n" + events::EventContainer::wrapString(scheduledEvents[0]->description) + "\n" + events::EventContainer::wrapString(scheduledEvents[0]->time) + "\n" + events::EventContainer::wrapString(scheduledEvents[0]->leader) + " ";
    cv::putText(renderfield, opsText, cv::Point(0, fontSize), cv::Scalar(251, 255, 140, 255), font, fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
    
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

std::string events::EventContainer::wrapString(const std::string &text, int *largestWordWidth){
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
        }else if(currentLineWidth + spaceWidth + wordWidth > width){
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