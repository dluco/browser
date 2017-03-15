#ifndef _BROWSER_NOTEBOOK_H
#define _BROWSER_NOTEBOOK_H_

#include <gtk/gtk.h>

#define BROWSER_TYPE_NOTEBOOK (browser_notebook_get_type())
G_DECLARE_FINAL_TYPE(BrowserNotebook, browser_notebook, BROWSER, NOTEBOOK, GtkNotebook)

GtkWidget *browser_notebook_new(void);

#endif /* _BROWSER_NOTEBOOK_H_ */
