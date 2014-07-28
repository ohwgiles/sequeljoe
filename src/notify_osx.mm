#include "notify.h"
#import <Foundation/NSUserNotification.h>
#import <Foundation/NSString.h>

@interface PopupEnabler : NSObject
{
}
@end

@implementation PopupEnabler
- (bool) userNotificationCenter:(NSUserNotificationCenter *)center
  shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}
@end

Notifier* notify;

Notifier::Notifier() {
    id popupEnabler = [[PopupEnabler alloc] init];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:popupEnabler];
}

void Notifier::send(const char *title, const char *msg) {
    NSUserNotification *userNotification = [[[NSUserNotification alloc] init] autorelease];
    userNotification.title = [NSString stringWithUTF8String:title];
    userNotification.informativeText = [NSString stringWithUTF8String:msg];
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:userNotification];
}
