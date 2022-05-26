#include <QFileDialog>
#include <QGraphicsRectItem>
#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "editsnippetscollectiondialog.h"
#include "editwebsearchdialog.h"
#include "editsnippetdialog.h"
#include "tablehelper.h"
#include "wfconfigurationtabhandler.h"


class GeneralConfigurationTabHandler {
    GeneralConfiguration*configuration;
    ConfigurationDialog*dialog;
    Ui::ConfigurationDialog *ui;
  public:
    GeneralConfigurationTabHandler(ConfigurationDialog*dialog, GeneralConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
    }

    void init() {
        ui->whiteTrayIconCheckbox->setChecked(configuration->isWhiteTrayIcon);
        ui->generalLauncherToggleHotkeyEditor->setHotkeyId("_toggleLauncherHotkey");
        ui->generalLauncherToggleHotkeyEditor->setKeySequence(QKeySequence(configuration->toggleLauncherHotkey));
        QObject::connect(ui->generalLauncherToggleHotkeyEditor, &CustomHotkeyEditor::editingFinished, dialog, [this]() {
            QKeySequence keySequence=ui->generalLauncherToggleHotkeyEditor->keySequence();
            QString newHotkey;
            if(keySequence.count()>0) {
                newHotkey=keySequence.toString();
            }

            configuration->toggleLauncherHotkey=newHotkey;
            dialog->modified(CONF_GENERAL);
        });
        QObject::connect(ui->whiteTrayIconCheckbox, &QCheckBox::stateChanged,dialog, [this]() {
            configuration->isWhiteTrayIcon= ui->whiteTrayIconCheckbox->isChecked();
            dialog->modified(CONF_GENERAL);
        });
    }
};

class FSListItem:public QListWidgetItem {
  public:
    FolderConfig *folderConfig;
    FSListItem(QListWidget*widget, FolderConfig *folderConfig):QListWidgetItem(widget) {
        this->folderConfig=folderConfig;
    }
};

class FileSystemConfigurationTabHandler {
    FileSystemConfiguration*configuration;
    ConfigurationDialog*dialog;
    Ui::ConfigurationDialog *ui;
  public:
    FileSystemConfigurationTabHandler(ConfigurationDialog*dialog, FileSystemConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
    }

