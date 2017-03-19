#ifndef _BROWSER_APP_WINDOW_H_
#define _BROWSER_APP_WINDOW_H_

#include "browser-app.h"
#include "browser-tab.h"
#include <gtk/gtk.h>

#define BROWSER_TYPE_WINDOW (browser_window_get_type())
G_DECLARE_FINAL_TYPE(BrowserWindow, browser_window, BROWSER, WINDOW, GtkApplicationWindow)

BrowserTab *browser_window_get_active_tab(BrowserWindow *window);
BrowserWindow *browser_window_new(BrowserApp *app);
void browser_window_open(BrowserWindow *window, GFile *file);

#endif /* _BROWSER_APP_WINDOW_H_ */
