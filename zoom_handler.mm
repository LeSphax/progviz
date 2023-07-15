#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

extern "C" {
    typedef void (*scrollCallback_t)(float);

    void registerScroll(scrollCallback_t callback) {
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskScrollWheel handler:^NSEvent*(NSEvent* event) {
            if ([event deltaY] != 0) {
                callback([event deltaY]);
            }
            return event;
        }];
    }
}
