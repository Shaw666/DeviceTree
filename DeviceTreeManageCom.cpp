#include "DeviceTreeManageCom.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include "CommonHelper.h"
#include <QHeaderView>
#include "libthings.h"
#include "ClientApplication.h"
#include <QMessageBox>
#include "PwdInputDialog.h"
#include "DevieCapability.h"
#include "DeviceOutput.h"
#include "CreateGroupDlg.h"
#include <QDir>
#include <qscrollbar.h>

#pragma execution_character_set("utf-8")

#define TREE_ITEM_HEIGHT 25

DeviceTreeManageCom::DeviceTreeManageCom(bool hideChannelWhenOffline, QWidget* parent)
	: BaseWidget(parent)
{
	setMinimumWidth(250);
	m_bHideChannelWhenOffline = hideChannelWhenOffline;
	m_nDeviceGroupVersion = -1;
	m_pRightClickItem = nullptr;
	m_pMatrixDeviceTreeItemRoot = nullptr;
	m_bNameSortASC = true;
	QTimer* p2MinTimer = new QTimer(this);
	initView();
	initRightClickMenu();
	//connect(jyApp->getThings(), SIGNAL(sglThingsEvent(int)), this, SLOT(sltThingsEvent(int)));
	connect(jyApp->getThings(), SIGNAL(sglThingsNodeEvent(QString, int)), this, SLOT(sltThingsNodeEvent(QString, int)));
	connect(this, SIGNAL(sglDeviceGroupChanged()), this, SLOT(sltDeviceGroupChanged()));
	connect(p2MinTimer, SIGNAL(timeout()), this, SLOT(slt2MinTimeout()));
	p2MinTimer->start(120000);
}

DeviceTreeManageCom::~DeviceTreeManageCom()
{
	//delete m_mapThingsObj;
}

void DeviceTreeManageCom::updateMatrixDev(QList<CWEcoderInfo*>* listEncoder)
{
	if (listEncoder == nullptr){
		return;
	}
	if (m_pMatrixDeviceTreeItemRoot == nullptr) {
		m_pMatrixDeviceTreeItemRoot = new DeviceTreeItem(tr("Matrix encoder device"), m_pDeviceTreeModel->topLevelItem());
		m_pMatrixDeviceTreeItemRoot->setRow(m_pDeviceTreeModel->topLevelItem()->childCount());
		m_pMatrixDeviceTreeItemRoot->setType(NODE_GROUP);
		m_pDeviceTreeModel->topLevelItem()->appendChild(m_pMatrixDeviceTreeItemRoot);
	}
	int devCount = listEncoder->size();
	for (int i=0;i<devCount;i++){
		addNewDevItem(m_pMatrixDeviceTreeItemRoot, listEncoder->at(i));
	}
	m_pDeviceTreeModel->layoutChanged();
	m_pDeviceTreeView->expand(m_pDeviceTreeModel->index(1, 0));
}

