#ifndef _BROWSER_TAB_H_
#define _BROWSER_TAB_H_

#include "browser-web-view.h"
#include <gtk/gtk.h>

#define BROWSER_TYPE_TAB (browser_tab_get_type())
G_DECLARE_FINAL_TYPE(BrowserTab, browser_tab, BROWSER, TAB, GtkBox)

BrowserWebView *browser_tab_get_web_view(BrowserTab *tab);
void browser_tab_load_uri(BrowserTab *tab, const gchar *uri);
gchar *browser_tab_get_uri(BrowserTab *tab);
gchar *browser_tab_get_title(BrowserTab *tab);
GtkWidget *browser_tab_new(void);

#endif /* _BROWSER_TAB_H_ */
