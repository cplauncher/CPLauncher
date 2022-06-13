#include "includes.h"
#include "verticallayout.h"
#include <QCache>
#include <QtConcurrent>

const int PANEL_TOTAL_WIDTH=600;
const int PANEL_TOTAL_HEIGHT=50;
const int PANEL_ICON_WIDTH=40;
const int PANEL_ICON_HEIGHT=PANEL_TOTAL_HEIGHT;
QPixmap*defaultIcon=NULL;

QString removeNewLines(QString&str){
    if(str.contains("\n")){
        return str.replace("\n"," ");
    }
    return str;
}

QFileIconProvider iconProvider;
QCache<QString, QIcon> iconsCache(1000);
QMutex getIconMutex;

void insertInIconCache(QString iconPath, QIcon*icon){
  getIconMutex.lock();
  iconsCache.insert(iconPath, icon);
  getIconMutex.unlock();
}
QIcon* getFromIconCache(QString iconPath){
  getIconMutex.lock();
  if(!iconsCache.contains(iconPath)){
      getIconMutex.unlock();
      return NULL;
  }
  QIcon*result=iconsCache[iconPath];
  getIconMutex.unlock();
  return result;
}

QIcon getIcon(QString iconPath) {
  QIcon*cachedIcon=getFromIconCache(iconPath);
  if (cachedIcon!=NULL) {
    return *cachedIcon;
  }

  if (iconPath.startsWith(":")) {
    // it is resource icon
    QIcon *icon = new QIcon(iconPath);
    insertInIconCache(iconPath, icon);
    return *icon;
  }
  QString fileIconPrefix("fileicon://");
  if (iconPath.startsWith(fileIconPrefix)) {
    QString iconPathPart = iconPath.mid(fileIconPrefix.size());
    QIcon *icon = new QIcon(iconProvider.icon(QFileInfo(iconPathPart)));
    insertInIconCache(iconPath, icon);
    return *icon;
  }
  return getIcon(":/icons/res/internet_web_icon.png");
}

QPixmap*getDefaultIcon(){
    if(defaultIcon==NULL){
        QPixmap*pixmap=new QPixmap(PANEL_ICON_WIDTH, PANEL_ICON_HEIGHT);
        pixmap->fill(Qt::transparent);
        defaultIcon=pixmap;
    }
    return defaultIcon;
}

class ItemPanel : public QWidget {
private:
  QLabel *iconLabel;
  QLabel *captionLabel;
  QLabel *descriptionLabel;
  QString iconPath;
  int loadIndex=0;
  int itemIndex;
  std::function<void(ItemPanel*panel)>onClick;

protected:
  void mousePressEvent ( QMouseEvent * event ) override {
    if(event->type()==QMouseEvent::MouseButtonPress) {
        onClick(this);
    }
    QWidget::mousePressEvent(event);
  }

public:
    ItemPanel(QWidget *parent=NULL) : QWidget(parent) {
        itemIndex=-1;
        setSelected(false);
        setAutoFillBackground(true);

        iconLabel = new QLabel(this);

        captionLabel = new QLabel(this);
        descriptionLabel = new QLabel(this);
        captionLabel->setStyleSheet("font-size: 20px; font-weight: bold; color:white");
        descriptionLabel->setStyleSheet("font-size: 12px;color:white");

        setGeometry(0,0, PANEL_TOTAL_WIDTH,PANEL_TOTAL_HEIGHT);
        setBaseSize(PANEL_TOTAL_WIDTH,PANEL_TOTAL_HEIGHT);
        iconLabel->setGeometry(5,0,PANEL_ICON_WIDTH,PANEL_ICON_HEIGHT);
        captionLabel->setGeometry(PANEL_ICON_WIDTH+5,5,PANEL_TOTAL_WIDTH-(PANEL_ICON_WIDTH+5+5),PANEL_TOTAL_HEIGHT/2+5);
        descriptionLabel->setGeometry(PANEL_ICON_WIDTH+5,PANEL_TOTAL_HEIGHT/2+5,PANEL_TOTAL_WIDTH-(PANEL_ICON_WIDTH+5+5),PANEL_TOTAL_HEIGHT/3);
    }

    void setOnClickEvent(std::function<void(ItemPanel*panel)>onClick) {this->onClick=onClick;}
    void setItemIndex(int index) {itemIndex=index;}
    int getItemIndex() {return itemIndex;}
    void setIcon(QString path) { iconPath=path; }
    void setCaption(QString caption) { captionLabel->setText(removeNewLines(caption)); }
    void setDescription(QString description) {descriptionLabel->setText(removeNewLines(description));}

