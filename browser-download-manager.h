#ifndef _BROWSER_DOWNLOAD_MANAGER_H_
#define _BROWSER_DOWNLOAD_MANAGER_H_

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define BROWSER_TYPE_DOWNLOAD_MANAGER (browser_download_manager_get_type())
G_DECLARE_FINAL_TYPE(BrowserDownloadManager, browser_download_manager, BROWSER, DOWNLOAD_MANAGER, GObject)

void browser_download_manager_add_download(BrowserDownloadManager *manager, WebKitDownload *download);

BrowserDownloadManager *browser_download_manager_get_default(void);
BrowserDownloadManager *browser_download_manager_new(void);

G_END_DECLS

#endif /* _BROWSER_DOWNLOAD_MANAGER_H_ */
