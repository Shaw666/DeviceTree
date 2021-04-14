#pragma once
#include "BaseWidget.h"
#include <QLineEdit>
//#include <DragTreeWidget.h>
//#include "VideoWidgetItem.h"
#include "tparser.h"
#include "things_def.h"
#include <QMenu>
#include "ClientDataDef.h"
#include <qradiobutton.h>
#include "JYToolButton.h"
#include <qstackedlayout.h>
#include "ConfigHelper.h"
#include "MatrixLibDef.h"
#include "DeviceTreeModel.h"
#include "DeviceTreeItem.h"
#include <qtreeview.h>
#include "DeviceTreeView.h"

constexpr auto DEVICE_GROUP_FILE_NAME = "devicegroup.config";
constexpr auto CONFIG_FILE_MAX_LEN = 1024*1024;

//struct DeviceTreeItemCount
//{
//	int all;
//	int online;
//};

class DeviceTreeManageCom : public BaseWidget
{
	Q_OBJECT
public:
	DeviceTreeManageCom(bool hideChannelWhenOffline = true, QWidget* parent = nullptr);
	~DeviceTreeManageCom();
	void updateMatrixDev(QList<CWEcoderInfo*>* listEncoder);
	static QString getIconName(QString & tid, bool playing = false);
	static QString getIconName(QString & model, bool online, bool playing);
	static QString getIconName(CHANNEL_OBJ vc);
	void coverDeviceGroup(QString& deviceGroupString, bool refresh = false);
	void coverDevTree();
	void updateDevicePlayingStatus(DeviceTreeItem* parentItem, QList<QString>& tids);
	void updateDevicePlayingStatus(QList<QString>& tids);
public:
	//DragTreeWidget* m_pDevTree;
	//DragTreeWidget* m_pHistoryTree;
protected:
	void showEvent(QShowEvent * event);
private:
	QLineEdit* m_pEdtKeyWord;
	TObject* m_pDeviceGroupJson;
	int m_nDeviceGroupVersion;
	int m_nCoverDeviceGroupReq;
	int m_nUpdateDeviceGroupReq;
	QStringList m_listAllFollowTid;
	QMenu* m_pDevTreeRightClickMenu;
	QMenu* m_pRemoteVoiceMenu;
	QMenu* m_pGroupMenu;
	QAction* m_pCreateGroupAction;
	QAction* m_pRenameGroupAction;
	QAction* m_pDeleteGroupAction;
	QAction* m_pDevRebootAction;
	QAction* m_pDevOutputAction;
	DeviceTreeItem* m_pRightClickItem;
	DeviceVoicePlay m_sDeviceVoicePlay;
	QToolButton* m_pNameSort;
	//QToolButton* m_pNameDesc;
	bool m_bNameSortASC;
	//bool m_bNumberSortDESC;
	QRadioButton* m_pOnlineBtn;
	JYToolButton* m_pDeviceNavBtn;
	JYToolButton* m_pFavoritesNavBtn;
	JYToolButton* m_pHistoryNavBtn;
	//QStackedLayout* m_pContentStackedLayout;
	//QList<QString> m_listUpdateTid;
	//QList<QString> m_listUpdatingTid;
	//QList<QString> m_listRemoveTid;
	//QList<QString> m_listNewTid;
	//QMap<QString, THINGS_OBJ>* m_mapThingsObj;
	bool m_bHideChannelWhenOffline;
	DeviceTreeView* m_pDeviceTreeView;
	DeviceTreeModel* m_pDeviceTreeModel;
	DeviceTreeItem* m_pDeviceTreeItemRoot;
	DeviceTreeModel* m_pDeviceHistoryTreeModel;
	DeviceTreeItem* m_pDeviceHistoryTreeItemRoot;
	DeviceTreeItem* m_pMatrixDeviceTreeItemRoot;
private:
	void initView();
	void initRightClickMenu();
	void uploadDeviceGroup();
	//QString getDeviceGroupConfig();
	QString saveToLocalDeviceGroup();
	//void updateAllDeviceNode(QTreeWidgetItem * parentItem);
	//void updateDeviceGroup();
	void moveChildDevice(DeviceTreeItem* srcItem, DeviceTreeItem* destItem);
	void getDeviceGroupToJson(DeviceTreeItem* item, TObject * t);
	//void getAllDevice(DeviceTreeItem* item, THINGS_OBJ& obj, QString & action);
	//void updateDeviceTreeByAction(DeviceTreeItem* item, THINGS_OBJ * obj, QString & action);
	//void updateDeviceTree(DeviceTreeItem* item);
	//void updateDeviceTreeCount();
	//void updateDeviceTreeCount(VideoWidgetItem * item);
	//void updateDeviceGroup(QString tid);
	//void getAllDevice(VideoWidgetItem * item, int & devCount);
	//计算group下设备数量
	//void getTreeCount(VideoWidgetItem * item, DeviceTreeItemCount& treeCount);
	//DeviceTreeItemCount getThingsListCount();
	void coverDeviceGroup();
	void getJsonToDeviceGroup(TObject * parentT, DeviceTreeItem* parentItem);
	bool addNewDevItem(DeviceTreeItem* parentItem, const QString& tid);
	void addNewDevItem(DeviceTreeItem* parentItem, CWEcoderInfo* CEncoder);
	void updateDevItem(DeviceTreeItem* itemDev, THINGS_OBJ * things);
	//void updateDevItem(DeviceTreeItem* itemDev, THINGS_OBJ & things);
	bool updateDevTreeRightClickMenu(int nodeType);
	QString getCmdPwd();
	void getDeviceRemotePlayParam(QString tid);
	void updateRemoteVoiceBtn();
	void thingsNodeNew(QString tid);
	void thingsNodeRemove(QString);
	void thingsNodeUpdate(QString tid);
public slots:
	void sltQTreeviewScrollMove(int);
	void sltMatrixDeviceOnline(QString&, bool);
private slots:
	void sltNavClicked();
	void sltSearVideo();
	void sltNameSortBtnClicked();
	void sltAllBtnClicked();
	void sltDevTreeRightClicked(const QPoint & point);
	void sltCoverDeviceGroup();
	void sltUpdateDeviceGroup();
	void sltSyncGetDeviceGroupResponse(int req, int code, QString data);
	void sltSyncSetDeviceGroupResponse(int req, int code, QString data);
	//void sltRemoveDevice(QString tid);
	//void sltVarUpdate(QString tid);
	void sltThingsNodeEvent(QString tid, int code);
	//void sltThingsEvent(int event);
	void sltDeviceGroupChanged();
	void slt2MinTimeout();
	void sltOnlineBtnClicked();
	void sltOfflineBtnClicked();
	void sltDevReboot();
	void sltDevOutput();
	void sltGroupAction();
	void sltAddHistoryItem(const QModelIndex&);
	void sltItemDoubleClicked(const QModelIndex& index);
	void sltDeviceRemotePlayParam(int req, int code, QString data);
	void sltRemoteVoicePlay();
	void sltDeviceRemotePlay(int req, int code, QString data);
signals:
	void sglItemDoubleClicked(DeviceTreeItem*);
	void sglDeviceGroupChanged();
};

//Q_DECLARE_METATYPE(DeviceTreeItemCount)
