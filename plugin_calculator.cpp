#include "plugins.h"
#include "tinyexpr.h"

#include <QClipboard>

class CalculatorMatcher : public AbstractMatcher {
public:
  CalculatorMatcher(AppGlobals *) {
    matchingTags << "default"
                 << "calculator"
                 << "number";
  }

  void match(MatchContext *matchContext) override {
    int error = 0;
    if(matchContext->stringToSearch.isEmpty()) {
      return;
    }

    std::string str=matchContext->stringToSearch.toLatin1().toStdString();
    double result = te_interp(str.c_str(), &error);
    if (error == 0) {
      InputItem resultItem;
      resultItem.id = "calculator";
      resultItem.icon = ":/icons/res/calculator.png";
      resultItem.keyword = "keyword";
      resultItem.text = QString::number(result);
      resultItem.smallDescription = "Copy to clipboard";
      resultItem.ownerMatcher = this;
      resultItem.executable = true;
      resultItem.userObject = QString::number(result);
      matchContext->collectedData.append(resultItem);
    }
  }

  void execute(InputItem*inputItem) override {
     qDebug()<<"Copy calculator result";
     QClipboard *clipboard = QGuiApplication::clipboard();
     clipboard->setText(inputItem->userObject.toString());
  }
};

void CalculatorPlugin::init(AppGlobals *appGlobals) {
  matcher = new CalculatorMatcher(appGlobals);
}

QList<AbstractMatcher *> CalculatorPlugin::getMatchers() {
  return QList<AbstractMatcher *>() << matcher;
}
