#include <ApplicationServices/ApplicationServices.h> 
//#include <iostream>

int main() {
    CGEventRef event = CGEventCreate(NULL);
    CGPoint center = CGEventGetLocation(event);
    CFRelease(event);
    center.x = 0;
    center.y = 0;

    CGDisplayCount displayCount;
    CGGetActiveDisplayList(0, NULL, &displayCount);

    //std::cout<<"displaycount is: "<<displayCount<<"\n";

    CGDirectDisplayID *displays = new CGDirectDisplayID[displayCount];
    CGGetActiveDisplayList(displayCount, displays, NULL);

    for (int i = 0; i < displayCount; i++) {
        CGRect bounds = CGDisplayBounds(displays[i]);
        center.x += bounds.size.width / 2;
        center.y += bounds.size.height / 2;
    }

    center.x /= displayCount;
    center.y /= displayCount;

    CGWarpMouseCursorPosition(center);
    CGAssociateMouseAndMouseCursorPosition(true);

    delete[] displays;

    return 0;
}