void DeviceTreeManageCom::initView()
{

	CommonHelper::loadStyleSheet(this, "../Common/Skin/common/DeviceTreeManageCom.qss");
	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setMargin(10);
	pMainLayout->setSpacing(10);
	QHBoxLayout* pTopNavLayout = new QHBoxLayout(this);
	pTopNavLayout->setSpacing(20);
	pTopNavLayout->setMargin(5);
	QString style = "QToolButton{border:0px solid #CCCCCC;border-radius:5px;background-color:rgb(72,82,102);color:rgb(255,255,255);font-size:12px;text-align:center;}"
		"QToolButton:hover{border-radius:5px;background-color:rgb(0,96,159);}"
		"QToolButton:checked{border-radius:5px;background-color:rgb(0,96,159);}";
	m_pDeviceNavBtn = new JYToolButton(tr("Device"), QSize(64, 24), style, "", this);
	//m_pFavoritesNavBtn = new JYToolButton(tr("Favorites"), QSize(64, 24), style, "", this);
	m_pHistoryNavBtn = new JYToolButton(tr("History"), QSize(64, 24), style, "", this);
	m_pDeviceNavBtn->setChecked(true);
	pTopNavLayout->addWidget(m_pDeviceNavBtn, 1);
	//pTopNavLayout->addWidget(m_pFavoritesNavBtn, 1);
	pTopNavLayout->addWidget(m_pHistoryNavBtn, 1);
	pTopNavLayout->addStretch();
	pMainLayout->addLayout(pTopNavLayout);
	connect(m_pDeviceNavBtn, SIGNAL(sglClicked()), this, SLOT(sltNavClicked()));
	//connect(m_pFavoritesNavBtn, SIGNAL(clicked()), this, SLOT(sltNavClicked()));
	connect(m_pHistoryNavBtn, SIGNAL(sglClicked()), this, SLOT(sltNavClicked()));

	//m_pContentStackedLayout = new QStackedLayout(this);
	//m_pContentStackedLayout->setMargin(0);
	//m_pContentStackedLayout->setSpacing(0);
	//pMainLayout->addLayout(m_pContentStackedLayout);
	QWidget* pDeviceContentWidget = new QWidget(this);
	pMainLayout->addWidget(pDeviceContentWidget);
	QVBoxLayout* pDeviceContentLayout = new QVBoxLayout(pDeviceContentWidget);
	pDeviceContentLayout->setMargin(0);
	pDeviceContentLayout->setSpacing(10);
	//m_pContentStackedLayout->addWidget(pDeviceContentWidget);
	QWidget* ptreeNavWidget = new QWidget(pDeviceContentWidget);
	ptreeNavWidget->setStyleSheet("QWidget{background-color: rgb(255, 255, 255); border-radius:5px;}");
	QHBoxLayout* treeNavLayout = new QHBoxLayout(ptreeNavWidget);
	treeNavLayout->setMargin(0);
	treeNavLayout->setSpacing(0);
	m_pEdtKeyWord = new QLineEdit(ptreeNavWidget);
	m_pEdtKeyWord->setObjectName("m_pEdtKeyWord");
	//m_pEdtKeyWord->setMinimumWidth(TREE_WIDGET_WIDTH - 30);
	m_pEdtKeyWord->setPlaceholderText(tr("Please input keyword"));
	m_pEdtKeyWord->setFixedHeight(30);
	QPushButton* pBtnSearch = new QPushButton(ptreeNavWidget);
	pBtnSearch->setToolTip(tr("Search"));
	pBtnSearch->setStyleSheet("QPushButton{background:transparent;border-radius:5px;}"
		"QPushButton{image: url(../Common/Skin/video/search_n.png);}"
		"QPushButton:hover{image: url(../Common/Skin/video/search_h.png);}"
	);
	connect(pBtnSearch, SIGNAL(clicked()), this, SLOT(sltSearVideo()));
	connect(m_pEdtKeyWord, SIGNAL(returnPressed()), this, SLOT(sltSearVideo()));
	treeNavLayout->addWidget(m_pEdtKeyWord, 1);
	treeNavLayout->addWidget(pBtnSearch);
	pDeviceContentLayout->addWidget(ptreeNavWidget);
	QHBoxLayout* pTopNavLayout3 = new QHBoxLayout(pDeviceContentWidget);
	pTopNavLayout3->setMargin(0);
	pTopNavLayout3->setSpacing(10);
	m_pNameSort = new QToolButton(pDeviceContentWidget);
	//m_pNameDesc = new QToolButton(pDeviceContentWidget);
	connect(m_pNameSort, SIGNAL(clicked()), this, SLOT(sltNameSortBtnClicked()));
	//connect(m_pNameDesc, SIGNAL(clicked()), this, SLOT(sltGroupDeviceNumberSortBtnClicked()));
	m_pNameSort->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	//m_pNameDesc->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	m_pNameSort->setIcon(QIcon("../Common/Skin/common/up_n.png"));
	//m_pNameDesc->setIcon(QIcon("../Common/Skin/common/down_n.png"));
	m_pNameSort->setText(tr("Name"));
	//m_pNameDesc->setText(tr("Name"));
	m_pNameSort->setStyleSheet("QToolButton{background:transparent;border: 1px solid #666666;color:#666666;border-radius:5px;font-size:12px;}");

	QRadioButton* pAllBtn = new QRadioButton(tr("All"), pDeviceContentWidget);
	m_pOnlineBtn = new QRadioButton(tr("Online"), pDeviceContentWidget);
	QRadioButton* pOfflineBtn = new QRadioButton(tr("Offline"), pDeviceContentWidget);
	connect(m_pOnlineBtn, SIGNAL(clicked()), this, SLOT(sltOnlineBtnClicked()));
	connect(pOfflineBtn, SIGNAL(clicked()), this, SLOT(sltOfflineBtnClicked()));
	connect(pAllBtn, SIGNAL(clicked()), this, SLOT(sltAllBtnClicked()));
	pTopNavLayout3->addWidget(m_pNameSort);
	//pTopNavLayout3->addStretch();
	//pTopNavLayout3->addWidget(m_pNameDesc);
	pTopNavLayout3->addWidget(pAllBtn);
	pTopNavLayout3->addWidget(m_pOnlineBtn);
	pTopNavLayout3->addWidget(pOfflineBtn);
	pAllBtn->setChecked(true);
	pDeviceContentLayout->addLayout(pTopNavLayout3);
	QHBoxLayout* pSyncDeviceGroupLayout = new QHBoxLayout(pDeviceContentWidget);
	QPushButton* pUpdateDeviceGroupBtn = new QPushButton(tr("Update device group"), pDeviceContentWidget);
	connect(pUpdateDeviceGroupBtn, SIGNAL(clicked()), this, SLOT(sltUpdateDeviceGroup()));
	QPushButton* pCoverDeviceGroupBtn = new QPushButton(tr("Force cover device group"), pDeviceContentWidget);
	connect(pCoverDeviceGroupBtn, SIGNAL(clicked()), this, SLOT(sltCoverDeviceGroup()));
	pSyncDeviceGroupLayout->addWidget(pUpdateDeviceGroupBtn);
	pSyncDeviceGroupLayout->addWidget(pCoverDeviceGroupBtn);
	pDeviceContentLayout->addLayout(pSyncDeviceGroupLayout);

	//m_pDevTree = new DragTreeWidget(this);
	//m_pDevTree->setContextMenuPolicy(Qt::CustomContextMenu);
	////m_pDevTree->header()->setResizeMode()
	////m_pDevTree->setMinimumWidth(250);
	//m_pDevTree->setColumnCount(2);
	//m_pDevTree->setHeaderLabel(tr("device list"));
	//m_pDevTree->setHeaderHidden(true);
	//m_pDevTree->setIconSize(QSize(TREE_ITEM_HEIGHT - 2, TREE_ITEM_HEIGHT - 2));
	//m_pDevTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	//m_pDevTree->header()->setStretchLastSection(false);
	//m_pDevTree->setAcceptDrops(true);
	//m_pDevTree->setDragDropMode(QAbstractItemView::DragDrop);
	//m_pDevTree->setDefaultDropAction(Qt::MoveAction);//drag drop
	//m_pDevTree->setSortingEnabled(false);
	//m_pDevTree->setUniformRowHeights(true);
	//m_pDevTree->setColumnHidden(1, true);
	//m_pDevTree->header()->setSortIndicator(0, Qt::AscendingOrder);
	//m_pDevTree->verticalScrollBar()->setStyleSheet("../Common/Skin/qss/scrollbar.qss");
	////connect(m_pDevTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(onSort(int, Qt::SortOrder)));
	////m_pDevTree->setStyleSheet(QString("QTreeWidget::item{height:%1px;color:red;}QTreeWidget{font-size:17px;}").arg(TREE_ITEM_HEIGHT));
	////connect(m_pDevTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SIGNAL(sglItemDoubleClicked(QTreeWidgetItem*, int)));
	//connect(m_pDevTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SIGNAL(sglItemDoubleClicked(QTreeWidgetItem*, int)));
	//connect(m_pDevTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(sltAddHistoryItem(QTreeWidgetItem*, int)));
	////connect(m_pDevTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SIGNAL(sltCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
	//connect(m_pDevTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sltDevTreeRightClicked(const QPoint&)));
	//VideoWidgetItem* jyDevRoot = new VideoWidgetItem(m_pDevTree, QStringList(tr("JY Video")));
	//jyDevRoot->setData(0, Qt::UserRole + 1, NODE_GROUP);
	//m_pDevTree->insertTopLevelItem(0, jyDevRoot);
	//pDeviceContentLayout->addWidget(m_pDevTree);
	//设备
	m_pDeviceTreeView = new DeviceTreeView(this);
	m_pDeviceTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	pDeviceContentLayout->addWidget(m_pDeviceTreeView);
	m_pDeviceTreeView->setSelectionMode(QTreeView::SingleSelection);
	m_pDeviceTreeView->setDragEnabled(true);
	m_pDeviceTreeView->setAcceptDrops(true);
	m_pDeviceTreeView->setDropIndicatorShown(true);
	m_pDeviceTreeView->setSortingEnabled(true);
	m_pDeviceTreeView->sortByColumn(0, Qt::AscendingOrder);
	m_pDeviceTreeModel = new DeviceTreeModel(nullptr, this);
	m_pDeviceTreeItemRoot = new DeviceTreeItem(tr("JY Video"), m_pDeviceTreeModel->topLevelItem());
	m_pDeviceTreeItemRoot->setRow(m_pDeviceTreeModel->topLevelItem()->childCount());
	m_pDeviceTreeItemRoot->setType(NODE_GROUP);
	m_pDeviceTreeModel->topLevelItem()->appendChild(m_pDeviceTreeItemRoot);
	m_pDeviceTreeView->setModel(m_pDeviceTreeModel);
	m_pDeviceTreeView->setHeaderHidden(true);
	m_pDeviceTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	//历史
	m_pDeviceHistoryTreeModel = new DeviceTreeModel(nullptr, this);
	m_pDeviceHistoryTreeItemRoot = new DeviceTreeItem(tr("JY Video history"), m_pDeviceHistoryTreeModel->topLevelItem());
	m_pDeviceHistoryTreeItemRoot->setRow(m_pDeviceHistoryTreeModel->topLevelItem()->childCount());
	m_pDeviceHistoryTreeItemRoot->setType(NODE_GROUP);
	m_pDeviceHistoryTreeModel->topLevelItem()->appendChild(m_pDeviceHistoryTreeItemRoot);
	//m_pDeviceTreeView->setModel(m_pDeviceHistoryTreeModel);
	m_pDeviceTreeView->setHeaderHidden(true);
	m_pDeviceTreeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	connect(m_pDeviceTreeModel, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)), this, SLOT(sltDeviceGroupChanged()));
	connect(m_pDeviceTreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(sltItemDoubleClicked(const QModelIndex&)));
	connect(m_pDeviceTreeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(sltAddHistoryItem(const QModelIndex&)));
	connect(m_pDeviceTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(sltDevTreeRightClicked(const QPoint&)));
	//CommonHelper::loadStyleSheet(m_pDeviceTreeView, "../Common/Skin/video/deviceTree/DevTree.qss");
	CommonHelper::loadStyleSheet(m_pDeviceTreeView->verticalScrollBar(), "../Common/Skin/qss/scrollbar.qss");
}

