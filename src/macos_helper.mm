// this replicates WA_MacToolAlwaysOnTop attribute
// by accesssing the widget part of the qtquickview
#import <Cocoa/Cocoa.h>
#include "macos_helper.h"

void setupMacOSWindow(void* winId) {
    NSView *nsView = reinterpret_cast<NSView*>(winId);
    NSWindow *nsWindow = [nsView window];

    [nsWindow setLevel:NSFloatingWindowLevel];
    [nsWindow setHidesOnDeactivate:NO];
    [nsWindow setStyleMask:[nsWindow styleMask] | NSWindowStyleMaskNonactivatingPanel];
}
