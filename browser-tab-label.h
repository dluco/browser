#ifndef _BROWSER_TAB_LABEL_H_
#define _BROWSER_TAB_LABEL_H_

#include "browser-tab.h"
#include <gtk/gtk.h>

#define BROWSER_TYPE_TAB_LABEL (browser_tab_label_get_type())
G_DECLARE_FINAL_TYPE(BrowserTabLabel, browser_tab_label, BROWSER, TAB_LABEL, GtkBox)

BrowserTab *browser_tab_label_get_tab(BrowserTabLabel *tab_label);
GtkWidget *browser_tab_label_new(BrowserTab *tab);

#endif /* _BROWSER_LABEL_H_ */
