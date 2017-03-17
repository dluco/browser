#ifndef _BROWSER_TAB_LABEL_H_
#define _BROWSER_TAB_LABEL_H_

#include <gtk/gtk.h>

#define BROWSER_TYPE_TAB_LABEL (browser_tab_label_get_type())
G_DECLARE_FINAL_TYPE(BrowserTabLabel, browser_tab_label, BROWSER, TAB_LABEL, GtkBox)

GtkWidget *browser_tab_label_new(void);

#endif /* _BROWSER_LABEL_H_ */
