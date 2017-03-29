#ifndef _BROWSER_APP_WINDOW_H_
#define _BROWSER_APP_WINDOW_H_

#include "browser-app.h"
#include "browser-tab.h"
#include <gtk/gtk.h>

#define BROWSER_TYPE_WINDOW (browser_window_get_type())
G_DECLARE_FINAL_TYPE(BrowserWindow, browser_window, BROWSER, WINDOW, GtkApplicationWindow)

BrowserTab *browser_window_get_active_tab(BrowserWindow *window);
BrowserWindow *browser_window_new(BrowserApp *app);
BrowserTab *browser_window_create_tab_from_uri(BrowserWindow *window, const gchar *uri, gint position, gboolean jump_to);
void browser_window_open(BrowserWindow *window, const gchar *uri);
void browser_window_close_tab(BrowserWindow *window, BrowserTab *tab);

void browser_window_fullscreen(BrowserWindow *window);
void browser_window_unfullscreen(BrowserWindow *window);
gboolean browser_window_is_fullscreen(BrowserWindow *window);

#endif /* _BROWSER_APP_WINDOW_H_ */
