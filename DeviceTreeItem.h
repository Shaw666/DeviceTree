#pragma once

#include <QObject>
#include <QVariant>
#include "tparser.h"
#include "MatrixLibDef.h"

enum NODE_TYPE
{
	NODE_UNKNOWN = 0,
	NODE_CHANNEL,
	NODE_DEVICE,
	NODE_GROUP,
	NODE_DEVICE_MATRIX,
	NODE_CHANNEL_MATRIX
};

struct DeviceTreeCount
{
	int all;
	int online;
};

class DeviceTreeItem : public QObject
{
	Q_OBJECT

public:
	//DeviceTreeItem(QString& name, DeviceTreeItem* parent = nullptr);
	//DeviceTreeItem(void* data, DeviceTreeItem* parent);
	DeviceTreeItem(QVariant data, DeviceTreeItem* parent);
	~DeviceTreeItem();
	//它应该有个正确的父亲才可以正确的执行该函数
	void appendChild(DeviceTreeItem* child);      //在本节点下增加子节点
	void updateChannel();
	void removeChilds();                    //清空所有节点
	DeviceTreeItem* removeItem(int row);
	DeviceTreeItem* child(int row);               //获取第row个子节点指针
	DeviceTreeItem* parentItem();                 //获取父节点指针
	void setParentItem(DeviceTreeItem* parent);
	int childCount() const;                 //子节点计数
	int row() const;                        //获取该节点是父节点的第几个子节点
											//核心函数：获取节点第column列的数据
	QVariant data(int column = 0) const;

	QString getIconName();
	void sortByName(Qt::SortOrder order = Qt::AscendingOrder);
	void filter(QString& text);
	void filter(bool online);
	//设置、获取节点是几级节点（就是树的层级）
	int level() { return m_nLevel; }
	void setLevel(int level) { m_nLevel = level; }

	//设置、获取节点存的数据指针
	//void setPtr(void* p) { m_pPtr = p; }
	//void* ptr() { return m_pPtr; }
	QString name() { return m_Variant.toString(); };
	int channel() { return m_Variant.toInt(); };
	bool isHide() { return m_bHide; }
	bool isPlaying() { return m_bPlaying; }
	void setHide(bool b) { m_bHide = b; }
	void setName(QString& name) { m_Variant.setValue(name); };
	//保存该节点是其父节点的第几个子节点，查询优化所用
	void setRow(int row) {
		m_nRow = row;
	}
	int type() { return m_nType; }
	void setType(int type) { m_nType = type; }
	bool operator<(DeviceTreeItem* other) const;
	void setPlaying(bool b) { m_bPlaying = b; };
public:
	QList<DeviceTreeItem*> m_listChildItem;   //子节点
	QVariant m_Variant;
protected:

private:
	QString getDeviceIconName(bool playing = false);
	QString getIconName(QString& model, bool online, bool playing);
	QString getChannelIconName();
private:
	//QMap<QString, DeviceTreeItem*> m_mapChildItem;   //子节点
	DeviceTreeItem* m_pParentItem;          //父节点
	int m_nLevel;     //2设备 3通道
	//void* m_pPtr;     //存储数据的指针
	int m_nRow;       //记录该item是第几个，可优化查询效率
	int m_nType;
	bool m_bPlaying;
	bool m_bHide;
	//QString m_sName;
};
