#include <QDesktopServices>
#include "plugins.h"


class WebSearchMatcher:public AbstractMatcher {
  public:
    WebSearchMatcher(AppGlobals*appGlobals) {
        this->appGlobals=appGlobals;
        matchingTags<<"default"<<"websearch";
    }

    QString extractInputArgument(QString keyword, QString userInput) {
        if (userInput.length() <= keyword.length()) {
            return "";
        }
        return userInput.mid(keyword.length()).trimmed();
    }

    InputItem createInputItem(int id, WebSearch*webSearchItem, QString userInput) {
        QString inputArgument = extractInputArgument(webSearchItem->keyword, userInput);

        QString title = webSearchItem->title;
        title = title.replace("${input}", inputArgument != "" ? inputArgument : "...");
        QString url;
        if (inputArgument != "") {
            url = webSearchItem->url;
            url = url.replace("${input}", QUrl::toPercentEncoding(inputArgument));
        }

        InputItem item;
        item.id=QString("websearch_") + webSearchItem->keyword+"_"+ QString::number(id);
        item.icon=":/icons/res/internet_web_icon.png";;
        item.keyword = webSearchItem->keyword;
        item.executable=inputArgument != "";
        item.text=title;
        item.smallDescription=url;
        item.ownerMatcher=this;
        item.userObject=url;
        return item;
    }

    virtual void match(MatchContext*matchContext) {
        if (matchContext->stringToSearch.isEmpty())
            return;
        WebSearchConfiguration*conf=&appGlobals->configuration->webSearchConfiguration;
        int count = 0;
        for (int i=0;i<conf->searchCollection.size();i++) {
            WebSearch*webSearch= &conf->searchCollection[i];
            if (defaultMatch(matchContext, webSearch->keyword, true)) {
                matchContext->collectedData.append(createInputItem(count++, webSearch, matchContext->stringToSearch));
            }
        }
    }

    virtual void execute(InputItem*inputItem) {
        QDesktopServices::openUrl(QUrl(inputItem->userObject.toString()));
    }
};

void WebSearchPlugin::initDefaultConfiguration(AbstractConfig*abstractConfig) {
    WebSearchConfiguration*wsConf=(WebSearchConfiguration*)abstractConfig;
    wsConf->searchCollection.append(WebSearch("https://www.google.com/search?q=${input}","google","Search in Google: {input}"));
    wsConf->searchCollection.append(WebSearch("https://ru.wikipedia.org/w/index.php?title=Special:Search&search=${input}","wiki","Search in Wikipedia: {input}"));
}

void WebSearchPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    matcher=new WebSearchMatcher(appGlobals);
};

QList<AbstractMatcher*> WebSearchPlugin::getMatchers() {
    QList<AbstractMatcher*>matchers;
    matchers<<matcher;
    return matchers;
};
