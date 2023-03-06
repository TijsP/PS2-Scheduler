#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <ctime>
#include <any>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <GLFW/glfw3.h>
#include "TinyFD/tinyfiledialogs.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_stdlib.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")     //  prevents console opening automatically

//  TODO: fill out settings bar
//  TODO: create OPS/layout/advanced settings tabs
//  DONE: create custom text wrapping function
//  DONE: create OPS data struct

//  TODO: create log file
//  DONE: save/load schedule
//  DONE: export schedule
//  DONE: implement working directory
//  DONE: implement resizing window size to the size of the previous instance (through settings.ini)
//  DONE: implement ostream/istream of OpsEvent. Newline characters should not interfere with the method used to retrieve the string (i.e. extraction operator should be able to deal with newlines)
//  DONE: save user preferences (including working directory, last window size)
//  DONE: add date and time of creation to exported schedule filename

//  BUGS:
//  - app crashes when previewROI in renderPreview() tries to access non-existant data when the parameters involved in size and position are too large
//      - in addition, the app crashes when an image is loaded which is smaller than the original background image. This is caused by the same bug
//  - with select container parameters and font sizes, the unify font size function doesn't work properly and allows a single character to overflow onto the next line
//  - when the squad title/time occupies too many lines, the subsequent fields are pushed out of line compared to the same fields in different containers

#define APP_VERSION "0.1.1"
// #define DEBUG

#define FINAL_BUILD

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
std::ostream &operator<<(std::ostream &output, const Weekdays &weekday){
    int weekdayInt = weekday;
    output << weekdayInt;
    return output;
}
std::istream &operator>>(std::istream &input, Weekdays &weekday){
    int weekdayInt;
    input >> weekdayInt;
    weekday = (Weekdays)weekdayInt;
    return input;
}

class EventContainer;

struct OpsEvent{
    
    Weekdays weekday = Monday;
    std::string title = "";
    std::string renderedTitle = "";
    std::string description = "";
    std::string renderedDescription = "";
    std::string leader = "";
    std::string renderedLeader = "";
    std::string time = "";
    std::string renderedTime = "";

    OpsEvent(std::string title, std::string leader, Weekdays weekday, std::string time = "", std::string description = "") : title(title), description(description), leader(leader), time(time), weekday(weekday) { }

    OpsEvent &operator= (const OpsEvent &rhs){
        weekday = rhs.weekday;
        title = rhs.title;
        description = rhs.description;
        leader = rhs.leader;
        time = rhs.time;

        return *this;
    }

    const static char *returnWeekday(Weekdays weekdayOfInterest){
        switch (weekdayOfInterest)
        {
        case Monday:
            return "Monday";
            break;
        case Tuesday:
            return "Tuesday";
            break;
        case Wednesday:
            return "Wednesday";
            break;
        case Thursday:
            return "Thursday";
            break;
        case Friday:
            return "Friday";
            break;
        case Saturday:
            return "Saturday";
            break;
        case Sunday:
            return "Sunday";
            break;
        case tbd:
            return "t.b.d.";
            break;
        
        default:
            return "Invalid";
            break;
        }
    }

};
std::ostream &operator<<(std::ostream &output, const OpsEvent &opsevent){
    //  "/u009C" is the unicode character for unit separator
    output << opsevent.title << "\t" << opsevent.description << "\t" << opsevent.leader << "\t" << opsevent.time << "\t" << opsevent.weekday;
    return output;
}
std::istream &operator>>(std::istream &input, OpsEvent &opsevent){
    std::getline(input, opsevent.title, '\t');
    std::getline(input, opsevent.description, '\t');
    std::getline(input, opsevent.leader, '\t');
    std::getline(input, opsevent.time, '\t');
    input >> opsevent.weekday;
    
    return input;
}
bool compareByWeekday (const OpsEvent &lhs, const OpsEvent &rhs){
    return lhs.weekday < rhs.weekday;
}

class EventContainer{
    private:
    cv::FontFace &font = cv::FontFace("Times New Roman");
    int fontSize = 60;
    int width = 75, height = 300;

