#include "DeviceTreeModel.h"
#include <qdebug.h>
#include <QIcon>
#include "ClientApplication.h"

DeviceTreeModel::DeviceTreeModel(DeviceTreeItem* item, QObject* parent)
	: QAbstractItemModel(parent)
{
	m_pRootItem = new DeviceTreeItem(QString(""), nullptr);
}

DeviceTreeModel::~DeviceTreeModel()
{
	delete m_pRootItem;
}

int DeviceTreeModel::rowCount(const QModelIndex& parent) const
{
	DeviceTreeItem* parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid()) {
		parentItem = m_pRootItem;
	}
	else {
		parentItem = static_cast<DeviceTreeItem*>(parent.internalPointer());
	}
	return parentItem->childCount();
}

int DeviceTreeModel::columnCount(const QModelIndex& parent) const
{
	return 1;
}

DeviceTreeItem* DeviceTreeModel::itemFromIndex(const QModelIndex& index) const
{
	DeviceTreeItem* item;
	if (!index.isValid())
		item = m_pRootItem;
	else
		item = static_cast<DeviceTreeItem*>(index.internalPointer());
	return item;
}
QModelIndex DeviceTreeModel::indexFromItem(DeviceTreeItem* item) const
{
	return createIndex(item->row(), 0, item);
}
DeviceTreeItem* DeviceTreeModel::topLevelItem()
{
	return m_pRootItem;
}
//QModelIndexList DeviceTreeModel::match(const QModelIndex& start, int role, const QVariant& value, int hits, Qt::MatchFlags flags) const
//{
//	return QAbstractItemModel::match(start,role, value, hits, flags);
//}

QModelIndex DeviceTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();
	DeviceTreeItem* parentItem = itemFromIndex(parent);
	DeviceTreeItem* item = parentItem->child(row);
	if (item && !item->isHide())
		return createIndex(row, column, item);
	else
		return QModelIndex();
}

QModelIndex DeviceTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	DeviceTreeItem* item = itemFromIndex(index);
	DeviceTreeItem* parentItem = item->parentItem();
	if (!parentItem || parentItem == m_pRootItem)
		return QModelIndex();
	return createIndex(parentItem->row(), 0, parentItem);
}

bool DeviceTreeModel::insertRow(const QString& name, int type, DeviceTreeItem* parentItem)
{
	beginInsertRows(createIndex(parentItem->row(), 0, parentItem), parentItem->childCount(), parentItem->childCount());
	DeviceTreeItem* pDevItem = new DeviceTreeItem(name, parentItem);
	pDevItem->setRow(parentItem->childCount());
	pDevItem->setType(type);
	parentItem->appendChild(pDevItem);
	if (type == NODE_DEVICE) {
		pDevItem->updateChannel();
	}
	endInsertRows();
	return true;
}

bool DeviceTreeModel::insertDeviceChannel(DeviceTreeItem* parentItem)
{
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(parentItem->name());
	beginInsertRows(createIndex(parentItem->row(), 0, parentItem), 0, things.chList.size() -1);
	for each (auto chObj in things.chList) {
		DeviceTreeItem* pChannelItem = new DeviceTreeItem(chObj.channel, parentItem);
		pChannelItem->setType(NODE_CHANNEL);
		parentItem->appendChild(pChannelItem);
	}
	endInsertRows();
	return true;
}

bool DeviceTreeModel::removeRow(DeviceTreeItem* pItem)
{
	DeviceTreeItem* parentItem = pItem->parentItem();
	QModelIndex parentIndex = createIndex(parentItem->row(), 0, parentItem);
	beginRemoveRows(parentIndex, pItem->row(), pItem->row());
	DeviceTreeItem* childItem = parentItem->removeItem(pItem->row());
	delete childItem;
	childItem = nullptr;
	endRemoveRows();
	return true;
}

bool DeviceTreeModel::removeRowChildren(DeviceTreeItem* pItem)
{
	QModelIndex parentIndex = createIndex(pItem->row(), 0, pItem);
	beginRemoveRows(parentIndex, 0, pItem->childCount());
	pItem->removeChilds();
	endRemoveRows();
	return true;
}

//QList<DeviceTreeItem*> DeviceTreeModel::takeRowChildren(DeviceTreeItem* pItem)
//{
//	QModelIndex parentIndex = createIndex(pItem->row(), 0, pItem);
//	beginRemoveRows(parentIndex, 0, pItem->childCount());
//	pItem->removeChilds();
//	endRemoveRows();
//	return true;
//}

bool DeviceTreeModel::moveRows(DeviceTreeItem* srcItem, DeviceTreeItem* destItem)
{
	for (int i = srcItem->childCount() -1; i >= 0; i--) {
		DeviceTreeItem* childItem = srcItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			return moveRows(childItem, destItem);
		}
		else if (nodeType == NODE_DEVICE) {
			bool ret = beginMoveRows(indexFromItem(srcItem), childItem->row(), childItem->row(),
				indexFromItem(destItem), destItem->childCount());
			if (!ret) { return false; }
			srcItem->removeItem(childItem->row());
			childItem->setParentItem(destItem);
			destItem->appendChild(childItem);
			endMoveRows();
		}
	}
	return true;
}

