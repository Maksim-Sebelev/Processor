#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h> // for LogTitle only
#include <assert.h>
#include <stddef.h>
#include <time.h>
#include "logger/log.hpp"
#include "lib/lib.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// #define _IMG_BACKGROUND  // if you want to see your image on background

#ifdef _IMG_BACKGROUND
    #define ON_IMG(...) __VA_ARGS__
    #define OFF_IMG(...)

    #define ON_GRADIENT(...) 
    #define OFF_GRADIENT(...)

#else
    #define ON_IMG(...)
    #define OFF_IMG(...) __VA_ARGS__

    // #define _GRADIENT  // if you want to see color gradient on background
 
    #ifdef _GRADIENT
        #define ON_GRADIENT(...) __VA_ARGS__
        #define OFF_GRADIENT(...)
        
    #else
        #define ON_GRADIENT(...)
        #define OFF_GRADIENT(...) __VA_ARGS__

    #endif
    
#endif

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const char* LogDir  = "../log/";
const char* LogName = "log.html";
FILE*       LogFile = nullptr;

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_IMG(
const char* background_image = "../common/src/logger/backgrounds/anime_tyan_2.webp";
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeStyle     (ON_IMG(const char* wayToImage,) size_t nTabBefore);
static void makeTextClass (size_t nTabBefore);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetHtmlColor(LogColor color);

static void fprintfInHtml                   (const char* format, ...);
static void fprintfHtml                     ();
static void fprintfBody                     ();
static void fprintfBodyWithBracket          ();
static void fprintfBodyWithBracketEnd       ();
static void fprintfHead                     ();
static void fprintfHeadEnd                  ();
static void fprintfStyle                    ();
static void fprintfStyleEnd                 ();
static void fprintfNTab                     (size_t nTab);
static void fprintfNS                       ();
static void fprintfNNS                      (size_t nNS);
ON_GRADIENT(
static void fprintfCloseBracket             ();
)
static void fprintfTextSection              ();
static void fprintfTextSectionEnd           ();
static void fprintfH1                       ();
static void fprintfH1End                    ();
static void fprintfH2                       ();
static void fprintfH2End                    ();
static void fprintfP                        ();
static void fprintfPEnd                     ();
static void fprintfPtext                    ();
static void fprintfPtextEnd                 ();
static void fprintfMetaWithArgs             (const char* format, ...);
static void fprintfDivWithArgs              (const char* format, ...);
static void fprintfSpanWithArgs             (const char* format, ...);
static void fprintfSpanEnd                  ();
static void fprintfBodyBefore               ();
static void fprintfBodyBeforeEnd            ();
ON_GRADIENT(
static void fprintfKeyFramesGradient        ();
static void fprintfKeyFramesGradientEnd     ();
)

static void makeBodyBefore    (                         size_t nTabBefore);

ON_IMG(
static void makeBackGroundImage      (const char* wayToImage, size_t nTabBefore);
)
OFF_IMG(
static void makeBackGround           (                        size_t nTabBefore);
)
ON_GRADIENT(
static void makeBackGroundGradient   (                        size_t nTabBefore);
)
static void makeTextSection          (                        size_t nTabBefore);
static void makeTextColor            (                        size_t nTabBefore);
static void makeP                    (                        size_t nTabBefore);
static void makeColor                (                        size_t nTabBefore);
static void makeColorP               (                        size_t nTabBefore);
static void makeH1                   (                        size_t nTabBefore);
static void makeH2                   (                        size_t nTabBefore);
static void LogDate                  (                        size_t nTabBefore);


//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void OpenLog()
{
    assert(LogName && "logName = nullptr");
    assert(LogDir  && "logDir = nullptr");

    static const size_t buffer_size = 256;
    char command[buffer_size] = {};
    snprintf(command, buffer_size, "mkdir -p %s", LogDir);
    int sys_return = system(command);

    char full_log_path[buffer_size] = {};
    snprintf(full_log_path, buffer_size, "%s%s", LogDir, LogName);

    if (sys_return != 0)
        EXIT(EXIT_FAILURE, "failed create log dir.");

    LogFile = fopen(full_log_path, "w");
    if (!LogFile)
        EXIT(EXIT_FAILURE, "failed open log file: '%s'\n", full_log_path);

    fprintfHtml(); fprintfNS();

    makeStyle(ON_IMG(background_image,) 1);

    makeTextClass(0);
    
    LogDate(3);

    LogPrint(White, "\n");

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CloseLog()
{
    assert(LogFile);

    LogDate(3);

    fclose(LogFile);
    LogFile = nullptr;

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogTextColor(LogColor color)
{
    static bool WasUsed = false;
    if (WasUsed)
    {
        LogTextColorEnd();
        WasUsed = false;
    }

    else
    {
        WasUsed = true;
    }

    const char* color_html = GetHtmlColor(color);

    fprintfNTab(3);
        fprintfSpanWithArgs("class=\"color %s\"", color_html); fprintfNS();
        
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogTextColorEnd()
{
    fprintfNTab(3); fprintfSpanEnd(); fprintfNS();

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogAdcPrint(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    fprintfNTab(4); fprintfPtext(); vfprintf(LogFile, format, args); fprintfPtextEnd(); fprintfNS();

    fflush(LogFile);

    va_end(args);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogPrint(LogColor color, const char* format, ...)
{
    assert(LogFile);
    assert(format);

    va_list args;
    va_start(args, format);
    
    const char* color_html = GetHtmlColor(color);
    fprintfNTab(3);
    fprintfSpanWithArgs("class=\"color %s\"", color_html); fprintfNS();

    fprintfNTab(4); fprintfPtext(); vfprintf(LogFile, format, args); fprintfPtextEnd(); fprintfNS();
    
    fflush(LogFile);

    va_end(args);

    fprintfNTab(3);
    fprintfSpanEnd(); fprintfNS();

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogTitle(LogColor color, const char* title)
{
    assert(LogFile);
    assert(title);

    static const size_t ScreenSize = 45; //count in char's, that size is like h2
    size_t title_len = strlen(title) + 2;

    size_t free_place_len = 0;
    if (title_len < ScreenSize - 2) free_place_len = (ScreenSize - (title_len + 2)) / 2;

    if (free_place_len > 0 && (ScreenSize - title_len) % 2 == 0) --free_place_len;

    LogTextColor(color);

    fprintfNTab(3);
    fprintfH2();

    for (size_t i = 0; i < free_place_len; i++)
        fprintfInHtml("=");

    fprintfInHtml(" ");
    fprintfInHtml(title);
    fprintfInHtml(" ");

    for (size_t i = 0; i < free_place_len; i++)
        fprintfInHtml("=");

    
    fprintfH2End(); fprintfNS();

    LogTextColorEnd();

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void LogDate(size_t nTabBefore)
{
    assert(LogFile);

    time_t raw_time;
    struct tm *time_info;
    static const size_t date_len = 64;
    char date[date_len] = {};

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(date, date_len, "%H:%M:%S %Y-%m-%d", time_info);

        fprintfNTab(nTabBefore + 1);
            LogTitle(White, date);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeStyle(ON_IMG(const char* wayToImage,) size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfHead(); fprintfNS();
    
    fprintfNTab(nTabBefore + 1);
        fprintfMetaWithArgs("name=\"viewport\" content=\"width=device-width, initial-scale=1.0\""); fprintfNS();
    
    fprintfNTab(nTabBefore + 1);
        fprintfStyle(); fprintfNS();
    
    ON_IMG
    (
            makeBackGroundImage(wayToImage, nTabBefore + 2);        
    ) 
    OFF_IMG
    (
            makeBackGround          (nTabBefore + 2);
        ON_GRADIENT
        (
            makeBackGroundGradient  (nTabBefore + 2);
        )
    )
            makeTextSection         (nTabBefore + 2);
            makeH1                  (nTabBefore + 2);
            makeH2                  (nTabBefore + 2);
            makeP                   (nTabBefore + 2);
            makeColor               (nTabBefore + 2);
            makeColorP              (nTabBefore + 2);
            makeTextColor           (nTabBefore + 2);

            makeBodyBefore(nTabBefore + 2);

    fprintfNTab(nTabBefore + 1);
        fprintfStyleEnd(); fprintfNS();
    
    fprintfNTab(nTabBefore);
    fprintfHeadEnd(); fprintfNS();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeTextClass(size_t nTabBefore)
{

    fprintfNTab(nTabBefore);
    fprintfNS(); fprintfBody(); fprintfNS();
    
    fprintfNTab(nTabBefore + 1);
        fprintfDivWithArgs("class=\"text-section\""); fprintfNS();
    }
    
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeBodyBefore(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfBodyBefore(); fprintfNS();

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("content: \"\";\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("position: fixed;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("top: 0;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("left: 0;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("right: 0;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("bottom: 0;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("background: rgba(0, 0, 0, 0.5);\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("z-index: -1;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("pointer-events: none;\n");


    fprintfNTab(nTabBefore);
    fprintfBodyBeforeEnd(); fprintfNS();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

OFF_IMG(
static void makeBackGround(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfBodyWithBracket(); fprintfNS();
    
        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("margin: 0;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("padding: 0;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("height: 100vh;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("position: relative;\n");

        fprintfNTab(nTabBefore + 1);
        
        ON_GRADIENT
        (
            fprintfInHtml("background: linear-gradient(135deg, #000000, #000000, #6600ff, #6600ff);\n");
            // fprintfInHtml("background: linear-gradient(135deg, #ffffff, #ffffff, #ffffff, #0644f6, #0644f6, #0644f6, #ea2b0b, #ea2b0b, #ea2b0b);\n");
        )
        OFF_GRADIENT
        (
            fprintfInHtml("background-color: #000000;\n");
        )

        ON_GRADIENT(
        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("animation: gradient 15s ease infinite;\n");
        )
    
        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-attachment: fixed;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-size: 400%% 400%%;\n");


        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("font-family: Arial, sans-serif;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("color: white;\n");



    fprintfNTab(nTabBefore);
    fprintfBodyWithBracketEnd(); fprintfNNS(2);
}
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_GRADIENT(
static void makeBackGroundGradient(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfKeyFramesGradient(); fprintfNS();

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("0%% {\n");
            
            fprintfNTab(nTabBefore + 2);
                fprintfInHtml("background-position: 0%% 50%%;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfCloseBracket(); fprintfNS();

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("50%% {\n");

            fprintfNTab(nTabBefore + 2);
                fprintfInHtml("background-position: 100%% 50%%;\n");

        fprintfNTab(nTabBefore + 1);
                fprintfCloseBracket(); fprintfNS();

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("100%% {\n");
        
            fprintfNTab(nTabBefore + 2);
                fprintfInHtml("background-position: 0%% 50%%;\n");

        fprintfNTab(nTabBefore + 1);
                fprintfCloseBracket(); fprintfNS();

    fprintfNTab(nTabBefore);
    fprintfKeyFramesGradientEnd(); fprintfNNS(2);
}
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
ON_IMG(
static void makeBackGroundImage(const char* wayToImage, size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfBodyWithBracket(); fprintfNS();
    
        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("margin: 0;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("padding: 0;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("height: 100vh;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-image: url('%s');\n", wayToImage);

        fprintfNTab(nTabBefore + 1);
            OFF_IMG(
            fprintfInHtml("background-size: 400%% 400%%;\n");
            )
            ON_IMG(
            fprintfInHtml("background-size: cover;\n");
            )
        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-position: center;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-repeat: no-repeat;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("background-attachment: fixed;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("font-family: Arial, sans-serif;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("color: white;\n");

    fprintfNTab(nTabBefore);
    fprintfBodyWithBracketEnd(); fprintfNNS(2);
}
)
//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeTextSection(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfTextSection(); fprintfNS();

        // fprintfNTab(nTabBefore + 1);
            // fprintfInHtml("background-color: rgba(0, 0, 0, 0.4);\n"); // затемнение секции текста

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("padding: 30px;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("border-radius: 10px;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("max-width: 800px;\n");

        fprintfNTab(nTabBefore + 1);
            fprintfInHtml("margin: 20px auto;\n");

        // fprintfNTab(nTabBefore + 1);
            // fprintfInHtml("backdrop-filter: blur(5px);\n");

    fprintfNTab(nTabBefore);
            fprintfTextSectionEnd();  fprintfNNS(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeH1(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfH1(); fprintfNS();
    
    fprintfNTab(nTabBefore + 1); 
        fprintfInHtml("text-align: center;\n");
    fprintfNTab(nTabBefore + 1); 
        fprintfInHtml("margin-bottom: 5px;\n");

    fprintfNTab(nTabBefore);
    fprintfH1End(); fprintfNNS(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeH2(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfInHtml("h2 {"); fprintfNS();
    
    // fprintfNTab(nTabBefore + 1); 
        // fprintfInHtml("text-align: center;\n");
    fprintfNTab(nTabBefore + 1); 
        fprintfInHtml("margin-bottom: 5px;\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml("}"); fprintfNNS(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeP(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfP(); fprintfNS();

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("line-height: 0.5;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("text-align: justify;\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("margin: 10px 0;\n");

    fprintfNTab(nTabBefore);
    fprintfPEnd(); fprintfNNS(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeTextColor(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfInHtml(".red_text { color: #ff4444; }\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml(".green_text { color: #0bf80b; }\n");
    
    fprintfNTab(nTabBefore);
    fprintfInHtml(".pink_text { color: #f605c7; }\n");
    
    fprintfNTab(nTabBefore);
    fprintfInHtml(".yellow_text { color: #ecc40b; }\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml(".blue_text { color: #0c89e8; }\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml(".black_text { color: #000000; }");

    fprintfNS();
    fprintfNS();
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeColor(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfInHtml(".color {\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("padding: 0px 10px;\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("margin: 1px 0;\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("line-height: 0.5;\n");

    // fprintfNTab(nTabBefore + 1);
    // fprintfInHtml("border-radius: 3px;\n");
    
    // fprintfNTab(nTabBefore + 1);
    // fprintfInHtml("border-left: 3px solid currentColor;\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml("}"); fprintfNNS(2);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void makeColorP(size_t nTabBefore)
{
    fprintfNTab(nTabBefore);
    fprintfInHtml(".color p {\n");

    fprintfNTab(nTabBefore  + 1);
    fprintfInHtml("white-space: pre;\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("padding: 0;\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("margin: 0;\n");

    fprintfNTab(nTabBefore + 1);
    fprintfInHtml("line-height: 1.1;\n");
    
    // fprintfNTab(nTabBefore + 1);
    // fprintfInHtml("border-left: 3px solid currentColor;\n");

    fprintfNTab(nTabBefore);
    fprintfInHtml("}"); fprintfNNS(2);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfInHtml(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    va_end(args);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfHtml()
{
    fprintfInHtml("<html>");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfBody()
{
    fprintfInHtml("<body>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfBodyWithBracket()
{
    fprintfInHtml("body {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfBodyWithBracketEnd()
{
    fprintfInHtml("}");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfBodyBefore()
{
    fprintfInHtml("body::before {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfBodyBeforeEnd()
{
    fprintfInHtml("}");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfHead()
{
    fprintfInHtml("<head>");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfHeadEnd()
{
    fprintfInHtml("</head>");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfStyle()
{
    fprintfInHtml("<style>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfStyleEnd()
{
    fprintfInHtml("</style>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfMetaWithArgs(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    fprintfInHtml("<meta ");

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    fprintfInHtml(">");

    va_end(args);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfDivWithArgs(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    fprintfInHtml("<div ");

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    fprintfInHtml(">");

    va_end(args);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfSpanWithArgs(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    fprintfInHtml("<span ");

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    fprintfInHtml(">");

    va_end(args);
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfSpanEnd()
{
    fprintfInHtml("</span>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_GRADIENT(
static void fprintfKeyFramesGradient()
{
    fprintfInHtml("@keyframes gradient {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfKeyFramesGradientEnd()
{
    fprintfInHtml("}");
}
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfNTab(size_t nTab)
{
    for (size_t i = 0; i < nTab; i++) fprintfInHtml("\t");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfNS()
{
    fprintfInHtml("\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfNNS(size_t nNS)
{
    for (size_t i = 0; i < nNS; i++) fprintfInHtml("\n");
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

ON_GRADIENT(
static void fprintfCloseBracket()
{
    fprintfInHtml(" }");
    return;
}
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfTextSection()
{
    fprintfInHtml(".text-section {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfTextSectionEnd()
{
    fprintfInHtml("}");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfH1()
{
    fprintfInHtml("h1 {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfH1End()
{
    fprintfInHtml("}");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfH2()
{
    fprintfInHtml("<h2>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfH2End()
{
    fprintfInHtml("</h2>");
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfP()
{
    fprintfInHtml("p {");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfPEnd()
{
    fprintfInHtml("}");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfPtext()
{
    fprintfInHtml("<p>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfPtextEnd()
{
    fprintfInHtml("</p>");
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetHtmlColor(LogColor color)
{
    switch (color)
    {
        case LogColor::Red:    return "red_text";
        case LogColor::Green:  return "green_text";
        case LogColor::Pink:   return "pink_text";
        case LogColor::Yellow: return "yellow_text";
        case LogColor::Black:  return "black_text";
        case LogColor::Blue:   return "blue_text";
        case LogColor::White:  return "";
        default:               assert(0 && "undef color in log");
    }

    return "";
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------