    void init() {
        ui->fsDirectoriesList->setSelectionMode(QListWidget::SingleSelection);
        disableFolderConfigUi();
        QObject::connect(ui->fsDirLineEdit, &QLineEdit::textChanged, dialog, [this](QString text) {
            FSListItem*listItem = getSelectedFolderConfig();
            if(listItem!=NULL) {
                listItem->folderConfig->directory=text;
                listItem->setText(text);
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsSelectDirButton, &QPushButton::pressed, dialog, [this]() {
            FSListItem*listItem = getSelectedFolderConfig();
            if(listItem!=NULL) {
                QString dir = QFileDialog::getExistingDirectory(dialog, "Choose Directory", listItem->folderConfig->directory,QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
                if(dir=="") {
                    return;
                }
                listItem->folderConfig->directory=dir;
                listItem->setText(dir);
                ui->fsDirLineEdit->setText(dir);
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsDepthSpinBox, &QSpinBox::textChanged, dialog, [this](QString value) {
            FSListItem*listItem = getSelectedFolderConfig();
            if(listItem!=NULL) {
                listItem->folderConfig->depth=value.toInt();
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsIncludeExecutablesCB, &QCheckBox::stateChanged, dialog, [this](bool value) {
            FSListItem*listItem = getSelectedFolderConfig();
            if(listItem!=NULL) {
                listItem->folderConfig->executableIncluded=value;
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsAllowedGlobsTextField, &QPlainTextEdit::textChanged, dialog, [this]() {
            FSListItem*listItem = getSelectedFolderConfig();
            if(listItem!=NULL) {
                listItem->folderConfig->allowedGlobs=ui->fsAllowedGlobsTextField->document()->toPlainText();
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsExcludedGlobsTextField, &QPlainTextEdit::textChanged, dialog, [this]() {
            FSListItem*listItem=getSelectedFolderConfig();
            if(listItem!=NULL) {
                listItem->folderConfig->excludedGlobs=ui->fsExcludedGlobsTextField->document()->toPlainText();
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });

        QObject::connect(ui->fsAddDirButton, &QPushButton::clicked, dialog, [this]() {
            FolderConfig folderConfig;
            folderConfig.directory = defaultFolderPath();
            folderConfig.depth=3;
            folderConfig.executableIncluded=true;

            configuration->folderConfigs.append(folderConfig);
            FSListItem*newItem = addNewFolderConfig(&configuration->folderConfigs.last());
            newItem->setSelected(true);
            onDirectoryItemSelected();
            dialog->modified(CONF_FILE_SYSTEM);
        });
        QObject::connect(ui->fsRemoveDirButton,&QPushButton::clicked, dialog, [this]() {
            FSListItem*listItem=getSelectedFolderConfig();
            if(listItem!=NULL) {
                disableFolderConfigUi();
                int rowIndex=ui->fsDirectoriesList->row(listItem);
                ui->fsDirectoriesList->takeItem(rowIndex);
                configuration->folderConfigs.removeAt(rowIndex);
                dialog->modified(CONF_FILE_SYSTEM);
            }
        });
        QObject::connect(ui->fsDirectoriesList, &QListWidget::itemClicked, dialog, [this]() {
            dialog->loading=true;
            onDirectoryItemSelected();
            dialog->loading=false;
        });

        dialog->loading=true;
        for(int i=0; i<configuration->folderConfigs.size(); i++) {
            addNewFolderConfig(&configuration->folderConfigs[i]);
        }
        ui->fsDirectoriesList->setCurrentRow(0);
        onDirectoryItemSelected();
        dialog->loading=false;
    }

    FSListItem*getSelectedFolderConfig() {
        if(ui->fsDirectoriesList->selectedItems().size()>0) {
            FSListItem*listItem = (FSListItem*)ui->fsDirectoriesList->selectedItems()[0];
            return listItem;
        } else {
            return NULL;
        }
    }

    FSListItem* addNewFolderConfig(FolderConfig*folderConfig) {
        FSListItem*item=new FSListItem(ui->fsDirectoriesList,folderConfig);
        item->setText(folderConfig->directory);
        return item;
    }

    void onDirectoryItemSelected() {
        FSListItem*listItem=getSelectedFolderConfig();
        if(listItem==NULL) {
            disableFolderConfigUi();
        } else {
            folderConfigToForm(listItem->folderConfig);
        }
    }

    void disableFolderConfigUi() {
        ui->fsDirLineEdit->setText("");
        ui->fsDirLineEdit->setEnabled(false);
        ui->fsSelectDirButton->setEnabled(false);
        ui->fsDepthSpinBox->setValue(0);
        ui->fsDepthSpinBox->setEnabled(false);
        ui->fsIncludeExecutablesCB->setChecked(false);
        ui->fsIncludeExecutablesCB->setEnabled(false);
        ui->fsAllowedGlobsTextField->document()->setPlainText("");
        ui->fsAllowedGlobsTextField->setEnabled(false);
        ui->fsExcludedGlobsTextField->document()->setPlainText("");
        ui->fsExcludedGlobsTextField->setEnabled(false);
    }

    void folderConfigToForm(FolderConfig*folderConfig) {
        ui->fsDirLineEdit->setText(folderConfig->directory);
        ui->fsDirLineEdit->setEnabled(true);
        ui->fsSelectDirButton->setEnabled(true);
        ui->fsDepthSpinBox->setValue(folderConfig->depth);
        ui->fsDepthSpinBox->setEnabled(true);
        ui->fsIncludeExecutablesCB->setChecked(folderConfig->executableIncluded);
        ui->fsIncludeExecutablesCB->setEnabled(true);
        ui->fsAllowedGlobsTextField->document()->setPlainText(folderConfig->allowedGlobs);
        ui->fsAllowedGlobsTextField->setEnabled(true);
        ui->fsExcludedGlobsTextField->document()->setPlainText(folderConfig->excludedGlobs);
        ui->fsExcludedGlobsTextField->setEnabled(true);
    }
};

class SnippetListItem:public QListWidgetItem {
  public:
    SnippetsCollection*snippetsCollection;
    SnippetListItem(QListWidget*widget, SnippetsCollection*snippetsCollection) : QListWidgetItem(widget) {
        this->snippetsCollection = snippetsCollection;
    }
};

QString snippetToString(Snippet*snippet, int column) {
    if(column==0) {
        return snippet->name;
    }else{
        return snippet->snippet;
    }
}

class SnippetsTabHandler {
    SnippetsConfiguration*configuration;
    ConfigurationDialog*dialog;
    Ui::ConfigurationDialog*ui;
    TableHelper<Snippet*>*snippetTableHelper;
  public:
    SnippetsTabHandler(ConfigurationDialog*dialog, SnippetsConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
        snippetTableHelper=new TableHelper<Snippet*>(ui->snipTable,2, snippetToString);
    }

    ~SnippetsTabHandler() {

    }

    void init() {
        QStringList snippetsTableHeader;
        snippetsTableHeader<<"Name"<<"Text";
        ui->snipTable->setHorizontalHeaderLabels(snippetsTableHeader);
        ui->snipTable->horizontalHeader()->setStretchLastSection(true);
        ui->snipTable->setEditTriggers(QTableWidget::NoEditTriggers);
        ui->snipTable->setSelectionMode(QTableWidget::SingleSelection);
        ui->snipTable->setSelectionBehavior(QTableWidget::SelectRows);
        ui->snipTable->verticalHeader()->setVisible(false);
        ui->snipTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        QObject::connect(ui->snipKeywordInput, &QLineEdit::textChanged, dialog, [this](QString text) {
            configuration->activationKeyword=text;
            dialog->modified(CONF_SNIPPETS);
        });
        QObject::connect(ui->snipCollectionList, &QListWidget::doubleClicked, dialog, [this]() {
            SnippetListItem*listItem=getSelectedSnippetCollection();
            if(listItem!=NULL) {
                SnippetsCollection*snippetCollection = listItem->snippetsCollection;
                EditSnippetsCollectionDialog editSnippetsCollectionDialog(dialog);
                if(editSnippetsCollectionDialog.editSnippetsCollection(snippetCollection->name, snippetCollection->keyword)) {
                    snippetCollection->name=editSnippetsCollectionDialog.getName();
                    snippetCollection->keyword=editSnippetsCollectionDialog.getKeyword();
                    listItem->setText(editSnippetsCollectionDialog.getName());
                    dialog->modified(CONF_SNIPPETS);
                }
            }
        });
        QObject::connect(ui->snipCollectionList, &QListWidget::itemSelectionChanged, dialog, [this]() {
            showSelectedSnippetCollection();
        });

        QObject::connect(ui->snipAddCollectionButton, &QPushButton::clicked, dialog, [this]() {
            EditSnippetsCollectionDialog editSnippetsCollectionDialog(dialog);
            if(editSnippetsCollectionDialog.editSnippetsCollection("Collection", "")) {
                SnippetsCollection snippetCollection(editSnippetsCollectionDialog.getName());
                snippetCollection.keyword=editSnippetsCollectionDialog.getKeyword();
                configuration->snippetsCollections.append(snippetCollection);
                SnippetsCollection*collectionPointer=&configuration->snippetsCollections.last();
                SnippetListItem*newItem = new SnippetListItem(ui->snipCollectionList, collectionPointer);
                newItem->setText(collectionPointer->name);
                newItem->setSelected(true);
                updateEditableStatus();
                dialog->modified(CONF_SNIPPETS);
            }
        });
        QObject::connect(ui->snipRemoveCollectionButton,&QPushButton::clicked, dialog, [this]() {
            SnippetListItem*listItem=getSelectedSnippetCollection();
            if(listItem!=NULL) {
                int rowIndex=ui->snipCollectionList->row(listItem);
                ui->snipCollectionList->takeItem(rowIndex);
                configuration->snippetsCollections.removeAt(rowIndex);
                updateEditableStatus();
                dialog->modified(CONF_SNIPPETS);
            }
        });

        QObject::connect(ui->snipAddButton,&QPushButton::clicked, dialog, [this]() {
            SnippetListItem*collectionItem=getSelectedSnippetCollection();
            if(collectionItem!=NULL) {
                EditSnippetDialog snippetDialog(dialog);
                snippetDialog.setWindowTitle("Add Snippet");
                if(snippetDialog.editSnippet("", "")) {
                    collectionItem->snippetsCollection->snippets.append(Snippet(snippetDialog.getName(),snippetDialog.getSnippet()));
                    Snippet*snippet=&collectionItem->snippetsCollection->snippets.last();
                    snippetTableHelper->add(snippet);
                    snippetTableHelper->setSelectedRow(snippetTableHelper->rowCount()-1);
                    updateEditableStatus();
                    dialog->modified(CONF_SNIPPETS);
                }
            }
        });
        QObject::connect(ui->snipRemoveButton,&QPushButton::clicked, dialog, [this]() {
            SnippetListItem*collectionItem=getSelectedSnippetCollection();
            if(collectionItem!=NULL && snippetTableHelper->getSelectedRow()!=-1) {
                int row = snippetTableHelper->getSelectedRow();
                snippetTableHelper->remove(row);
                collectionItem->snippetsCollection->snippets.takeAt(row);
                updateEditableStatus();
                dialog->modified(CONF_SNIPPETS);
            }
        });
        QObject::connect(ui->snipTable,&QTableWidget::itemSelectionChanged, dialog, [this]() {
            updateEditableStatus();
        });
        QObject::connect(ui->snipTable,&QTableWidget::doubleClicked, dialog, [this]() {
            SnippetListItem*collectionItem=getSelectedSnippetCollection();
            if(collectionItem!=NULL && snippetTableHelper->getSelectedRow()!=-1) {
                int row = snippetTableHelper->getSelectedRow();
                Snippet*snippet=&collectionItem->snippetsCollection->snippets[row];
                EditSnippetDialog snippetDialog(dialog);
                if(snippetDialog.editSnippet(snippet->name, snippet->snippet)) {
                    snippet->name=snippetDialog.getName();
                    snippet->snippet=snippetDialog.getSnippet();
                    snippetTableHelper->refreshRow(row);
                    dialog->modified(CONF_SNIPPETS);
                }
            }
        });

        dialog->loading=true;
        ui->snipKeywordInput->setText(configuration->activationKeyword);
        ui->snipCollectionList->clear();
        for(int i=0;i<configuration->snippetsCollections.size();i++) {
            SnippetsCollection*collectionPointer=&configuration->snippetsCollections[i];
            SnippetListItem*newItem = new SnippetListItem(ui->snipCollectionList, collectionPointer);
            newItem->setText(collectionPointer->name);
        }
        if(ui->snipCollectionList->count()>0) {
            ui->snipCollectionList->setCurrentRow(0);
            showSelectedSnippetCollection();
        }
        updateEditableStatus();
        dialog->loading=false;
    }

    void showSelectedSnippetCollection() {
        SnippetListItem*listItem=getSelectedSnippetCollection();
        if(listItem!=NULL) {
            fillSnippetsTable(listItem->snippetsCollection);
            updateEditableStatus();
        }
    }

    void updateEditableStatus() {
        SnippetListItem*snippetsCollection=getSelectedSnippetCollection();
        if(snippetsCollection==NULL) {
            ui->snipSnippetEditor->setEnabled(false);
            ui->snipRemoveCollectionButton->setEnabled(false);
            snippetTableHelper->clear();
            return;
        }
        ui->snipSnippetEditor->setEnabled(true);
        ui->snipRemoveCollectionButton->setEnabled(true);
        ui->snipRemoveButton->setEnabled(snippetTableHelper->getSelectedRow()!=-1);
    }

    void fillSnippetsTable(SnippetsCollection*collection) {
        snippetTableHelper->clear();
        for(int i=0;i<collection->snippets.size();i++) {
            Snippet*snippet=&collection->snippets[i];
            snippetTableHelper->add(snippet);
        }
    }

    SnippetListItem*getSelectedSnippetCollection() {
        if(ui->snipCollectionList->selectedItems().size()>0) {
            SnippetListItem*listItem = (SnippetListItem*)ui->snipCollectionList->selectedItems()[0];
            return listItem;
        } else {
            return NULL;
        }
    }
};

class BrowsersConfigurationTabHandler {
    BrowserConfiguration*configuration;
    ConfigurationDialog*dialog;
    Ui::ConfigurationDialog *ui;
    BrowserBookmarksExtractor browsersBookmarkExtractor;
  public:
    BrowsersConfigurationTabHandler(ConfigurationDialog*dialog, BrowserConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
    }

    void init() {
        QListWidgetItem*chromeListItem = new QListWidgetItem("Google Chrome", ui->browsersChooserList);
        if (browsersBookmarkExtractor.isChromeSupported()) {
            chromeListItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
            chromeListItem->setCheckState(configuration->chromeEnabled?Qt::Checked:Qt::Unchecked);
        } else {
            chromeListItem->setFlags(Qt::ItemIsUserCheckable);
            chromeListItem->setCheckState(Qt::Unchecked);
        }

        QListWidgetItem*firefoxListItem = new QListWidgetItem("Firefox", ui->browsersChooserList);
        if (browsersBookmarkExtractor.isFirefoxSupported()) {
            firefoxListItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
            firefoxListItem->setCheckState(configuration->firefoxEnabled?Qt::Checked:Qt::Unchecked);
        } else {
            firefoxListItem->setFlags(Qt::ItemIsUserCheckable);
            firefoxListItem->setCheckState(Qt::Unchecked);
        }
        QObject::connect(ui->browsersChooserList, &QListWidget::itemChanged, dialog, [firefoxListItem, this](QListWidgetItem*item) {
            bool checked=item->checkState()==Qt::Checked;
            if(item==firefoxListItem) {
                configuration->firefoxEnabled=checked;
            } else {
                configuration->chromeEnabled=checked;
            }

            dialog->modified(CONF_BROWSER);
        });
    }
};

QString webSearchToString(WebSearch*webSearch, int column) {
    if(column==0) {
        return webSearch->keyword;
    }else if(column==1) {
        return webSearch->title;
    }else if(column==2) {
        return webSearch->url;
    }else{
        return "unknown column";
    }
}

class WebSearchConfigurationTabHandler {
    WebSearchConfiguration*configuration;
    ConfigurationDialog*dialog;
    Ui::ConfigurationDialog *ui;
    BrowserBookmarksExtractor browsersBookmarkExtractor;
    TableHelper<WebSearch*>*tableHelper;
  public:
    WebSearchConfigurationTabHandler(ConfigurationDialog*dialog, WebSearchConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
        tableHelper=new TableHelper<WebSearch*>(ui->wsTable,3, webSearchToString);
    }

    void init() {
        QStringList webSearchTableHeader;
        webSearchTableHeader<<"Keyword"<<"Title"<<"Url";
        ui->wsTable->setHorizontalHeaderLabels(webSearchTableHeader);
        ui->wsTable->horizontalHeader()->setStretchLastSection(true);
        ui->wsTable->setEditTriggers(QTableWidget::NoEditTriggers);
        ui->wsTable->setSelectionMode(QTableWidget::SingleSelection);
        ui->wsTable->setSelectionBehavior(QTableWidget::SelectRows);
        ui->wsTable->verticalHeader()->setVisible(false);
        ui->wsTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        QObject::connect(ui->wsAddButton,&QPushButton::clicked, dialog, [this]() {
            EditWebSearchDialog editDialog(dialog);
            editDialog.setWindowTitle("Add Web Search");
            QVariantMapWithCheck result;
            if(editDialog.editWebSearch(result, "", "","")) {
                configuration->searchCollection.append(WebSearch(result.getString("url"), result.getString("keyword"),result.getString("title")));
                WebSearch*webSearch=&configuration->searchCollection.last();
                tableHelper->add(webSearch);
                tableHelper->setSelectedRow(tableHelper->rowCount()-1);
                updateEditableStatus();
                dialog->modified(CONF_WEB_SEARCH);
            }
        });
        QObject::connect(ui->wsRemoveButton,&QPushButton::clicked, dialog, [this]() {
            int selectedRow=tableHelper->getSelectedRow();
            if(selectedRow!=-1) {
                tableHelper->remove(selectedRow);
                configuration->searchCollection.removeAt(selectedRow);
                updateEditableStatus();
                dialog->modified(CONF_WEB_SEARCH);
            }
        });
        QObject::connect(ui->wsTable,&QTableWidget::itemSelectionChanged, dialog, [this]() {
            updateEditableStatus();
        });
        QObject::connect(ui->wsTable,&QTableWidget::doubleClicked, dialog, [this]() {
            int selectedRow=tableHelper->getSelectedRow();
            WebSearch*ws=tableHelper->getSelectedObject();
            if(ws!=NULL) {
                EditWebSearchDialog editDialog(dialog);
                QVariantMapWithCheck result;
                if(editDialog.editWebSearch(result, ws->url, ws->keyword,ws->title)) {
                    ws->url=result.getString("url");
                    ws->keyword=result.getString("keyword");
                    ws->title=result.getString("title");
                    tableHelper->refreshRow(selectedRow);
                    updateEditableStatus();
                    dialog->modified(CONF_WEB_SEARCH);
                }
            }
        });

        dialog->loading=true;
        tableHelper->clear();
        for(int i=0; i<configuration->searchCollection.size(); i++) {
            WebSearch*webSearch=&configuration->searchCollection[i];
            tableHelper->add(webSearch);
        }
        updateEditableStatus();
        dialog->loading=false;
    }

    void updateEditableStatus() {
        ui->wsRemoveButton->setEnabled(tableHelper->getSelectedRow()!=-1);
    }
};

ConfigurationDialog::ConfigurationDialog(AppGlobals*appGlobals, Configuration*configuration, QWidget *parent) : QDialog(parent), ui(new Ui::ConfigurationDialog) {
    ui->setupUi(this);
    //setWindowFlags(Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    this->configuration = configuration;

    connect(ui->applyButton, &QPushButton::pressed, this, &ConfigurationDialog::pressApply);
    connect(ui->cancelButton, &QPushButton::pressed, this, &ConfigurationDialog::pressCancel);
    connect(ui->okButton, &QPushButton::pressed, this, &ConfigurationDialog::pressOk);

    generalTabHandler=new GeneralConfigurationTabHandler(this, &configuration->generalConfiguration, ui);
    generalTabHandler->init();
    fsTabHandler=new FileSystemConfigurationTabHandler(this, &configuration->fileSystemConfiguration, ui);
    fsTabHandler->init();
    snippetsTabHandler=new SnippetsTabHandler(this, &configuration->snippersConfiguration, ui);
    snippetsTabHandler->init();
    browserTabHandler=new BrowsersConfigurationTabHandler(this, &configuration->browserConfiguration, ui);
    browserTabHandler->init();
    webSearchTabHandler=new WebSearchConfigurationTabHandler(this, &configuration->webSearchConfiguration, ui);
    webSearchTabHandler->init();
    workflowTabHandler=new WorkflowConfigurationTabHandler(appGlobals, this, &configuration->workflowConfiguration, ui);
    workflowTabHandler->init();
}

ConfigurationDialog::~ConfigurationDialog() {
    delete generalTabHandler;
    delete fsTabHandler;
    delete snippetsTabHandler;
    delete browserTabHandler;
    delete workflowTabHandler;

    delete ui;
}

void ConfigurationDialog::keyPressEvent(QKeyEvent *e) {
    if(e->key() != Qt::Key_Escape) {
        QDialog::keyPressEvent(e);
    }
}

void ConfigurationDialog::modified(QString key) {
    if(loading==false) {
        ui->applyButton->setEnabled(true);
        changedConfigurations<<key;
    }
}

void ConfigurationDialog::pressApply() {
    QStringList changedConfigurationList;
    foreach(QString str, changedConfigurations) {
        changedConfigurationList<<str;
    }
    configuration->saveAll(changedConfigurationList);
    emit configurationSaved(changedConfigurationList);
    changedConfigurations.clear();
    ui->applyButton->setEnabled(false);
}

void ConfigurationDialog::pressOk() {
    pressApply();
    close();
}

void ConfigurationDialog::pressCancel() {
    close();
}