QVariant DeviceTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}
	DeviceTreeItem* item = static_cast<DeviceTreeItem*>(index.internalPointer());

	if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
		//if (m_listUpdateTid.contains(item->name())) {
		//	item->updateChannel();
		//	QList<QString>* list = const_cast<QList<QString>*>(&m_listUpdateTid);
		//	list->removeOne(item->name());
		//}
		//qDebug() << "view" << item->row() << item->data();
		return item->data(index.column());
	}
	else if (role == Qt::DecorationRole) {
		QString name = item->getIconName();
		if (!name.isEmpty()) {
			return QIcon(name);
		}
	}
	else if (role == Qt::TextAlignmentRole) {
		return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else if (role == Qt::UserRole) {
		return item->m_Variant;
	}
	return QVariant();
}

bool DeviceTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	return false;
}

Qt::ItemFlags DeviceTreeModel::flags(const QModelIndex& index) const
{
	//return QAbstractItemModel::flags(index);
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	if (index.isValid())
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	else
		return Qt::ItemIsDropEnabled | defaultFlags;
}

//void DeviceTreeModel::setHorizontalHeader(const QStringList & headers)
//{
//}

Qt::DropActions DeviceTreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QStringList DeviceTreeModel::mimeTypes() const
{
	QStringList types;
	types << "deviceMimeData";
	return types;
}

void DeviceTreeModel::sortByName(DeviceTreeItem* parentItem, Qt::SortOrder order)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			sortByName(childItem, order);
			childItem->sortByName(order);
		}
	}
}

void DeviceTreeModel::sort(int column, Qt::SortOrder order)
{
	if (column != 0) return;
	qDebug() << "sort";
	// 关键：发送模型layout即将改变信号
	emit layoutAboutToBeChanged();
	sortByName(m_pRootItem, order);
	// 关键：发送模型layout已改变信号
	emit layoutChanged();
}

void DeviceTreeModel::filter(QString& text, DeviceTreeItem* parentItem)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			childItem->filter(text);
			filter(text, childItem);
		}
		else if (nodeType == NODE_DEVICE) {
			childItem->filter(text);
		}
	}
}

void DeviceTreeModel::filter(bool online, DeviceTreeItem* parentItem)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			filter(online, childItem);
		}
		else if (nodeType == NODE_DEVICE) {
			childItem->filter(online);
		}
	}
}

QMimeData* DeviceTreeModel::mimeData(const QModelIndexList& indexes) const
{
	QMimeData* mimeData = QAbstractItemModel::mimeData(indexes);
	for (int i = 0; i < indexes.count(); i++){
		QModelIndex index = indexes[i];
		QModelIndex* p = new QModelIndex(index);
		DeviceTreeItem* item = static_cast<DeviceTreeItem*>(index.internalPointer());
		//qDebug() << item->data(0).toString();
		QByteArray array;
		QDataStream stream(&array, QIODevice::WriteOnly);
		stream << (qint64)p;
		mimeData->setData(QString("deviceMimeData"), array);
		return mimeData;
	}
	return mimeData;
}

bool DeviceTreeModel::canDropMimeData(const QMimeData* data,
	Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(parent);
	if (!data->hasFormat("deviceMimeData"))
		return false;
	if (column > 0)
		return false;
	DeviceTreeItem* parentItem = itemFromIndex(parent);
	QByteArray array = data->data(QString("deviceMimeData"));
	QDataStream stream(&array, QIODevice::ReadOnly);
	qint64 p;
	stream >> p;
	QModelIndex* index = (QModelIndex*)p;
	DeviceTreeItem* item = static_cast<DeviceTreeItem*>(index->internalPointer());
	//qDebug() << "canDropMimeData" << row << action << parent.row() << parentItem->type();
	if (parentItem->type() != NODE_GROUP || item->parentItem() == parentItem) {
		return false;
	}
	return true;
}

bool DeviceTreeModel::dropMimeData(const QMimeData* data,
	Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	qDebug() << "dropMimeData" << row << column << parent.row();
	if (!canDropMimeData(data, action, row, column, parent))
		return false;
	if (action == Qt::IgnoreAction)
		return true;
	QByteArray array = data->data(QString("deviceMimeData"));
	QDataStream stream(&array, QIODevice::ReadOnly);
	qint64 p;
	stream >> p;
	QModelIndex* index = (QModelIndex*)p;
	DeviceTreeItem* item = static_cast<DeviceTreeItem*>(index->internalPointer());
	DeviceTreeItem* ParentItem = static_cast<DeviceTreeItem*>(parent.internalPointer());
	beginMoveRows(index->parent(), index->row(), index->row(), parent, ParentItem->childCount());
	item->parentItem()->removeItem(item->row());
	item->setParentItem(ParentItem);
	ParentItem->appendChild(item);
	endMoveRows();
	delete index;
	return true;
}