void DeviceTreeManageCom::initRightClickMenu()
{
	m_pDevTreeRightClickMenu = new QMenu(this);
	m_pDevRebootAction = new QAction(QIcon("Skin/base/logo.png"), tr("Device Reboot"), m_pDevTreeRightClickMenu);
	m_pDevOutputAction = new QAction(QIcon("Skin/base/logo.png"), tr("Device Output"), m_pDevTreeRightClickMenu);
	m_pGroupMenu = m_pDevTreeRightClickMenu->addMenu(QIcon("Skin/base/logo.png"), tr("Group action"));
	m_pCreateGroupAction = new QAction(QIcon("Skin/base/logo.png"), tr("Create group"), m_pGroupMenu);
	m_pRenameGroupAction = new QAction(QIcon("Skin/base/logo.png"), tr("Rename group"), m_pGroupMenu);
	m_pDeleteGroupAction = new QAction(QIcon("Skin/base/logo.png"), tr("Delete group"), m_pGroupMenu);
	m_pGroupMenu->addAction(m_pCreateGroupAction);
	m_pGroupMenu->addAction(m_pRenameGroupAction);
	m_pGroupMenu->addAction(m_pDeleteGroupAction);
	m_pDevTreeRightClickMenu->addAction(m_pDevRebootAction);
	m_pDevTreeRightClickMenu->addAction(m_pDevOutputAction);
	m_pRemoteVoiceMenu = m_pDevTreeRightClickMenu->addMenu(QIcon("Skin/base/logo.png"), tr("remote voice"));
	connect(m_pCreateGroupAction, SIGNAL(triggered()), this, SLOT(sltGroupAction()));
	connect(m_pRenameGroupAction, SIGNAL(triggered()), this, SLOT(sltGroupAction()));
	connect(m_pDeleteGroupAction, SIGNAL(triggered()), this, SLOT(sltGroupAction()));
	connect(m_pDevRebootAction, SIGNAL(triggered()), this, SLOT(sltDevReboot()));
	connect(m_pDevOutputAction, SIGNAL(triggered()), this, SLOT(sltDevOutput()));
}

void DeviceTreeManageCom::coverDevTree()
{
	JYThingsRequest* req = jyApp->getThings()->createRequest(this, ".", QString("/user/get-profile?keys=priv.deviceGroup"));
	connect(req, SIGNAL(sglResponse(int, int, QString)), this, SLOT(sltSyncGetDeviceGroupResponse(int, int, QString)));
	m_nCoverDeviceGroupReq = req->start();
}

void DeviceTreeManageCom::updateDevicePlayingStatus(DeviceTreeItem* parentItem, QList<QString>& tids)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			updateDevicePlayingStatus(childItem, tids);
		}
		else if (nodeType == NODE_DEVICE) {
			QString tid = childItem->name();
			if (tids.contains(tid)) {
				if (!childItem->isPlaying()) {
					childItem->setPlaying(true);
					emit m_pDeviceTreeModel->dataChanged(m_pDeviceTreeModel->indexFromItem(childItem)
						, m_pDeviceTreeModel->indexFromItem(childItem));
				}
				tids.removeOne(tid);
			}
			else {
				if (childItem->isPlaying()) {
					childItem->setPlaying(false);
					emit m_pDeviceTreeModel->dataChanged(m_pDeviceTreeModel->indexFromItem(childItem)
						, m_pDeviceTreeModel->indexFromItem(childItem));
				}
			}
		}
	}
}

void DeviceTreeManageCom::updateDevicePlayingStatus(QList<QString>& tids)
{
	updateDevicePlayingStatus(m_pDeviceTreeItemRoot, tids);
}

void DeviceTreeManageCom::uploadDeviceGroup()
{
	QString cacheRootPath = QCoreApplication::applicationDirPath() + "/";
	QString fileName = cacheRootPath + jyApp->getThings()->getTid() + DEVICE_GROUP_FILE_NAME;
	const wchar_t* encodedName = reinterpret_cast<const wchar_t*>(fileName.utf16());
	FILE* pSaveHandle = _wfopen(encodedName, L"rb");
	if (!pSaveHandle) {
		QMessageBox::information(this, tr("Tip"), tr("Sync fail"));
		return;
	}
	char* readbuffer = new char[CONFIG_FILE_MAX_LEN];
	memset(readbuffer, 0, CONFIG_FILE_MAX_LEN);
	int len = fread(readbuffer, CONFIG_FILE_MAX_LEN, 1, pSaveHandle);
	fclose(pSaveHandle);
	QString deviceGroupString = QString(readbuffer);
	delete readbuffer;
	qDebug() << deviceGroupString;
	JYThingsRequest* req = jyApp->getThings()->createRequest(this, ".", QString("/user/set-profile?location=priv&key=deviceGroup&value=%1").arg(deviceGroupString));
	connect(req, SIGNAL(sglResponse(int, int, QString)), this, SLOT(sltSyncSetDeviceGroupResponse(int, int, QString)));
	req->start();

}