    void setSelected(bool selected) {
        QPalette pal = QPalette();
        if(selected) {
            pal.setColor(QPalette::Window, QColor(80,130,140));
        } else {
            pal.setColor(QPalette::Window, Qt::black);
        }
        setPalette(pal);
    }

    void loadIcon(){
        iconLabel->setPixmap(*getDefaultIcon());
        loadIndex++;
        int _loadIndex=loadIndex;
        QString _iconPath=this->iconPath;
        QtConcurrent::run([this, _loadIndex, _iconPath](){
            if(_loadIndex!=this->loadIndex){
                return;
            }

            QIcon icon=getIcon(_iconPath);
            QMetaObject::invokeMethod(this, [this, icon, _loadIndex] {
                if(_loadIndex==this->loadIndex){
                    iconLabel->setPixmap(icon.pixmap(PANEL_ICON_WIDTH, PANEL_ICON_HEIGHT));
                }
            });
        });
    }
};

InputDialog::InputDialog(AppGlobals *appGlobals, QWidget *parent,
                         QApplication *application)
    : QWidget(parent) {
  Q_UNUSED(application)
  this->appGlobals = appGlobals;
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setAutoFillBackground(true);
  QPalette palette;
  palette.setColor(QPalette::Background, QColor::fromRgb(45, 45, 45));
  setPalette(palette);
  setPalette(palette);

  textField = new CustomLineEdit(this);
  textField->setBaseSize(600, 40);
  textField->setStyleSheet("font-size:30px; background-color:rgb(20,20,20); color:white; border:none");
  textField->setAttribute(Qt::WA_MacShowFocusRect, 0);
  textField->setOnBeforeKeyPressHandler([this](QKeyEvent *event)
                                        { return onBeforeKeyTyped(event); });
  textField->setOnAfterKeyPressHandler([this](QKeyEvent *event)
                                        { return onAfterKeyTyped(event); });
  itemsWidget=new WidgetPanelList<InputItem>(this);
  itemsWidget->setPaddings(0,0,0,0,5);
  itemsWidget->setMaxItemsToShow(MAX_VISIBLE_ITEMS_COUNT);
  itemsWidget->setCreateItemPanelCallback([this](){
      ItemPanel*panel= new ItemPanel(itemsWidget);
      panel->setOnClickEvent([this](ItemPanel*panel){
        itemsWidget->setSelectedIndex(panel->getItemIndex());
      });
      return panel;
  });
  itemsWidget->setInitItemPanelCallback([](QWidget*itemPanel, InputItem&item, int itemIndex) {
      ItemPanel*panel=(ItemPanel*)itemPanel;
      panel->setCaption(item.text);
      panel->setDescription(item.smallDescription);
      panel->setIcon(item.icon);
      panel->setItemIndex(itemIndex);
      panel->loadIcon();
  });

  itemsWidget->setSelectionStatusCallback([](QWidget*itemPanel, InputItem&, bool selected) {
      ItemPanel*panel=(ItemPanel*)itemPanel;
      panel->setSelected(selected);
  });
  itemsWidget->init();

  layout = new VerticalLayout(this);
  layout->setPaddings(5, 5, 5, 5);
  layout->setPaddingBetweenComponents(5);
  layout->addWidget(textField);
  layout->addWidget(itemsWidget);
  layout->pack();

  if (appGlobals->configuration->generalConfiguration.inputDialogPositionX !=-1) {
    move(appGlobals->configuration->generalConfiguration.inputDialogPositionX, appGlobals->configuration->generalConfiguration.inputDialogPositionY);
  }

  fileIconProvider = new QFileIconProvider();
  statisticStorage = new StatisticStorage();
}

InputDialog::~InputDialog() {}

void InputDialog::mousePressEvent(QMouseEvent *event) {
  windowDraggingMousePos = event->pos();
}

void InputDialog::mouseReleaseEvent(QMouseEvent *) {
  appGlobals->configuration->generalConfiguration.inputDialogPositionX =
      geometry().x();
  appGlobals->configuration->generalConfiguration.inputDialogPositionY =
      geometry().y();
  appGlobals->configuration->saveAll(QStringList() << CONF_GENERAL);
}

