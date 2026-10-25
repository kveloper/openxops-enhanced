#ifndef MENU_LICENSE_H
#define MENU_LICENSE_H
#include <string>

inline std::string BuildMenuLicenseText(const char* const* lines, int count, int first, int visible)
{
    std::string text;
    if(!lines || count<=0 || visible<=0) return text;
    if(first<0) first=0;
    if(first>=count) first=count-1;
    const int remaining=count-first;
    const int length=visible<remaining ? visible : remaining;
    for(int i=0;i<length;i++){
        if(lines[first+i]) text+=lines[first+i];
        text+='\n';
    }
    return text;
}
#endif
