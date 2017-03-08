#ifndef _BROWSER_APP_WINDOW_H_
#define _BROWSER_APP_WINDOW_H_

#include "browser-app.h"
#include <gtk/gtk.h>

#define BROWSER_WINDOW_TYPE (browser_window_get_type())
G_DECLARE_FINAL_TYPE(BrowserAppWindow, browser_app_window, BROWSER, APP_WINDOW, GtkApplicationWindow)

BrowserAppWindow *browser_app_window_new(BrowserApp *app);
void browser_app_window_open(BrowserAppWindow *window, GFile *file);

#endif /* _BROWSER_APP_WINDOW_H_ */