QString DeviceTreeManageCom::saveToLocalDeviceGroup()
{
	TObject* pNewDeviceGroupJson = new TObject();
	TObject* rootJson = new TObject();
	QString rootName = m_pDeviceTreeItemRoot->name();/*rootItem->data(0, Qt::UserRole+2).toString();*/
	rootJson->set("type", NODE_GROUP);
	rootJson->set("name", rootName.toStdString().c_str());
	TObject* child = new TObject();
	rootJson->set("child", child);
	pNewDeviceGroupJson->set("ver", (m_nDeviceGroupVersion));
	pNewDeviceGroupJson->set("root", rootJson);
	getDeviceGroupToJson(m_pDeviceTreeItemRoot, child);
	QString deviceGroupString = QString(pNewDeviceGroupJson->to_string());
	qDebug() << deviceGroupString;
	QString cacheRootPath = QCoreApplication::applicationDirPath() + "/";
	QString fileName = cacheRootPath + jyApp->getThings()->getTid() + DEVICE_GROUP_FILE_NAME;
	const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(fileName.utf16());
	FILE* m_pGroupConfigHandle = _wfopen(encodedName, L"wb");
	if (!m_pGroupConfigHandle) {
		delete pNewDeviceGroupJson;
		return "";
	}
	char* data = pNewDeviceGroupJson->to_string();
	int ret = fwrite(data, strlen(data), 1, m_pGroupConfigHandle);
	fclose(m_pGroupConfigHandle);
	delete pNewDeviceGroupJson;
	return deviceGroupString;
}

void DeviceTreeManageCom::moveChildDevice(DeviceTreeItem* srcItem, DeviceTreeItem* destItem)
{
	for (int i = 0; i < srcItem->childCount(); i++) {
		DeviceTreeItem* childItem = srcItem->child(i);
		int nodeType = childItem->type();
		QString nodeName = childItem->name();
		if (nodeType == NODE_GROUP) {
			moveChildDevice(childItem, destItem);
		}
		else if (nodeType == NODE_DEVICE) {
			srcItem->removeItem(childItem->row());
			childItem->setParentItem(destItem);
			destItem->appendChild(childItem);
			//if (childItem->parent()){
			//	childItem->parent()->removeChild(childItem);
			//	i--;
			//}
			//else{
			//	m_pDevTree->takeTopLevelItem(m_pDevTree->indexOfTopLevelItem(childItem));
			//}
			//VideoWidgetItem* newItem = (VideoWidgetItem*)childItem->clone();
			//QString childTid = childItem->data(0, Qt::UserRole).toString();
			//destItem->insertChild(0, childItem);
			//if (childItem->childCount() == 1) {
			//	QTreeWidgetItem *pItemCh = childItem->child(0);
			//	pItemCh->setHidden(true);
			//}

		}
	}
}

void DeviceTreeManageCom::getDeviceGroupToJson(DeviceTreeItem* parentItem, TObject* t)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			QString name = childItem->name();
			TObject* group = new TObject();
			group->set("type", NODE_GROUP);
			group->set("name", name.toStdString().c_str());
			TObject* child = new TObject();
			group->set("child", child);
			t->push(group);
			getDeviceGroupToJson(childItem, child);
		}
		else if (nodeType == NODE_DEVICE) {
			TObject* device = new TObject();
			device->set("type", NODE_DEVICE);
			QString childTid = childItem->name();
			device->set("name", childTid.toStdString().c_str());
			t->push(device);
		}
	}
}

//void DeviceTreeManageCom::updateDeviceTreeByAction(DeviceTreeItem* parentItem, THINGS_OBJ* obj, QString& action)
//{
//	for (int i = 0; i < parentItem->childCount(); i++) {
//		if (action.isEmpty()) { break; }
//		DeviceTreeItem* childItem = parentItem->child(i);
//		int nodeType = childItem->type();
//		QString nodeName = childItem->name();
//		if (nodeType == NODE_GROUP) {
//			updateDeviceTreeByAction(childItem, obj, action);
//		}
//		else if (nodeType == NODE_DEVICE) {
//			QString childTid = childItem->data(0, Qt::UserRole).toString();
//			if (childTid == obj->tid) {
//				if (action == "update") {
//					updateDevItem(childItem, obj);
//				}
//				else if (action == "delete") {
//					m_pDevTree->removeItemWidget(childItem, 0);
//					childItem->setHidden(true);
//					delete childItem;
//				}
//				action = "";
//			}
//		}
//	}
//}

//void DeviceTreeManageCom::updateDeviceTree(DeviceTreeItem* parentItem)
//{
//	for (int i = 0; i < parentItem->childCount(); i++) {
//		DeviceTreeItem* childItem = parentItem->child(i);
//		int nodeType = childItem->type();
//		QString name = childItem->name();
//		if (nodeType == NODE_GROUP) {
//			updateDeviceTree(childItem);
//		}
//		else if (nodeType == NODE_DEVICE) {
//			QString childTid = childItem->name();
//			if (m_listUpdatingTid.contains(childTid)) {
//				THINGS_OBJ obj = jyApp->getThings()->getThingsObj(childTid);
//				updateDevItem(childItem, &obj);
//				m_listUpdatingTid.removeOne(childTid);
//			}
//			else if (m_listRemoveTid.contains(childTid)) {
//				m_pDevTree->removeItemWidget(childItem, 0);
//				childItem->setHidden(true);
//				delete childItem;
//			}
//		}
//	}
//}

//void DeviceTreeManageCom::updateDeviceTreeCount()
//{
//	VideoWidgetItem* rootItem = (VideoWidgetItem*)m_pDevTree->topLevelItem(0);
//	updateDeviceTreeCount(rootItem);
//	DeviceTreeItemCount count = getThingsListCount();
//	QVariant var;
//	var.setValue(count);
//	rootItem->setData(1, Qt::DisplayRole, var);
//	rootItem->setText(0, tr("JY Video") + QString(" (%1/%2)").arg(count.online).arg(count.all));
//}

//void DeviceTreeManageCom::updateDeviceTreeCount(VideoWidgetItem* item)
//{
//	for (int i = 0; i < item->childCount(); i++) {
//		VideoWidgetItem* childItem = (VideoWidgetItem*)item->child(i);
//		int nodeType = childItem->data(0, Qt::UserRole + 1).toInt();
//		QString nodeName = childItem->data(0, Qt::UserRole + 2).toString();
//		if (nodeType == NODE_GROUP) {
//			updateDeviceTreeCount(childItem);
//			DeviceTreeItemCount count = { 0,0 };
//			getTreeCount(childItem, count);
//			QVariant var;
//			var.setValue(count);
//			childItem->setData(1, Qt::DisplayRole, var);
//			childItem->setText(0, nodeName + QString(" (%1/%2)").arg(count.online).arg(count.all));
//			childItem->setToolTip(0, nodeName + QString(" (%1/%2)").arg(count.online).arg(count.all));
//		}
//	}
//}