    public:
    Weekdays weekday = tbd;
    std::vector<OpsEvent*> scheduledEvents;
    cv::Mat renderfield = cv::Mat(300, 75, CV_8UC4, cv::Scalar(0, 255, 0, 255));
    int pos[2] = { 10, 20 };

    EventContainer();
    EventContainer(int renderfieldWidth = 75, int renderfieldHeight = 300) : width(renderfieldWidth), height(renderfieldHeight) { changeRenderfieldSize(renderfieldWidth, renderfieldHeight); };

    void setFontSize(int newFontSize){
        fontSize = newFontSize;
        drawText();
    }
    int getFontSize(){ return fontSize; }
    void setFont(cv::FontFace &newFont){
        font = newFont;
        drawText();
    }
    bool setGlobalFontSize(int &globalFontSize, bool unifyFontSize){
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
    void setFont(cv::FontFace &newFont, int size){
        setFontSize(size);
        setFont(newFont);
    }

    bool drawText(bool isPreview = true){
        renderfield = cv::Scalar(0, 0, 0, 0);
        if(isPreview)
            cv::rectangle(renderfield, cv::Rect(0, 0, width, height), cv::Scalar(0, 255, 0, 255), 4);
        if(scheduledEvents.size() == 0){
#ifdef DEBUG
            std::cout << "No events found!" << std::endl;
#endif
            return false;
            }

        std::string opsText = wrapString(scheduledEvents[0]->title) + "\n" + wrapString(scheduledEvents[0]->description) + "\n" + wrapString(scheduledEvents[0]->time) + "\n" + wrapString(scheduledEvents[0]->leader) + " ";
        cv::putText(renderfield, opsText, cv::Point(0, fontSize), cv::Scalar(251, 255, 140, 255), font, fontSize, 390, cv::PUT_TEXT_WRAP, cv::Range(0, width));
        
        return true;
    }

    void changeRenderfieldSize(int newWidth, int newHeight){
        width = newWidth; height = newHeight;
        renderfield = cv::Scalar(0, 0, 0, 255);
        cv::resize(renderfield, renderfield, cv::Size(newWidth, newHeight));
        drawText();
    }
    void changeRenderfieldSize(int newWidth, int newHeight, cv::FontFace &newFont){
        width = newWidth; height = newHeight;
        renderfield = cv::Scalar(0, 0, 0, 255);
        cv::resize(renderfield, renderfield, cv::Size(newWidth, newHeight));
        drawText();
    }
    int getWidth(){
        return width;
    }
    int getHeight(){
        return height;
    }

    private:
    std::string wrapString(const std::string &text, int *largestWordWidth = nullptr){
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
};
void renderPreview(std::vector<EventContainer> &containers, const cv::Mat &background, cv::Mat &preview, int &globalFontSize, bool unifyFontSize = false, bool isPreview = true){
    preview = background.clone();

    //  needs to be done before drawing any of the text, to ensure all containers use the same size font
    for(int i = 0; i < containers.size(); ++i)
        if(containers[i].setGlobalFontSize(globalFontSize, unifyFontSize))
            i = -1;      //  if font size was changed, reiterate over all containers to ensure the font size is up to date
    
    for(auto &container : containers){
        container.drawText(isPreview);
        cv::Mat previewROI = preview(cv::Rect(container.pos[0], container.pos[1], container.getWidth(), container.getHeight()));
        cv::add(container.renderfield, previewROI, previewROI);
    }
}
void setContainerSpacing(std::vector<EventContainer> &containers, int startPos[2], int horizontalSpacing, int verticalSpacing = 0){
    for(int i = 0; i < containers.size(); i++){
        containers[i].pos[0] = startPos[0] + i * (horizontalSpacing + containers[i].getWidth());
        if(verticalSpacing)
            containers[i].pos[1] = startPos[1] + i * (verticalSpacing + containers[i].getHeight());
        else
            containers[i].pos[1] = startPos[1];
    }
}
void rebindOpsEvents(std::vector<OpsEvent> &opsEvents, std::vector<EventContainer> &eventContainers){
    for(auto &container : eventContainers){
        container.scheduledEvents.clear();
#ifdef DEBUG
        std::cout << "number of scheduled events in this container: " << container.scheduledEvents.size() << std::endl;
#endif
        for(auto &ops : opsEvents){
            if(container.weekday == ops.weekday){
                container.scheduledEvents.push_back(&ops);
            }
        }
    }
}

void loadSettings(std::ifstream &inStream);
void saveSettings(std::ofstream &outStream);

void loadOpsEvents(std::ifstream &inStream, std::vector<OpsEvent> &opsEvents);
void saveOpsEvents(std::ofstream &outStream, const std::vector<OpsEvent> &opsEvents);

const char *imageFilters[] = {"*.png", "*.jpg"};
    const char *saveFileFormats[] = { "*.sav" };

//  Dear ImGUI texture load example, modified to use OpenCV as the image loader
bool LoadTextureToMemory(cv::Mat image, GLuint* out_texture, int* out_width, int* out_height)
{
    // Perform necessary checks
    if (image.empty())
        return false;
    cv::cvtColor(image, image, cv::COLOR_RGBA2RGB);
    image.convertTo(image, CV_8UC3);
    int image_width = image.size().width;
    int image_height = image.size().height;
#ifdef DEBUG
    std::cout << "schedule width: " << image_width << std::endl;
    std::cout << "schedule height: " << image_height << std::endl;
#endif

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    if(image.isContinuous()){
        //  issue when passing image width and height
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, image_width, image_height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, image.data);
#ifdef DEBUG
        std::cout << "Schedule mat is continuous" << std::endl;
#endif
    }
    else {
#ifdef DEBUG
        std::cout << "Schedule mat is not continous" << std::endl;
#endif
        return false;
    }

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Window state
int currentWindowWidth = 850, currentWindowHeight = 500;
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//  Basic schedule editor parameters
const int settingsBarWidth = 350;
const int schedulePreviewMinimumWidth = 500;
const int minimumWindowWidth = settingsBarWidth + schedulePreviewMinimumWidth;
const int minimumWindowHeight = 400;

bool unifyFontSize = false;
int fontSize = 60;

std::vector<OpsEvent> OpsEvents;
OpsEvent dummyEvent = OpsEvent("", "", tbd, "");
OpsEvent defaultEvent = OpsEvent("New OPS", "Leader", Monday, "Time");
static int selectedEventIndex = -1;
bool OpsEventAdded = true;
bool OpsEventWeekdayChanged = false;

std::vector<EventContainer> eventContainers;
int firstEventContainerPos[2] = { 25, 295 };
int eventContainerSize[2] = { 250, 755 };
int eventCeventContainerHorizontalSpacing = 36, eventCeventContainerVerticalSpacing = 0;
bool showEventContainerBoundingBox = true;
bool eventContainerParametersChanged = false;
bool redrawEventContainer = false;

std::string workingDirectoryPath = std::filesystem::current_path().string();
#ifdef FINAL_BUILD
std::string schedulePath = workingDirectoryPath + "\\TXLC_Planning.png";
#else
std::string schedulePath = "..\\..\\TXLC_Planning.png";
#endif

int main(int, char**) {
    std::time_t currentTime = std::time({});
    std::ofstream saveSettingsStream;
    std::ifstream loadSettingsStream;

    loadSettingsStream.open("settings.ini");
    loadSettings(loadSettingsStream);
    loadSettingsStream.close();
    

#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    //TODO: set GLFW error callback?
    if(!glfwInit())
        return 1;
    
    std::string appName = "PS2 Squad Scheduler v";
    appName.append(APP_VERSION);
    GLFWwindow *windowContainer = glfwCreateWindow(currentWindowWidth, currentWindowHeight, appName.c_str(), NULL, NULL);
    if(windowContainer == NULL)
        return 1;
    glfwMakeContextCurrent(windowContainer);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle mainStyle = ImGui::GetStyle();
    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(windowContainer, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

#ifdef DEBUG
    if(!glfwInit()){
        std::cout << "GLFW failed to initiate..." << std::endl;
    }
#endif

    int windowsX = 0;

    cv::Mat scheduleBackground = cv::imread(schedulePath, cv::IMREAD_COLOR);
    cv::cvtColor(scheduleBackground, scheduleBackground, cv::COLOR_RGB2RGBA);
    cv::Mat schedulePreview = scheduleBackground.clone();
    GLuint schedulePreviewID;
    int scheduleWidth = -1, scheduleHeight = -1;
#ifdef DEBUG
    std::cout << "schedule width: " << scheduleWidth << std::endl;
    std::cout << "schedule height: " << scheduleHeight << std::endl;
#endif

// #ifdef DEBUG
//     //  fill OpsEvents for testing purposes
//     OpsEvents.push_back(OpsEvent("test title", "TAJp", Wednesday, "8PM", "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent ut nunc arcu. In auctor volutpat erat, sed pellentesque elit aliquet vitae. Nullam porttitor tortor eget ex interdum, vitae laoreet nulla pretium. Maecenas at imperdiet nibh. Nunc eu justo sit amet libero eleifend cursus. Donec eget quam dui. Vivamus porta, arcu et bibendum egestas, nunc ante aliquam nulla, quis venenatis est quam quis enim. Aliquam gravida sed urna et aliquam. Phasellus semper tempor nisl ut sagittis. Nulla mollis tellus vitae ex eleifend, sed commodo sem volutpat. Pellentesque dictum porttitor diam a tempus. Aliquam at gravida diam. Interdum et malesuada fames ac ante ipsum primis in faucibus. Sed iaculis porttitor sapien, at pharetra ligula vestibulum ut."));
//     OpsEvents.push_back(OpsEvent("test title 2", "ratatanFLO44", Friday, "9PM"));
//     OpsEvents.push_back(OpsEvent("test title 3", "ratatanFLO44", Friday, "9PM"));
//     OpsEvents.push_back(OpsEvent("test title", "TAJp", Thursday, "7PM"));
//     OpsEvents.push_back(OpsEvent("test title", "Rancy", Monday, "7PM"));
// #endif

    for(int i = 0; i < 7; ++i){
        eventContainers.push_back(EventContainer(eventContainerSize[0], eventContainerSize[1]));
        eventContainers[i].weekday = (Weekdays)i;
        for(auto &event : OpsEvents)
            if(i == event.weekday) eventContainers[i].scheduledEvents.push_back(&event);
    }
#ifdef DEBUG
    for(int i = 0; i < 7; ++i){
        std::cout << OpsEvent::returnWeekday((Weekdays)i) << " has " << eventContainers[i].scheduledEvents.size() << " events planned"<< std::endl;
    }
#endif
    setContainerSpacing(eventContainers, firstEventContainerPos, eventCeventContainerHorizontalSpacing);
    renderPreview(eventContainers, scheduleBackground, schedulePreview, fontSize);
    LoadTextureToMemory(schedulePreview, &schedulePreviewID, &scheduleWidth, &scheduleHeight);

    while(!glfwWindowShouldClose(windowContainer)){
        //  GLFW and Dear ImGui setup
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        bool basicSchedulerShouldClose = false;

        //  Begin custom window
        static ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        //  Begin schedule preview
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - settingsBarWidth, viewport->WorkSize.y));
        ImGui::Begin("Image preview##preview_window", &basicSchedulerShouldClose, windowFlags);

        //  Ensure window isn't made too small
        glfwGetWindowSize(windowContainer, &currentWindowWidth, &currentWindowHeight);
        if(currentWindowWidth < minimumWindowWidth) glfwSetWindowSize(windowContainer, minimumWindowWidth, currentWindowHeight);
        if(currentWindowHeight < minimumWindowHeight) glfwSetWindowSize(windowContainer, currentWindowWidth, minimumWindowHeight);

        /*  GUI elements  */
        ImGui::Image((void*)(intptr_t)schedulePreviewID, ImVec2((float)ImGui::GetContentRegionAvail().x, (float)scheduleHeight * ImGui::GetContentRegionAvail().x / scheduleWidth));
        float windowPadding = mainStyle.WindowPadding.y;

        //  Schedule working directory
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Working directory:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 150);
        ImGui::InputTextWithHint("##working_directory", "Please select a working directory...", &workingDirectoryPath); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Currently selected working directory");
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        bool selectWorkingDirectoryPath = ImGui::Button("Select directory", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Select main directory to save/load files from");

        float buttonWidth = (ImGui::GetContentRegionAvail().x - mainStyle.WindowPadding.x) / 3 - mainStyle.FramePadding.x;
        bool saveSchedule = ImGui::Button("Save schedule project", ImVec2(buttonWidth, ImGui::GetFrameHeight() * 2)); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Saves all events to a file"); ImGui::SameLine();
        bool loadSchedule = ImGui::Button("Load schedule project", ImVec2(buttonWidth, ImGui::GetFrameHeight() * 2)); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Load events from a file"); ImGui::SameLine();
        bool exportSchedule = ImGui::Button("Export schedule", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() * 2)); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Export schedule as an image");
        bool loadScheduleBackground = ImGui::Button("Load schedule background", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight() * 2)); if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) ImGui::SetTooltip("Load background image"); ImGui::SameLine();

        /*  GUI functions  */
        if(selectWorkingDirectoryPath){
            char *directoryPathInput = tinyfd_selectFolderDialog("Select working directory", workingDirectoryPath.c_str());
            if(directoryPathInput) workingDirectoryPath = directoryPathInput;// strcpy_s(workingDirectoryPath, sizeof(workingDirectoryPath), directoryPathInput);
        }
        if(buttonWidth){ }
        if(saveSchedule){ 
            std::string path = workingDirectoryPath;;
            path.append("\\schedule_project.jpg");

            char *projectSavePath = tinyfd_saveFileDialog("Schedule project save location", path.c_str(), 1, saveFileFormats, "Project save file");
            if(projectSavePath){
                std::ofstream saveEventsStream(projectSavePath);
                saveOpsEvents(saveEventsStream, OpsEvents);
            }
        }
        if(loadSchedule){ 
            std::string path = workingDirectoryPath;
            path.append("\\*.sav");
            char *projectLoadPath = tinyfd_openFileDialog("Schedule project save location", path.c_str(), 1, saveFileFormats, "Project save file", 0);
            if(projectLoadPath){
                OpsEvents.clear();
                std::ifstream loadEventsStream(projectLoadPath);
                loadOpsEvents(loadEventsStream, OpsEvents);
                OpsEventAdded = true;
            }
        }
        if(exportSchedule){
            std::string fileName = "schedule_";
            char date[std::size("YYYY-MM-DD_HHhMMm")];
            std::strftime(date, sizeof(date), "%F_%Hh%Mm", std::localtime(&currentTime));
            fileName.append(date);
            fileName.append(".jpg");

            cv::Mat scheduleDefinitive = scheduleBackground.clone();
            renderPreview(eventContainers, scheduleBackground, scheduleDefinitive, fontSize, unifyFontSize, false);
            cv::imwrite(fileName, scheduleDefinitive);
        }
        if(loadScheduleBackground){
            std::string path = workingDirectoryPath;
            path.append("\\*.*");
            
            char *filepathInput = tinyfd_openFileDialog("Select schedule background", path.c_str(), 2, imageFilters, "image files", 0);
            if(filepathInput) schedulePath = filepathInput;
            scheduleBackground = cv::imread(schedulePath, cv::IMREAD_COLOR);
            LoadTextureToMemory(scheduleBackground, &schedulePreviewID, &scheduleWidth, &scheduleHeight);
        }

        //  End schedule preview window
        ImGui::End();

        //  Begin settings bar
        ImGui::SetNextWindowPos(ImVec2((float)currentWindowWidth - settingsBarWidth - 1, viewport->WorkPos.y));    //  Additional -1 for cleaner border between windows
        ImGui::SetNextWindowSize(ImVec2((float)settingsBarWidth + 1, viewport->WorkSize.y));                       //  Additional +1 because the entire settings bar window got shifted one pixel to the left
        ImGui::Begin("Settings bar##settings_window", &basicSchedulerShouldClose, windowFlags);

        ImGui::Text("Event container position:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The position (x, y) of the first\ncontainer in pixels, counted from\nthe top left");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 140);
        ImGui::SetNextItemWidth(140);
        ImGui::InputInt2("##container_position", firstEventContainerPos);
        if(ImGui::IsItemDeactivatedAfterEdit())
            eventContainerParametersChanged = true;

        ImGui::Text("Event container size:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The size (width, height) of the\ncontainers, in pixels");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 140);
        ImGui::SetNextItemWidth(140);
        ImGui::InputInt2("##container_size", eventContainerSize, 0);
        if(ImGui::IsItemDeactivatedAfterEdit())
            eventContainerParametersChanged = true;

        ImGui::Text("Event container spacing:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The amount of space between the\ncontainers, in pixels");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 140);
        ImGui::SetNextItemWidth(140);
        ImGui::InputInt("##container_horizontal_spacing", &eventCeventContainerHorizontalSpacing, 0);
        if(ImGui::IsItemDeactivatedAfterEdit())
            eventContainerParametersChanged = true;

        ImGui::Text("Show container borders:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Show the bounding boxes of the\nevent containers");
            // ImGui::SetTooltip("When selected, font size will\nautomatically be adjusted downward\nto fit within the event container");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 140);
        ImGui::Checkbox("##show_event_containers", &showEventContainerBoundingBox);
        if(ImGui::IsItemDeactivatedAfterEdit())
            eventContainerParametersChanged = true;

        ImGuiTableFlags defaultTableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
        if(ImGui::BeginTable("ops_table", 5, defaultTableFlags, ImVec2(0.0, ImGui::GetFrameHeight() * 6))){
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Squad title", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthFixed, 25.0f);
            ImGui::TableSetupColumn("Leader", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_None);
            ImGui::TableSetupColumn("Day", ImGuiTableColumnFlags_PreferSortAscending);      //  TODO: implement sorting base on weekday
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(OpsEvents.size());
            while (clipper.Step())
            {
                for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++){
                    std::string id = "Ops event entry ";    //  make sure every entry is associated with an unique ID
                    id.append(std::to_string(i + 1));
                    ImGui::PushID(id.c_str());

                    ImGui::TableNextRow();                  //  add relevant items to the table
                    ImGui::TableSetColumnIndex(0);
                    const bool isSelected = (selectedEventIndex == i) ? true : false;
                    if(ImGui::Selectable(OpsEvents[i].title.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                        selectedEventIndex = i;
                    if(isSelected) ImGui::SetItemDefaultFocus();
                    ImGui::TableNextColumn();
                    ImGui::Text(OpsEvents[i].description.c_str()); ImGui::TableNextColumn();
                    ImGui::Text(OpsEvents[i].leader.c_str()); ImGui::TableNextColumn();
                    ImGui::Text(OpsEvents[i].time.c_str()); ImGui::TableNextColumn();
                    ImGui::Text(OpsEvents[i].returnWeekday(OpsEvents[i].weekday)); ImGui::TableNextColumn();

                    ImGui::PopID();                         //  pop custom ID
                }
            }
            ImGui::EndTable();
        }
        
        OpsEvent *selectedOpsEvent = &dummyEvent;
        static int selectedWeekday = dummyEvent.weekday;

        ImGui::Text("Add/remove event:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Add new event, or remove the\nselected event");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        buttonWidth = (ImGui::GetContentRegionAvail().x - mainStyle.WindowPadding.x) / 2;// - mainStyle.FramePadding.x;
        if(ImGui::Button("+##add_event", ImVec2(buttonWidth, ImGui::GetFrameHeight()))){
            OpsEvents.push_back(defaultEvent);
            OpsEventAdded = true;
        }
        ImGui::SameLine();
        if(OpsEvents.size() == 0)
            selectedEventIndex = -1;
        if(selectedEventIndex < 0){
            ImGui::BeginDisabled();
            selectedOpsEvent = &dummyEvent;
        }
        else{
            selectedOpsEvent = &OpsEvents[selectedEventIndex];
            selectedWeekday = selectedOpsEvent->weekday;
        }
        if(ImGui::Button("-##remove_event", ImVec2(buttonWidth, ImGui::GetFrameHeight()))){
            if(selectedEventIndex >= 0){
                OpsEvents.erase(OpsEvents.begin() + selectedEventIndex);
                OpsEventAdded = true;
            }
        }
        ImGui::Text("Squad title:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Title of the squad");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##title", "title...", &selectedOpsEvent->title);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;
        
        ImGui::Text("Squad Description:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Description (currently       \nnon-functional)");
            // ImGui::SetTooltip("When selected, font size will\nautomatically be adjusted downward\nto fit within the event container");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##description", "coming soon...", &dummyEvent.description, ImGuiInputTextFlags_ReadOnly);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;
        
        ImGui::Text("Leader:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Leader of the squad");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##leader", "Leader name...", &selectedOpsEvent->leader);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;
        
        ImGui::Text("Start time:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Starting time");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputTextWithHint("##time", "time...", &selectedOpsEvent->time);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;
        
        const char *weekdaysArray[8] = { OpsEvent::returnWeekday(Monday),
                                        OpsEvent::returnWeekday(Tuesday),
                                        OpsEvent::returnWeekday(Wednesday),
                                        OpsEvent::returnWeekday(Thursday),
                                        OpsEvent::returnWeekday(Friday),
                                        OpsEvent::returnWeekday(Saturday),
                                        OpsEvent::returnWeekday(Sunday),
                                        OpsEvent::returnWeekday(tbd) };
        ImGui::Text("Weekday:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The day of the week the event\nwill take place");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::Combo("##weekday", &selectedWeekday, weekdaysArray, 8);
        selectedOpsEvent->weekday = (Weekdays)selectedWeekday;
        if(ImGui::IsItemEdited()){
            redrawEventContainer = true;
            OpsEventWeekdayChanged = true;
        }

        ImGui::EndDisabled();

        ImGui::Text("Font size:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("The size of the font, in pixels");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::InputInt("##font_size", &fontSize, 0);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;

        ImGui::Text("Unify font size:");
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("When selected, font size will\nautomatically be adjusted downward\nto fit within the event container");
        ImGui::SameLine(ImGui::GetContentRegionMax().x - 200);
        ImGui::Checkbox("##unify_font_size", &unifyFontSize);
        if(ImGui::IsItemDeactivatedAfterEdit())
            redrawEventContainer = true;

        if(OpsEventAdded | OpsEventWeekdayChanged){
            std::sort(OpsEvents.begin(), OpsEvents.end(), compareByWeekday);
            rebindOpsEvents(OpsEvents, eventContainers);
#ifdef DEBUG
            std::cout << "Ops events found: " << OpsEvents.size() << std::endl;
#endif
            redrawEventContainer = true;
            OpsEventAdded = false;
            OpsEventWeekdayChanged = false;
        }
        if(eventContainerParametersChanged){
            eventContainerSize[0] = (eventContainerSize[0] <= 0) ? 1 : eventContainerSize[0];     //  size can't be less than or equal to 0
            eventContainerSize[1] = (eventContainerSize[1] <= 0) ? 1 : eventContainerSize[1];
            for (auto &eventContainer : eventContainers)
            {
                eventContainer.changeRenderfieldSize(eventContainerSize[0], eventContainerSize[1]);
            }
            setContainerSpacing(eventContainers, firstEventContainerPos, eventCeventContainerHorizontalSpacing);

            redrawEventContainer = true;
            eventContainerParametersChanged = false;
        }
        if(redrawEventContainer){
            renderPreview(eventContainers, scheduleBackground, schedulePreview, fontSize, unifyFontSize, showEventContainerBoundingBox);
            LoadTextureToMemory(schedulePreview, &schedulePreviewID, &scheduleWidth, &scheduleHeight);
#ifdef DEBUG
            std::cout << "unify font size: " << unifyFontSize << std::endl;
#endif
            redrawEventContainer = false;
        }

        //  End settings bar
        ImGui::End();

#ifdef DEBUG
        ImGui::ShowDemoWindow();
#endif

        //  Display windows
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(windowContainer, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(windowContainer);
    }

    saveSettingsStream.open("settings.ini");
    saveSettings(saveSettingsStream);
    saveSettingsStream.close();

    //  Window cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(windowContainer);
    glfwTerminate();
}

#ifdef DEBUG
#define DEBUG_LOADED_PARAMS_MESSAGE(Tag, ParamStream) std::cout << "Succesfully loaded [" << Tag << "] with parameters: " << ParamStream << std::endl;
#else
#define DEBUG_LOADED_PARAMS_MESSAGE(Tag, ParamStream)
#endif
void loadSettings(std::ifstream &inStream){
    std::string settingsTag = "";
    while(std::getline(inStream, settingsTag)){
        std::string params;

        if(settingsTag == "unify-font-size"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> unifyFontSize)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, unifyFontSize)
        }else if(settingsTag == "font-size"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> fontSize)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, fontSize)
        }else if(settingsTag == "first-container-pos"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> firstEventContainerPos[0] >> firstEventContainerPos[1])){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, firstEventContainerPos[0] << " " << firstEventContainerPos[1])
        }else if(settingsTag == "container-size"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> eventContainerSize[0] >> eventContainerSize[1])){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, eventContainerSize[0] << " " << eventContainerSize[1])
        }else if(settingsTag == "container-spacing"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> eventCeventContainerHorizontalSpacing >> eventCeventContainerVerticalSpacing)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, eventCeventContainerHorizontalSpacing << " " << eventCeventContainerVerticalSpacing)
        }else if(settingsTag == "show-container-bounding-box"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> showEventContainerBoundingBox)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, showEventContainerBoundingBox)
        }else if(settingsTag == "working-directory-path"){
            std::getline(inStream, workingDirectoryPath);
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, workingDirectoryPath)
        }else if(settingsTag == "window-size"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> currentWindowWidth >> currentWindowHeight)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, currentWindowWidth << " " << currentWindowHeight)
        }
        
        else if(settingsTag == ""){
            continue;
        }
        else{
#ifdef DEBUG
            std::cout << "Unknown tag/parameter \"" << settingsTag << "\"" << std::endl;
#endif
            continue;
        }
    }
}
void saveSettings(std::ofstream &outStream){
    outStream << "Font settings:" << std::endl;
    outStream << "unify-font-size\n" << unifyFontSize << std::endl;
    outStream << "font-size\n" << fontSize << std::endl;

    outStream << "\nEvent container settings:" << std::endl;
    outStream << "first-container-pos\n" << firstEventContainerPos[0] << " " << firstEventContainerPos[1] << std::endl;
    outStream << "container-size\n" << eventContainerSize[0] << " " << eventContainerSize[1] << std::endl;
    outStream << "container-spacing\n" << eventCeventContainerHorizontalSpacing << " " << eventCeventContainerVerticalSpacing << std::endl;
    outStream << "show-container-bounding-box\n" << showEventContainerBoundingBox << std::endl;

    outStream << "\nFile locations:" << std::endl;
    outStream << "working-directory-path\n" << workingDirectoryPath << std::endl;
    
    outStream << "\nWindow settings:" << std::endl;
    outStream << "window-size\n" << currentWindowWidth << " " << currentWindowHeight << std::endl;
}

