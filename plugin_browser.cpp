#include <QDesktopServices>
#include "plugins.h"

class BrowserMatcher:public AbstractMatcher {
    QList<Bookmark>bookmarks;
  public:
    BrowserMatcher(AppGlobals*appGlobals) {
        this->appGlobals=appGlobals;
        matchingTags<<"default"<<"websearch";
    }

    void setBookmarks(QList<Bookmark>bookmarks) {
        this->bookmarks=bookmarks;
    }

    InputItem createInputItem(int id, Bookmark*bookmark) {
        InputItem item;
        item.id="browser_" + bookmark->getGuid() + QString::number(id);
        if(bookmark->getBrowser()=="chrome") {
            item.icon=":/icons/res/browser_google-chrome.png";
        }else if(bookmark->getBrowser()=="ff") {
            item.icon=":/icons/res/browser_firefox.png";
        }else{
            item.icon=":/icons/res/internet_web_icon.png";
        }
        item.keyword = bookmark->getBookmarkText();
        item.executable=true;
        item.text=bookmark->getBookmarkText();
        item.smallDescription=bookmark->getUrl();
        item.ownerMatcher=this;
        item.userObject=bookmark->getUrl();
        return item;
    }

    void match(MatchContext*matchContext)override {
        if (matchContext->stringToSearch.isEmpty())
            return;
        int count = 0;
        for (int i=0;i<bookmarks.count();i++) {
            Bookmark*bookmark= &bookmarks[i];
            if (defaultMatch(matchContext, bookmark->getBookmarkTextLower(), false)) {
                matchContext->collectedData.append(createInputItem(count++, bookmark));
            }
        }
    }

    void execute(InputItem*inputItem)override {
        QUrl url=QUrl(inputItem->userObject.toString());
        QDesktopServices::openUrl(url);
    }
};

void BrowserPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    matcher=new BrowserMatcher(appGlobals);
    refresh();
}

QList<AbstractMatcher*> BrowserPlugin::getMatchers() {
    return QList<AbstractMatcher*>()<<matcher;
};

void BrowserPlugin::refresh() {
    BrowserConfiguration&conf=appGlobals->configuration->browserConfiguration;
    QList<Bookmark>bookmarks;
    if(conf.chromeEnabled&&bookmarksExtractor.isChromeSupported()) {
        bookmarks.append(bookmarksExtractor.extractChromeBookmarks());
    }
    if(conf.firefoxEnabled&&bookmarksExtractor.isFirefoxSupported()) {
        bookmarks.append(bookmarksExtractor.extractFirefoxBookmarks());
    }
    matcher->setBookmarks(bookmarks);
};