void DeviceTreeManageCom::coverDeviceGroup(QString& deviceGroupString, bool refresh)
{
	int ver = -1;
	TObject* deviceGroupJson = parse_json(deviceGroupString.toStdString().c_str());
	if (!deviceGroupJson)return;
	if (deviceGroupJson && deviceGroupJson->member("ver")) {
		ver = deviceGroupJson->member("ver")->to_int();
		if (ver == -1) {
			ver = 0;
		}
	}
	else {
		ver = 0;
	}
	if (ver <= m_nDeviceGroupVersion && !refresh) {
		if (deviceGroupJson) delete deviceGroupJson;
		return;
	}
	m_nDeviceGroupVersion = ver;
	TObject* devRoot = deviceGroupJson->member("root");
	if (!devRoot) {
		delete deviceGroupJson;
		return;
	}
	TObject* devRoot1 = devRoot->member("child");
	if (!devRoot1) {
		delete deviceGroupJson;
		return;
	}
	m_listAllFollowTid.clear();
	m_listAllFollowTid = jyApp->getThings()->getThingsObjTidList();
	m_pDeviceTreeModel->removeRowChildren(m_pDeviceTreeItemRoot);
	getJsonToDeviceGroup(devRoot1, m_pDeviceTreeItemRoot);
	for (int i = 0; i < m_listAllFollowTid.count(); i++) {
		addNewDevItem(m_pDeviceTreeItemRoot, m_listAllFollowTid.at(i));
	}
	m_pDeviceTreeModel->sort(0);
	m_pDeviceTreeView->expand(m_pDeviceTreeModel->index(0, 0));
	m_listAllFollowTid.clear();
	m_listAllFollowTid = jyApp->getThings()->getThingsObjTidList();
	delete deviceGroupJson;
}

void DeviceTreeManageCom::coverDeviceGroup()
{
	m_nDeviceGroupVersion = 0;
	int parts[] = { 2000 };
	m_listAllFollowTid.clear();
	m_listAllFollowTid = jyApp->getThings()->getThingsObjTidList();
	m_pDeviceTreeModel->removeRowChildren(m_pDeviceTreeItemRoot);
	for (int i = 0; i < m_listAllFollowTid.count(); i++) {
		addNewDevItem(m_pDeviceTreeItemRoot, m_listAllFollowTid.at(i));
	}
	m_pDeviceTreeModel->sort(0);
	m_pDeviceTreeView->expand(m_pDeviceTreeModel->index(0, 0));
	m_listAllFollowTid.clear();
	m_listAllFollowTid = jyApp->getThings()->getThingsObjTidList();
}

void DeviceTreeManageCom::getJsonToDeviceGroup(TObject* parentT, DeviceTreeItem* parentItem)
{
	for (int i = 0; i < parentT->length(); i++) {
		TObject* itemNode = parentT->member(i);
		int nodeType = itemNode->member("type") ? itemNode->member("type")->to_int() : 0;
		QString nodeName = itemNode->member("name") ? QString(itemNode->member("name")->to_string()) : "";
		if (nodeType == NODE_GROUP) {
			DeviceTreeItem* itemNewGroup = new DeviceTreeItem(nodeName, parentItem);
			itemNewGroup->setType(NODE_GROUP);
			parentItem->appendChild(itemNewGroup);
			TObject* childNode = itemNode->member("child");
			getJsonToDeviceGroup(childNode, itemNewGroup);
		}
		else if (nodeType == NODE_DEVICE) {
			if (m_listAllFollowTid.contains(nodeName)) {
				addNewDevItem(parentItem, nodeName);
				m_listAllFollowTid.removeOne(nodeName);
			}
		}
	}
}

QString DeviceTreeManageCom::getIconName(QString& tid, bool playing)
{
	QString model = "";
	THINGS_OBJ things;
	things = jyApp->getThings()->getThingsObj(tid);
	model = things.model.toUpper();
	QString iconName = "";
	if (model.contains("CN6870") || model.contains("CN6871")) {
		iconName += "gateway_";
	}
	else if (model.contains("CN6953")) {
		iconName += "gun_";
	}
	else if (model.contains("CN6913")) {
		iconName += "ball_";
	}
	else if (model.contains("CN6808")) {
		iconName += "alarm_";
	}
	else if (model.contains("CN6811") || model.contains("CN6801")) {
		iconName += "talk_";
	}
	else if (model.contains("CN6901") || model.contains("CN6903")) {
		iconName += "card_";
	}
	else if (model.contains("CN6812") || model.contains("CN6809")) {
		iconName += "alarm_video_";
	}
	else {
		iconName += "dvr_";
	}
	if (things.online && !playing) {
		iconName += "online.png";
	}
	else if (things.online && playing) {
		iconName += "playing.png";
	}
	else {
		iconName += "offline.png";
	}
	iconName = "../Common/Skin/video/deviceTree/" + iconName;
	return iconName;
}

QString DeviceTreeManageCom::getIconName(QString& model, bool online, bool playing)
{
	QString iconName = "";
	if (model.contains("CN6870") || model.contains("CN6871")) {
		iconName += "gateway_";
	}
	else if (model.contains("CN6953")) {
		iconName += "gun_";
	}
	else if (model.contains("CN6913")) {
		iconName += "ball_";
	}
	else if (model.contains("CN6808")) {
		iconName += "alarm_";
	}
	else if (model.contains("CN6811") || model.contains("CN6801")) {
		iconName += "talk_";
	}
	else if (model.contains("CN6901") || model.contains("CN6903")) {
		iconName += "card_";
	}
	else if (model.contains("CN6812") || model.contains("CN6809")) {
		iconName += "alarm_video_";
	}
	else {
		iconName += "dvr_";
	}
	if (online && !playing) {
		iconName += "online.png";
	}
	else if (online && playing) {
		iconName += "playing.png";
	}
	else {
		iconName += "offline.png";
	}
	iconName = "../Common/Skin/video/deviceTree/" + iconName;
	return iconName;
}

QString DeviceTreeManageCom::getIconName(CHANNEL_OBJ vc)
{
	QString iconName = "ch_";
	if (vc.online && vc.enable) {
		iconName += "online.png";
	}
	else {
		iconName += "offline.png";
	}
	iconName = "../Common/Skin/video/deviceTree/" + iconName;
	return iconName;
}

bool DeviceTreeManageCom::addNewDevItem(DeviceTreeItem* parentItem, const QString& tid)
{
	if (!parentItem || tid.isEmpty()) {
		return false;
	}
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(tid);
	if (things.partId != 2000)return false;
	DeviceTreeItem* pDevItem = new DeviceTreeItem(tid, parentItem);
	pDevItem->setRow(parentItem->childCount());
	pDevItem->setType(NODE_DEVICE);
	parentItem->appendChild(pDevItem);
	pDevItem->updateChannel();
}

