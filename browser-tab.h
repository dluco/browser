#ifndef _BROWSER_TAB_H_
#define _BROWSER_TAB_H_

#include <gtk/gtk.h>

#define BROWSER_TYPE_TAB (browser_tab_get_type())
G_DECLARE_FINAL_TYPE(BrowserTab, browser_tab, BROWSER, TAB, GtkBox)

gchar *browser_tab_get_title(BrowserTab *tab);
GtkWidget *browser_tab_new(void);

#endif /* _BROWSER_TAB_H_ */
