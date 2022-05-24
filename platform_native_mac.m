#import <Cocoa/Cocoa.h>

void activateApp() {
    [NSApp activateIgnoringOtherApps:YES];
}

void deactivateApp() {
    [NSApp hide:NULL];
}

const char* defaultFolderPath() {
    return "/Applications";
}