void DeviceTreeManageCom::addNewDevItem(DeviceTreeItem* parentItem, CWEcoderInfo * CWEcoderInfo)
{
	if (parentItem == nullptr) {
		return;
	}
	tagEcoderInfo encoderInfo;
	encoderInfo.ip = CWEcoderInfo->ip;
	encoderInfo.port = CWEcoderInfo->port;
	encoderInfo.user = CWEcoderInfo->user;
	encoderInfo.pwd = CWEcoderInfo->pwd;
	encoderInfo.isOnline = CWEcoderInfo->isOnline;
	encoderInfo.devClass = CWEcoderInfo->devClass;
	encoderInfo.devName = CWEcoderInfo->devName;
	QVariant varDev;
	varDev.setValue(encoderInfo);
	DeviceTreeItem* pDevItem = new DeviceTreeItem(varDev, parentItem);
	pDevItem->setType(NODE_DEVICE_MATRIX);
	parentItem->appendChild(pDevItem);
	CWEncChnMap chnMap = CWEcoderInfo->mapCWEncChnMap;
	CWEncChnMap::iterator it;
	for (it = chnMap.begin();it!=chnMap.end();it++){
		CWEncChannel CEncChannel = it.value();
		CHANNEL_OBJ chObj;
		chObj.channel = CEncChannel.m_nChannelID;
		chObj.channelName = CEncChannel.m_strName;
		chObj.enable = true;
		chObj.online = true;
		QVariant var;
		var.setValue(chObj);
		DeviceTreeItem* itemCh = new DeviceTreeItem(var, pDevItem);
		itemCh->setType(NODE_CHANNEL_MATRIX);
		pDevItem->appendChild(itemCh);
	}

}

QString DeviceTreeManageCom::getCmdPwd()
{
	QString ret = nullptr;
	PwdInputDialog dlg(this);
	dlg.setInputTitle(tr("please intput password:"));
	if (QDialog::Accepted == dlg.exec()) {
		ret = dlg.getInputValue();
	}
	return ret;
}

void DeviceTreeManageCom::sltNavClicked()
{
	JYToolButton* btn = (JYToolButton*)sender();
	if (btn == m_pDeviceNavBtn) {
		m_pDeviceNavBtn->setChecked(true);
		m_pHistoryNavBtn->setChecked(false);
		//m_pContentStackedLayout->setCurrentIndex(0);
		m_pDeviceTreeView->setModel(m_pDeviceTreeModel);
		m_pDeviceTreeModel->layoutChanged();
		m_pDeviceTreeView->expand(m_pDeviceTreeModel->index(0, 0));
	}
	else if (btn == m_pFavoritesNavBtn) {

	}
	else if (btn == m_pHistoryNavBtn) {
		m_pDeviceNavBtn->setChecked(false);
		m_pHistoryNavBtn->setChecked(true);
		//m_pContentStackedLayout->setCurrentIndex(1);
		m_pDeviceTreeView->setModel(m_pDeviceHistoryTreeModel);
		m_pDeviceTreeModel->layoutChanged();
		m_pDeviceTreeView->expand(m_pDeviceTreeModel->index(0, 0));
	}
}

void DeviceTreeManageCom::sltSearVideo()
{
	QString text = m_pEdtKeyWord->text().trimmed();
	//if (text.isEmpty()){
	//	emit m_pDeviceTreeModel->layoutChanged();
	//	return;
	//}
	m_pDeviceTreeModel->filter(text, m_pDeviceTreeItemRoot);
	emit m_pDeviceTreeModel->layoutChanged();
}

void DeviceTreeManageCom::sltNameSortBtnClicked()
{
	if (!m_pDeviceTreeView->isSortingEnabled()) {
		m_pDeviceTreeView->setSortingEnabled(true);
	}
	if (m_bNameSortASC) {
		m_pNameSort->setIcon(QIcon("../Common/Skin/common/down_h.png"));
		m_pDeviceTreeModel->sort(0, Qt::DescendingOrder);
	}
	else {
		m_pNameSort->setIcon(QIcon("../Common/Skin/common/up_h.png"));
		m_pDeviceTreeModel->sort(0, Qt::AscendingOrder);
	}
	m_bNameSortASC = !m_bNameSortASC;
}


void DeviceTreeManageCom::sltAllBtnClicked()
{
	m_pDeviceTreeModel->filter(QString(""), m_pDeviceTreeItemRoot);
	emit m_pDeviceTreeModel->layoutChanged();
}

void DeviceTreeManageCom::sltOnlineBtnClicked()
{
	m_pDeviceTreeModel->filter(true, m_pDeviceTreeItemRoot);
	emit m_pDeviceTreeModel->layoutChanged();
}

void DeviceTreeManageCom::sltOfflineBtnClicked()
{
	m_pDeviceTreeModel->filter(false, m_pDeviceTreeItemRoot);
	emit m_pDeviceTreeModel->layoutChanged();
}

void DeviceTreeManageCom::sltDevReboot()
{
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(m_pDeviceTreeView->currentIndex());
	QString itemTid = pItem->name();
	if (!itemTid.isEmpty()) {
		QString pass = getCmdPwd();
		if (pass != nullptr) {
			QString sendCmd = QString("%1,,cmd,reboot,%2").arg(itemTid).arg(pass);
			jyApp->getThings()->pushMessage("im", sendCmd);
		}
	}
}
void DeviceTreeManageCom::sltDevOutput()
{
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(m_pDeviceTreeView->currentIndex());
	QString itemTid = pItem->name();
	QString videoName = pItem->data(0).toString();
	if (itemTid.isEmpty()) {
		return;
	}
	QList<QString> outList;
	int ret = DevieCapability::getOutputsList(itemTid, outList);
	if (ret <= 0) {
		QMessageBox::warning(this, tr("Warning"), "Get output channel failed");
		return;
	}
	DeviceOutput* pDeviceOutputDlg = new DeviceOutput(this);
	pDeviceOutputDlg->setDeviceName(videoName);
	pDeviceOutputDlg->setTid(itemTid);
	pDeviceOutputDlg->setOutputList(outList);
	pDeviceOutputDlg->exec();
}

void DeviceTreeManageCom::sltGroupAction()
{
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(m_pDeviceTreeView->currentIndex());
	DeviceTreeItem* pParentItem = pItem->parentItem();
	if (pItem == nullptr || pParentItem == nullptr) return;

	QAction* pAction = qobject_cast<QAction*>(sender());
	CreateGroupDlg* pGroupDlg;
	QString newGroupName = "";
	GROUP_ACTION_TYPE action = GAT_NONE;
	if (pAction == m_pCreateGroupAction) {
		action = GAT_CREATE;
	}
	else if (pAction == m_pRenameGroupAction) {
		action = GAT_RENAME;
	}
	else if (pAction == m_pDeleteGroupAction) {
		action = GAT_DELETE;
	}
	if (action == GAT_NONE) {
		return;
	}
	pGroupDlg = new CreateGroupDlg(action, this);
	if (pGroupDlg->exec() == QDialog::Rejected) {
		delete pGroupDlg;
		return;
	}
	if (pAction == m_pDeleteGroupAction) {
		//将分组下所有设备移到根目录下,然后删除自己
		if (!m_pDeviceTreeModel->moveRows(pItem, m_pDeviceTreeItemRoot)) {
			//return;
		}
		m_pDeviceTreeModel->removeRow(pItem);
	}
	else if (pAction == m_pCreateGroupAction) {
		newGroupName = pGroupDlg->getGroupName();
		if (newGroupName.isEmpty()) {
			QMessageBox::warning(this, tr("Warning"), tr("Name not allow empty"));
		}
		m_pDeviceTreeModel->insertRow(newGroupName, NODE_GROUP, pItem);
	}
	else if (pAction == m_pRenameGroupAction) {
		newGroupName = pGroupDlg->getGroupName();
		pItem->setName(newGroupName);
	}
	pGroupDlg->deleteLater();
	emit sglDeviceGroupChanged();
}

