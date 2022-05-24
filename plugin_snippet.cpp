#include <QClipboard>
#include <QTime>
#include <QThread>
#include "plugins.h"
#include "regexreplacer.h"
#include "placeholderexpander.h"
QString SNIPPET_ICON=":/icons/res/snippets.png";
QString SNIPPET_COLLECTION_ICON=":/icons/res/snippetsCollection.png";

class SubSnippetsCollectionMatcher:public MatcherWithActivationKeyword {
    SnippetsCollection*snippetsCollection;
    SnippetsMatcher*snippetsMatcher;
    int*counter;
public:
    SubSnippetsCollectionMatcher(AppGlobals*appGlobals,SnippetsCollection*snippetsCollection, int*counter, SnippetsMatcher*snippetsMatcher):MatcherWithActivationKeyword(appGlobals) {
        this->snippetsCollection=snippetsCollection;
        this->counter=counter;
        this->snippetsMatcher=snippetsMatcher;
        setActivationKeyword(snippetsCollection->keyword);
    }

    void matchAfterActivation(MatchContext*matchContext) override;
    void execute(InputItem*) override {}
    InputItem createActivationInputItem() override;
};

class SnippetsMatcher:public MatcherWithActivationKeyword {
  public:
    SnippetsMatcher(AppGlobals*appGlobals) : MatcherWithActivationKeyword(appGlobals) {
        matchingTags<<"default"<<"snippets";
    }

    InputItem createActivationInputItem() override {
        InputItem item;
        item.id="snippets_activation";
        item.keyword=activationKeyword;
        item.text=activationKeyword;
        item.smallDescription= "Search in snippets";
        item.icon= SNIPPET_COLLECTION_ICON;
        item.ownerMatcher= this;
        item.executable=false;
        return item;
    }

    void matchAfterActivation(MatchContext*matchContext) override {
        SnippetsConfiguration*conf=&appGlobals->configuration->snippersConfiguration;
        QList<SnippetsCollection>&snippetsCollection =  conf->snippetsCollections;
        int counter = 0;
        for (SnippetsCollection&collection : snippetsCollection) {
            QString stringToSearch=matchContext->stringToSearch;
            if(!collection.keyword.isEmpty()) {
                SubSnippetsCollectionMatcher subMatcher(appGlobals, &collection,&counter,this);
                subMatcher.match(matchContext);
            } else {
                for (Snippet&snippet : collection.snippets) {
                    QString itemKeyword= snippet.name.toLower();
                    if (defaultMatch(matchContext, itemKeyword, false)) {
                        matchContext->collectedData.append(createSnippetInputItem(snippet));
                        counter++;
                        if (counter > 50) {
                            return;
                        }
                    }
                }
            }
        }
    }

    InputItem createSnippetInputItem(Snippet&snippet) {
        QString description = "Snippet [" + snippet.snippet + "]";
        InputItem snippetItem;
        snippetItem.id="snippet_item_" + snippet.name;
        snippetItem.keyword=activationKeyword+" "+snippet.name;
        snippetItem.text=snippet.name;
        snippetItem.smallDescription=description;
        snippetItem.icon=SNIPPET_ICON;
        snippetItem.ownerMatcher=this;
        snippetItem.executable=true;
        snippetItem.userObject=snippet.snippet;
        return snippetItem;
    }

    void execute(InputItem*inputItem) override {
       qDebug()<<"Execute snippet:"<<inputItem->userObject.toString();
       if (!inputItem->id.startsWith("snippet_item_")) {
           qDebug()<<"[ERROR] SnippetsMatcher cannot execute item [" + inputItem->id + "]";
           return;
       }

       PlaceholderExpander placeholderExpander(appGlobals);
       QString snippetText = placeholderExpander.expand(inputItem->userObject.toString(), NULL);
       QClipboard *clipboard = QGuiApplication::clipboard();
       clipboard->setText(snippetText);
       appGlobals->trayManager->showMessage("Snippets","Snippet copied to clipboard");
    }
};

void SubSnippetsCollectionMatcher::matchAfterActivation(MatchContext*matchContext) {
   for (int i=0;i<snippetsCollection->snippets.size();i++) {
       Snippet&snippet = snippetsCollection->snippets[i];
       QString itemKeyword= snippet.name.toLower();
       if (defaultMatch(matchContext, itemKeyword, false)) {
           QString description = "Snippet [" + snippet.snippet + "]";
           InputItem snippetItem=snippetsMatcher->createSnippetInputItem(snippet);
           matchContext->collectedData.append(snippetItem);
           (*counter)++;
           if (*counter > 50) {
               return;
           }
       }
   }
}

InputItem SubSnippetsCollectionMatcher::createActivationInputItem() {
    InputItem item;
    item.id="snippets_activation";
    item.keyword=appGlobals->configuration->snippersConfiguration.activationKeyword+" "+snippetsCollection->keyword;
    item.text=snippetsCollection->name;
    item.smallDescription= "Search in snippet collection "+snippetsCollection->name;
    item.icon= SNIPPET_COLLECTION_ICON;
    item.ownerMatcher= snippetsMatcher;
    item.executable=false;
    return item;
}

void SnippetsPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    snippetsMatcher=new SnippetsMatcher(appGlobals);
    snippetsMatcher->setActivationKeyword(appGlobals->configuration->snippersConfiguration.activationKeyword);
}

QList<AbstractMatcher*> SnippetsPlugin::getMatchers() {
    QList<AbstractMatcher*>result;
    result<<snippetsMatcher;
    return result;
}

void SnippetsPlugin::refresh() {
    snippetsMatcher->setActivationKeyword(appGlobals->configuration->snippersConfiguration.activationKeyword);
}