void loadOpsEvents(std::ifstream &inStream, std::vector<OpsEvent> &opsEvents){
    std::string settingsTag = "";
    while(std::getline(inStream, settingsTag)){
        std::string params;

        if(settingsTag == "event:"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            OpsEvent newOpsEvent = OpsEvent("", "", Monday, "");
            if(!(iss >> newOpsEvent)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            OpsEvents.push_back(newOpsEvent);
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, unifyFontSize)
        }else if(settingsTag == "schedule background:"){
            std::getline(inStream, params);
            std::istringstream iss(params);
            if(!(iss >> schedulePath)){
                std::cout << "error parsing " << settingsTag << " parameters. Parameters found were: " << params << std::endl;
                continue;
            }
            DEBUG_LOADED_PARAMS_MESSAGE(settingsTag, schedulePath)
        }

        else if(settingsTag == ""){
            continue;
        }else{
#ifdef DEBUG
            std::cout << "Unknown tag/parameter \"" << settingsTag << "\"" << std::endl;
#endif
            continue;
        }
    }
}
void saveOpsEvents(std::ofstream &outStream, const std::vector<OpsEvent> &opsEvents){
    outStream << "Planned events:" << std::endl;
    for(auto &event : OpsEvents)
        outStream << "event:\n" << event << std::endl;
    
    outStream << "Image paths:" << std::endl;
    outStream << "schedule background:\n" << schedulePath << std::endl;
}