void DeviceTreeManageCom::sltAddHistoryItem(const QModelIndex& index)
{
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(index);
	int nodeType = pItem->type();
	QString tid;
	if (nodeType == NODE_CHANNEL) {
		tid = pItem->parentItem()->name();
	}
	else {
		tid = pItem->name();
	}
	for (int i = 0; i < m_pDeviceHistoryTreeItemRoot->childCount(); i++) {
		if (tid == m_pDeviceHistoryTreeItemRoot->child(i)->name()) {
			return;
		}
	}
	addNewDevItem(m_pDeviceHistoryTreeItemRoot, tid);
}

void DeviceTreeManageCom::sltItemDoubleClicked(const QModelIndex& index)
{
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(index);
	if (pItem->type() != NODE_GROUP && pItem->type() != NODE_UNKNOWN) {
		emit sglItemDoubleClicked(pItem);
		//if (pItem->type() > NODE_DEVICE) { return; }
		//DeviceTreeItem* pItemDev = pItem;
		//if (pItem->type() == NODE_CHANNEL) {
		//	pItemDev = pItem->parentItem();
		//}
		//pItemDev->setPlaying(true);
	}
}

void DeviceTreeManageCom::updateRemoteVoiceBtn()
{
	if (m_sDeviceVoicePlay.tid.isEmpty() || m_sDeviceVoicePlay.rvds.count() < 1) {
		return;
	}
	m_pRemoteVoiceMenu->clear();
	for (int i = 0; i < m_sDeviceVoicePlay.rvfs.count(); i++) {
		RemoteVoiceFile rvf = m_sDeviceVoicePlay.rvfs.at(i);
		QAction* voiceFile = new QAction(QIcon("Skin/base/logo.png"), rvf.name, m_pRemoteVoiceMenu);
		QVariant v;
		v.setValue(rvf.id);
		voiceFile->setData(v);
		m_pRemoteVoiceMenu->addAction(voiceFile);
		connect(voiceFile, SIGNAL(triggered()), this, SLOT(sltRemoteVoicePlay()));
	}
}

void DeviceTreeManageCom::getDeviceRemotePlayParam(QString tid)
{
	m_pRemoteVoiceMenu->clear();
	m_sDeviceVoicePlay.tid = tid;
	QString addr = jyApp->getThings()->getVars(m_sDeviceVoicePlay.tid, "session.1.addr");
	if (addr.isEmpty()) {
		return;
	}
	JYThingsRequest* req = jyApp->getThings()->createRequest(this, addr, QString("/remote-play/get-param"));
	connect(req, SIGNAL(sglResponse(int, int, QString)), this, SLOT(sltDeviceRemotePlayParam(int, int, QString)));
	req->start();
}



void DeviceTreeManageCom::sltDeviceRemotePlayParam(int req, int code, QString data)
{
	qDebug() << code << data;
	QList<QString> lDevs;
	if (code == 200) {
		TObject* root = parse_json(data.toStdString().c_str());
		if (!root) {
			return;
		}
		m_sDeviceVoicePlay.rvds.clear();
		m_sDeviceVoicePlay.rvfs.clear();
		TObject* devs = root->member("devs");
		if (!devs) { delete root;  return; }
		for (int i = 0; i < devs->length(); i++) {
			TObject* dev = devs->member(i);
			if (!dev) { continue; }
			RemoteVoiceDevice rvd;
			rvd.id = QString(dev->to_string("id"));
			rvd.name = QString(dev->to_string("name"));
			m_sDeviceVoicePlay.rvds.append(rvd);
			break;
		}
		TObject* files = root->member("files");
		if (!files) { delete root; return; }
		for (int i = 0; i < files->length(); i++) {
			TObject* file = files->member(i);
			if (!file) { continue; }
			RemoteVoiceFile rvf;
			rvf.id = QString(file->to_string("id"));
			rvf.name = QString(file->to_string("name"));
			m_sDeviceVoicePlay.rvfs.append(rvf);
		}
		delete root;
		updateRemoteVoiceBtn();
	}
}

void DeviceTreeManageCom::sltRemoteVoicePlay()
{
	QAction* pAction = (QAction*)sender();
	QString devId = m_sDeviceVoicePlay.rvds.at(0).id;
	QString fileId = pAction->data().value<QString>();
	QString addr = jyApp->getThings()->getVars(m_sDeviceVoicePlay.tid, "session.1.addr");
	if (addr.isEmpty()) {
		QMessageBox::information(this, tr("Warning"), tr("Device offline"));
		return;
	}
	else if (devId.isEmpty() || fileId.isEmpty()) {
		QMessageBox::information(this, tr("Warning"), tr("Remote voice file get failed"));
		return;
	}

	JYThingsRequest* req = jyApp->getThings()->createRequest(this, addr, QString("/remote-play/play?dev=%1&file=%2").arg(devId).arg(fileId));
	connect(req, SIGNAL(sglResponse(int, int, QString)), this, SLOT(sltDeviceRemotePlay(int, int, QString)));
	req->start();
}

void DeviceTreeManageCom::sltDeviceRemotePlay(int req, int code, QString data)
{
	qDebug() << code << data;
	if (code == 200) {
		QMessageBox::information(this, tr("Warning"), tr("Cmd send successful"));
	}
	else {
		QMessageBox::information(this, tr("Warning"), tr("Cmd send failed"));
	}
}


bool DeviceTreeManageCom::updateDevTreeRightClickMenu(int nodeType)
{
	if (nodeType == NODE_GROUP) {
		m_pGroupMenu->menuAction()->setVisible(true);
		m_pDevRebootAction->setVisible(false);
		m_pDevOutputAction->setVisible(false);
		m_pRemoteVoiceMenu->menuAction()->setVisible(false);
	}
	else if (nodeType == NODE_DEVICE) {
		m_pGroupMenu->menuAction()->setVisible(false);
		m_pDevRebootAction->setVisible(true);
		m_pDevOutputAction->setVisible(true);
		m_pRemoteVoiceMenu->menuAction()->setVisible(true);
	}
	else {
		return false;
	}
	return true;
}

void DeviceTreeManageCom::sltDevTreeRightClicked(const QPoint & point)
{
	//VideoWidgetItem* pItem = (VideoWidgetItem*)m_pDevTree->itemAt(point);
	QModelIndex index = m_pDeviceTreeView->currentIndex();
	m_pRightClickItem = m_pDeviceTreeModel->itemFromIndex(index);
	//if (pItem == nullptr) return;
	//m_pRightClickItem = pItem;
	int nodeType = m_pRightClickItem->type();
	bool bRet = updateDevTreeRightClickMenu(nodeType);
	if (!bRet) {
		return;
	}
	if (nodeType == NODE_DEVICE) {
		//QVariant var = pItem->data(0, Qt::UserRole);
		//CHANNEL_OBJ chObj = var.value<CHANNEL_OBJ>();
		QString itemTid = m_pRightClickItem->name();
		if (!itemTid.isEmpty()) {
			getDeviceRemotePlayParam(itemTid);
		}
	}
	m_pDevTreeRightClickMenu->move(QCursor::pos());
	m_pDevTreeRightClickMenu->show();
}


