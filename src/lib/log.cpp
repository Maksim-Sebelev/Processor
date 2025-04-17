#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "lib/log.hpp"

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

FILE* LogFile = nullptr;
const char* LogName = "Log/log.html";

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

static const char* GetHtmlColor(LogColor color);

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//=== background color ===//
const char* black_background     = "body { background-color: #121212; }                                           \n";
const char* gradient1_background = "body { background: linear-gradient(to right,  #0f2027, #203a43, #2c5364);} \n";
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

    fprintf(LogFile, "<html>                      \n"
        "\t<head>                                 \n"
        "\t<style>                                \n"
        "\t\t%s                                   \n" // background
        "\tpre { color: #ffffff; }              \n"
        ".%s%s                                    \n" // red
        ".%s%s                                    \n" // green
        ".%s%s                                    \n" // pink
        ".%s%s                                    \n" // yellow
        ".%s%s                                    \n" // black
        ".%s%s                                    \n" // blue
        "\t</style>                               \n"
        "\t</head>                                \n"
        "\t<body>                                 \n"
        "\t<div class=\"jumbotron text-center\">  \n"
        "\t</div>                                 \n"
        "\t<pre>                                  \n",
    black_background      ,
    red_text_html_name    , red_text_rgb    ,
    green_text_html_name  , green_text_rgb  ,
    pink_text_html_name   , pink_text_rgb   ,
    yellow_text_html_name , yellow_text_rgb ,
    black_text_html_name  , black_text_rgb  ,
    blue_text_html_name   , blue_text_rgb
);

    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CloseLog()
{
    fprintf(LogFile,    "\t\t</pre>     \n"
        "\t</body       \n"
        "</html>");
    
    return;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void LogPrint(LogColor color, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    const char* color_html = GetHtmlColor(color);
    fprintf(LogFile, "<span class=\"%s\">", color_html);

    vfprintf(LogFile, format, args);
    fflush(LogFile);

    va_end(args);

    fprintf(LogFile, "</span>");

    return;
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