void InputDialog::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    QPoint diff = event->pos() - windowDraggingMousePos;
    QPoint newpos = this->pos() + diff;
    this->move(newpos);
  }
}

void InputDialog::clearItems() {
  itemsWidget->clear();
}

int InputDialog::getSelectedIndex() {
    return itemsWidget->getSelectedIndex();
}

void InputDialog::setSelectedIndex(int index) {
    if(index>=itemsWidget->count()){
        index=itemsWidget->count()-1;
    }
    if(index<0){
        index=0;
    }
    itemsWidget->setSelectedIndex(index);
    itemsWidget->ensureIndexVisible(index);
}

bool InputDialog::onBeforeKeyTyped(QKeyEvent *event) {
  if (event->key() == Qt::Key_Down) {
    setSelectedIndex(getSelectedIndex() + 1);
    return false;
  }
  if (event->key() == Qt::Key_Escape) {
    hideDialog();
    return false;
  }
  if (event->key() == Qt::Key_Up) {
    setSelectedIndex(getSelectedIndex() - 1);
    return false;
  }
  if (event->key() == Qt::Key_Tab) {
    onExpand();
    return false;
  }
  if (event->key() == Qt::Key_Return||event->key() == Qt::Key_Enter) {
    if (!onAccept()) {
      return false;
    }
  }
  return true;
}

void InputDialog::onExpand() {
  if (getSelectedIndex() != -1) {
    InputItem &item = matchContext.collectedData[getSelectedIndex()];
    currentExpandedItem = item.id;
    textField->setText(item.keyword + " ");
    statisticStorage->addUsage(item.id, matchContext.stringToSearch);
    processMatch();
  }
}
bool InputDialog::onAccept() {
  if (getSelectedIndex() != -1) {
    InputItem item = matchContext.collectedData[getSelectedIndex()];
    statisticStorage->addUsage(item.id, matchContext.stringToSearch);
    hideDialog();
    onAcceptEvent(&item);
    return false;
  }

  return true;
}

void InputDialog::hideDialog() {
  clearItems();
  matchContext.clear();
  hide();
  textField->setText("");
  //Deactivate app only in case if configuration window is not present. Because in other case configuration window also hides.
  if(appGlobals->configurationDialog==NULL) {
    deactivateApp();
  }
  visible = false;
}

bool InputDialog::onAfterKeyTyped(QKeyEvent *event) {
  Q_UNUSED(event)
  if (matchContext.stringToSearch != textField->text()) {
    processMatch();
  }
  return true;
}

void InputDialog::select(AbstractMatcher *matcher,
                         std::function<void(InputItem *)> onAcceptEvent) {
  Q_UNUSED(matcher)
  this->matcher = matcher;
  this->onAcceptEvent = onAcceptEvent;
  activateApp();
  visible = true;
  show();
  //raise();
  activateWindow();//without this, in windows the window not receiving focus
  textField->setFocus();
  processMatch();
}

InputItem InputDialog::selectBlocking(AbstractMatcher*matcher, bool*result) {
    InputItem resultItem;
    bool res=false;
    select(matcher,[&resultItem, &res](InputItem*item) {
        resultItem=*item;
        res=true;
    });

    while(res==false&&visible) {
        QCoreApplication::processEvents();
        QThread::msleep(2);
    }
    QCoreApplication::processEvents();
    *result=res;
    return resultItem;
}

int InputDialog::getIdOrder(const QString &id, const QString &rememberedId) {
  if (id == currentExpandedItem)
    return 20;
  return (id == rememberedId) ? 10 : 0;
}

void InputDialog::sortMatchOptions(QList<InputItem> *matchOptions,
                                   QString userTypedString) {
  QString rememberedId = statisticStorage->getIdByKeyword(userTypedString);
  std::sort(
      matchOptions->begin(), matchOptions->end(),
      [this, &rememberedId](const InputItem &item1, const InputItem &item2) {
        int priority1 = getIdOrder(item1.id, rememberedId);
        int priority2 = getIdOrder(item2.id, rememberedId);
        return priority1 > priority2;
      });
}

void InputDialog::processMatch() {
    clearItems();
    matchContext.clear();
    matchContext.setUserTypedString(textField->text());
    matcher->match(&matchContext);
    QList<InputItem> *items = &matchContext.collectedData;
    sortMatchOptions(items, matchContext.stringToSearch);

    itemsWidget->setItems(*items);

    layout->pack();
    if (items->size() > 0) {
        setSelectedIndex(0);
    }
}