void DeviceTreeManageCom::sltCoverDeviceGroup()
{
	if (QMessageBox::question(this, tr("Warnning"), tr("Local group config will be covered, Confirm?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
		coverDevTree();
	}
}

void DeviceTreeManageCom::sltUpdateDeviceGroup()
{
	if (QMessageBox::question(this, tr("Warnning"), tr("Server group config will be covered, Confirm?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
		uploadDeviceGroup();
	}
}

void DeviceTreeManageCom::sltSyncGetDeviceGroupResponse(int req, int code, QString data)
{
	if (m_nCoverDeviceGroupReq == req) {
		if (code == 200 && !data.isEmpty()) {
			int position = data.indexOf("priv.deviceGroup");
			QString data1 = data.replace(position, 16, "deviceGroup");
			TObject* root = parse_json(data1.toStdString().c_str());
			TObject* devRoot = root->member("deviceGroup");
			if (root && devRoot) {
				QString data2 = QString(devRoot->to_string());
				coverDeviceGroup(data2, true);
				saveToLocalDeviceGroup();
			}
			else {
				coverDeviceGroup();
				saveToLocalDeviceGroup();
			}
			if (root) {
				delete root;
			}
		}
	}
}

void DeviceTreeManageCom::sltSyncSetDeviceGroupResponse(int req, int code, QString data)
{
	QString displayString = "";
	if (code != 200) {
		displayString = tr("Sync fail") + QString(" code: %1, error: %2").arg(code).arg(data);
	}
	else {
		displayString = QString(tr("Sync success"));
	}
	QMessageBox::information(this, tr("Tip"), displayString);
}

void DeviceTreeManageCom::thingsNodeNew(QString tid)
{
	if (tid.isEmpty() || m_listAllFollowTid.contains(tid)) {
		return;
	}
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(tid);
	if (things.partId != 2000)return;
	bool ret = m_pDeviceTreeModel->insertRow(tid, NODE_DEVICE, m_pDeviceTreeItemRoot);
	m_listAllFollowTid.append(tid);
}

void DeviceTreeManageCom::thingsNodeRemove(QString tid)
{
	QModelIndexList indexs = m_pDeviceTreeModel->match(m_pDeviceTreeModel->index(0, 0), Qt::UserRole,
		QVariant::fromValue(tid), 1, Qt::MatchRecursive);
	if (indexs.size() < 1)return;
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(indexs.first());
	m_pDeviceTreeModel->removeRow(pItem);

	m_listAllFollowTid.removeOne(tid);
}

void DeviceTreeManageCom::thingsNodeUpdate(QString tid)
{
	//qDebug() << "update " << tid;
	QModelIndexList indexs = m_pDeviceTreeModel->match(m_pDeviceTreeModel->index(0, 0), Qt::UserRole,
		QVariant::fromValue(tid), 1, Qt::MatchRecursive);
	if (indexs.size() < 1)return;
	DeviceTreeItem* pItem = m_pDeviceTreeModel->itemFromIndex(indexs.first());
	m_pDeviceTreeModel->removeRowChildren(pItem);
	m_pDeviceTreeModel->insertDeviceChannel(pItem);
}

void DeviceTreeManageCom::sltThingsNodeEvent(QString tid, int code)
{
	if (code == E_FOLLOWED_NEW) {
		thingsNodeNew(tid);
	}
	else if (code == E_FOLLOWED_REMOVE) {
		thingsNodeRemove(tid);
	}
	else if (code == E_VIDEO_CHANNEL_CHANGED) {
		thingsNodeUpdate(tid);
	}
	else if (code == E_FOLLOWED_RESET) {
		QModelIndexList indexs = m_pDeviceTreeModel->match(m_pDeviceTreeModel->index(0, 0), Qt::UserRole,
			QVariant::fromValue(tid), 1, Qt::MatchRecursive);
		if (indexs.size() < 1)return;
		emit m_pDeviceTreeModel->dataChanged(indexs.first(), indexs.first().child(0,0));
	}
}

void DeviceTreeManageCom::sltMatrixDeviceOnline(QString& devIp, bool online)
{
	if (m_pMatrixDeviceTreeItemRoot == nullptr) { return; }
	for each (auto item in m_pMatrixDeviceTreeItemRoot->m_listChildItem)
	{
		tagEcoderInfo encoderInfo = item->m_Variant.value<tagEcoderInfo>();
		if (encoderInfo.ip.contains(devIp)) {
			encoderInfo.isOnline = online;
			 QVariant var;
			 item->m_Variant.setValue(encoderInfo);
		}
	}
}

//void DeviceTreeManageCom::sltThingsEvent(int event)
//{
//	if (event == E_FOLLOWED_START) {
//		//m_pDevTree->setSortingEnabled(false);
//	}
//	else if (event == E_FOLLOWED_END) {
//		//m_pDevTree->setSortingEnabled(true);
//	}
//}

void DeviceTreeManageCom::sltDeviceGroupChanged()
{
	m_nDeviceGroupVersion++;
	//m_pDeviceTreeModel->sort(0, m_bNameSortASC ? Qt::AscendingOrder : Qt::DescendingOrder);
	saveToLocalDeviceGroup();
}

void DeviceTreeManageCom::sltQTreeviewScrollMove(int v)
{
	//int currValue = m_pDevTree->verticalScrollBar()->value() + v;
	//m_pDevTree->verticalScrollBar()->setValue(currValue);
}

void DeviceTreeManageCom::slt2MinTimeout()
{
	m_pDeviceTreeModel->sort(0, m_bNameSortASC ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void DeviceTreeManageCom::showEvent(QShowEvent* event)
{
	QString cacheRootPath = QCoreApplication::applicationDirPath() + "/";
	QString fileName = cacheRootPath + jyApp->getThings()->getTid() + DEVICE_GROUP_FILE_NAME;
	const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(fileName.utf16());
	FILE* pSaveHandle = _wfopen(encodedName, L"rb");
	if (pSaveHandle) {
		char *readbuffer = new char[CONFIG_FILE_MAX_LEN];
		memset(readbuffer, 0, CONFIG_FILE_MAX_LEN);
		int len = fread(readbuffer, CONFIG_FILE_MAX_LEN, 1, pSaveHandle);
		QString deviceGroup = QString(readbuffer);
		delete readbuffer;
		fclose(pSaveHandle);
		coverDeviceGroup(deviceGroup);
	}
	else {
		coverDevTree();
	}
}
