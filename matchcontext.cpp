#include "includes.h"

MatchContext::MatchContext() {}
QRegExp spaceRegex("\\s+");
void MatchContext::setUserTypedString(QString string) {
    stringToSearch = string.trimmed();
    stringParts.clear();
    if (!stringToSearch.isEmpty()) {
        QStringList parts = stringToSearch.split(spaceRegex);
        for (int i = 0; i < parts.size(); i++) {
            parts[i] = parts[i].toLower();
        }
        stringParts.append(parts);
    }
}

void MatchContext::clear() {
    collectedData.clear();
    stringToSearch="";
    stringParts.clear();
    canceled=false;
    done=false;
}
