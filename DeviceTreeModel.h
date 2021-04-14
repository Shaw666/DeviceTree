#pragma once

#include <QObject>
#include <QAbstractItemModel>
#include "DeviceTreeItem.h"
#include <QMimeData>

class DeviceTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	DeviceTreeModel(DeviceTreeItem* item, QObject* parent);
	~DeviceTreeModel();
	int rowCount(const QModelIndex& parent) const;
	int columnCount(const QModelIndex& parent) const;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex& index) const;
	bool insertRow(const QString& name, int type, DeviceTreeItem* parentItem);
	bool insertDeviceChannel(DeviceTreeItem* parentItem);
	//bool insertRow(const QString& tid, DeviceTreeItem* parentItem);
	//bool insertRows(const QString& tid, DeviceTreeItem* parentItem);
	//bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
	//bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
	bool removeRow(DeviceTreeItem* pItem);
	bool removeRowChildren(DeviceTreeItem* pItem);
	//QList<DeviceTreeItem*> takeRowChildren(DeviceTreeItem* pItem);
	bool moveRows(DeviceTreeItem* srcItem, DeviceTreeItem* destItem);
	QVariant data(const QModelIndex& index, int role) const;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	Qt::ItemFlags flags(const QModelIndex& index) const;
	Qt::DropActions supportedDropActions() const;
	QStringList mimeTypes() const;
	QMimeData* mimeData(const QModelIndexList& indexes) const;
	bool canDropMimeData(const QMimeData* data,
		Qt::DropAction action, int row, int column, const QModelIndex& parent);
	bool dropMimeData(const QMimeData* data,
		Qt::DropAction action, int row, int column, const QModelIndex& parent);
	//void setHorizontalHeader(const QStringList& headers);
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
	void filter(QString& text, DeviceTreeItem* parentItem);
	void filter(bool online, DeviceTreeItem* parentItem);
	DeviceTreeItem* itemFromIndex(const QModelIndex& index) const;
	QModelIndex indexFromItem(DeviceTreeItem* item) const;
	DeviceTreeItem* topLevelItem();

	//QModelIndexList match(const QModelIndex& start, int role, const QVariant& value,
		//int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;
	//QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	//bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
public:
	//QList<QString> m_listUpdateTid;
	//QList<QString> m_listDeleteTid;
private:
	void sortByName(DeviceTreeItem* parentItem, Qt::SortOrder order);
private:
	DeviceTreeItem* m_pRootItem;

};
