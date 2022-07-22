#ifndef WFNODEEDIT_DIALOG_H
#define WFNODEEDIT_DIALOG_H
#include "includes.h"

namespace Ui {
class WfNodeEditDialog;
}

class WfNodeEditDialog : public QDialog {
    Q_OBJECT
    AppGlobals*appGlobals;
public:
    explicit WfNodeEditDialog(QWidget *parent, AppGlobals*appGlobals);
    ~WfNodeEditDialog();
    bool editHotkeyNode(WFNode*node);
    bool editKeywordNode(WFNode*node);
    bool editScriptingNode(WFNode*node);
    bool editClipboardCopyNode(WFNode*node);
    bool editExecAppNode(WFNode*node);
    bool editNotificationNode(WFNode*node);
    bool editPlaySoundNode(WFNode*node);
    bool editExternalTriggerNode(WFNode*node);
    bool editExternalScriptNode(WFNode*node);
    bool editSelectorNode(WFNode*node);
    bool editOpenUrlNode(WFNode*node);
    bool editLuaScriptNode(WFNode*node);
    bool editVars(QList<Variable>*variables);
    bool editVariable(Variable*variable);

private:
    void showPanel(QWidget*widget);
    Ui::WfNodeEditDialog *ui;
};

#endif // WFNODEEDIT_DIALOG_H
