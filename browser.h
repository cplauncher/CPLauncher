#ifndef BROWSER_H
#define BROWSER_H

#include <QFile>

class Bookmark {
  private:
    QString url;
    QString bookmarkText;
    QString bookmarkTextLower;
    QString guid;
    QString browser;
  public:
    Bookmark() {}
    Bookmark(QString url,QString bookmarkText,QString guid,QString browser) {
        this->url=url;
        this->bookmarkText=bookmarkText;
        this->bookmarkTextLower=bookmarkText.toLower();
        this->guid=guid;
        this->browser=browser;
    }
    QString&getUrl() {
        return url;
    }
    QString&getBookmarkText() {
        return bookmarkText;
    }
    QString&getBookmarkTextLower() {
        return bookmarkTextLower;
    }
    QString&getGuid() {
        return guid;
    }
    QString&getBrowser() {
        return browser;
    }
};

class BrowserBookmarksExtractor {
    QString chromeConfigFile;
    QString firefoxConfigFile;
    QString getChromeConfigFile();
    QString getFirefoxConfigFile();
  public:
    bool isChromeSupported();
    bool isFirefoxSupported();
    QList<Bookmark>extractChromeBookmarks();
    QList<Bookmark>extractFirefoxBookmarks();
};
#endif // BROWSER_H
