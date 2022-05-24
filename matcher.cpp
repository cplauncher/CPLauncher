#include "includes.h"

bool AbstractMatcher::defaultMatch(MatchContext*matchContext, QString&itemKeyword, bool allowQueryArgument) {
    if (allowQueryArgument) {
        return itemKeyword.startsWith(matchContext->stringToSearch) || matchContext->stringToSearch.startsWith(itemKeyword);
    } else {
        return defaultMatch(matchContext->stringParts, itemKeyword);
    }
}

bool AbstractMatcher::defaultMatch(QStringList&userTypedStringParts, QString&itemKeyword) {
    int index = 0;
    for (int i=0;i<userTypedStringParts.size();i++) {
        QString userTypedStringPart = userTypedStringParts[i];
        int partPosition = itemKeyword.indexOf(userTypedStringPart, index);
        if (partPosition == -1) {
            return false;
        }
        index = partPosition + userTypedStringPart.length();
    }
    return true;
}

void CompoundMatcher::match(MatchContext*matchContext) {
    for (int i=0;i<matchers.size();i++) {
        AbstractMatcher*matcher = matchers[i];
        matcher->match(matchContext);
    }
}

void MatcherWithActivationKeyword::match(MatchContext*matchContext) {
    if (activationKeyword.isEmpty()) {
        matchAfterActivation(matchContext);
    }
    else if (matchContext->stringToSearch.isEmpty()==false && matchContext->stringParts[0]==activationKeyword) {
        passToChildMatcher(matchContext);
    }
    else if (activationKeyword.startsWith(matchContext->stringToSearch) && matchContext->stringParts.size() == 1) {
        InputItem item = createActivationInputItem();
        matchContext->collectedData.append(item);
    }
}

void MatcherWithActivationKeyword::passToChildMatcher(MatchContext*matchContext) {
    QList<QString> oldUserTypedStringParts = matchContext->stringParts;
    QList<QString> partsWithoutActivation = oldUserTypedStringParts;
    QString oldStringToSearch=matchContext->stringToSearch;
    QString removedPart=partsWithoutActivation.takeFirst();
    matchContext->stringToSearch=matchContext->stringToSearch.mid(removedPart.length()).trimmed();
    if (partsWithoutActivation.isEmpty()) {
        partsWithoutActivation.append("");
    }
    matchContext->stringParts=partsWithoutActivation;
    matchAfterActivation(matchContext);
    matchContext->stringParts=oldUserTypedStringParts;
    matchContext->stringToSearch=oldStringToSearch;
}

void TypeTextInputMatcher::match(MatchContext*matchContext) {
    QString icon = ":/icons/res/snippets.png";
    InputItem item;
    item.id="stringsInput";
    item.keyword= matchContext->stringToSearch;
    item.text= matchContext->stringToSearch;
    item.smallDescription= description;
    item.icon=icon;
    item.ownerMatcher=this;
    item.executable=true;
    matchContext->collectedData.append(item);
}
