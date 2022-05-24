#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include "includes.h"

namespace Ui {
class ConfigurationDialog;
}
class FileSystemConfigurationTabHandler;
class GeneralConfigurationTabHandler;
class SnippetsTabHandler;
class BrowsersConfigurationTabHandler;
class WebSearchConfigurationTabHandler;
class WorkflowConfigurationTabHandler;

class ConfigurationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigurationDialog(AppGlobals*appGlobals, Configuration*configuration, QWidget *parent = nullptr);
    ~ConfigurationDialog();
    void modified(QString key);
    bool loading;
    void keyPressEvent(QKeyEvent *e)override;
private:
    QSet<QString> changedConfigurations;
    Ui::ConfigurationDialog *ui;
    Configuration*configuration;
    FileSystemConfigurationTabHandler*fsTabHandler;
    GeneralConfigurationTabHandler*generalTabHandler;
    SnippetsTabHandler*snippetsTabHandler;
    BrowsersConfigurationTabHandler*browserTabHandler;
    WebSearchConfigurationTabHandler*webSearchTabHandler;
    WorkflowConfigurationTabHandler*workflowTabHandler;


    void pressApply();
    void pressOk();
    void pressCancel();
signals:
    void configurationSaved(QStringList changedConfigurations);
};

#endif // CONFIGURATIONDIALOG_H
