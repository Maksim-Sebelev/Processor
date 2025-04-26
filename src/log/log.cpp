#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "log/log.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define _IMG_BACKGROUND  // if you want to see your image on background

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


ON_IMG(
const char* background_image = "../include/log/backgrounds/anime_tyan.webp";
// const char* background_image = "../include/log/backgrounds/735.jpg";
)

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

FILE*       LogFile = nullptr;
const char* LogName = "Log/log.html";

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
// static void fprintfH2                       ();
// static void fprintfH2End                    ();
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
// static void makeH2                   (                        size_t nTabBefore);


//=== background color ===//
const char* black_background     = "body { background-color: #121212; }                                                                          \n";
const char* gradient1_background = "body { background: linear-gradient(to right,  #0f2027, #203a43, #2c5364);}                               \n";
const char* gradient2_background = "body { background: linear-gradient(to bottom, #ffffff,#ffffff, #1948ff, #1948ff, #f92b03, #f92b03);} \n";

//=== text color ===//

const char* red_text_html_name    = "red_text";
const char* green_text_html_name  = "green_text";
const char* pink_text_html_name   = "pink_text";
const char* yellow_text_html_name = "yellow_text";
const char* blue_text_html_name   = "blue_text";
const char* black_text_html_name  = "black_text";

const char* red_text_rgb    = " { color: #ff4444; } \n";
const char* green_text_rgb  = " { color: #0bf80b; } \n";
const char* pink_text_rgb   = " { color: #f605c7; } \n";
const char* yellow_text_rgb = " { color: #ecc40b; } \n";
const char* blue_text_rgb   = " { color: #0c89e8; } \n"; 
const char* black_text_rgb  = " { color: #000000; } \n";

//=== ===//

void OpenLog()
{
    assert(LogName && "logName = nullptr");

    int sys_return = system("mkdir -p Log/");
    assert(sys_return == 0 && "failed make log dir");

    LogFile = fopen(LogName, "w");
    assert(LogFile && "failed open log file");

    fprintfHtml(); fprintfNS();

    makeStyle(ON_IMG(background_image,) 1);

    makeTextClass(0);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CloseLog()
{
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
        fprintfInHtml("background: rgba(0, 0, 0, 0.6);\n");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("z-index: -1;");

    fprintfNTab(nTabBefore + 1);
        fprintfInHtml("pointer-events: none;\n");


    fprintfNTab(nTabBefore);
    fprintfBodyBeforeEnd();
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
            fprintfInHtml("background: linear-gradient(135deg, #000000, #000000, #6600ff, #0de612, #0de612);\n");
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

// static void makeH2(size_t nTabBefore)
// {
//     fprintfNTab(nTabBefore);
//     fprintfH2(); fprintfNS();
    
//     fprintfNTab(nTabBefore + 1); 
//         fprintfInHtml("text-align: center;\n");
//     fprintfNTab(nTabBefore + 1); 
//         fprintfInHtml("margin-bottom: 5px;\n");

//     fprintfNTab(nTabBefore);
//     fprintfH2End(); fprintfNNS(2);
// }

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
    fprintfInHtml(".%s %s", red_text_html_name    , red_text_rgb    );

    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", green_text_html_name  , green_text_rgb  );
    
    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", yellow_text_html_name , yellow_text_rgb );
    
    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", blue_text_html_name   , blue_text_rgb   );

    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", black_text_html_name  , black_text_rgb  );

    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", pink_text_html_name   , pink_text_rgb   );

    fprintfNTab(nTabBefore);
    fprintfInHtml(".%s %s", red_text_html_name    , red_text_rgb    );

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
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static void fprintfInHtml(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    va_end(args);
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

// static void fprintfH2()
// {
//     fprintfInHtml("h2 {");
// }

// //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// static void fprintfH2End()
// {
//     fprintfInHtml("}");
// }

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
        case LogColor::Red:    return red_text_html_name;
        case LogColor::Green:  return green_text_html_name;
        case LogColor::Pink:   return pink_text_html_name;
        case LogColor::Yellow: return yellow_text_html_name;
        case LogColor::Black:  return black_text_html_name;
        case LogColor::Blue:   return blue_text_html_name;
        case LogColor::White:  return "";
        default:               assert(0 && "undef color in log");
    }

    return "";
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

