#include "DeviceTreeItem.h"
#include <qdebug.h>
//#include <QDir>
#include <QtAlgorithms>
#include <Shlwapi.h>
#include "ClientApplication.h"

#pragma comment(lib,"shlwapi.lib")

DeviceTreeItem::DeviceTreeItem(QVariant data, DeviceTreeItem* parent)
	: QObject()
{
	m_bPlaying = false;
	m_bHide = false;
	m_nType = 0;
	if (parent == nullptr) {
		m_nLevel = 0;
	}
	else {
		m_nLevel = parent->level() + 1;
	}
	//m_pPtr = data;
	//m_ItemData = data;
	m_pParentItem = parent;
	m_listChildItem.clear();
	m_nRow = 0;
	m_Variant = data;
}

DeviceTreeItem::~DeviceTreeItem()
{
	//qDebug() << "delete tree item: " << data(0) << m_listChildItem.size() << m_nRow;
	if (m_listChildItem.size() > 0) {
		//qDebug() << "delete m_listChildItem" << data(0) << m_listChildItem.size();
		removeChilds();
	}

}

void DeviceTreeItem::appendChild(DeviceTreeItem* child)
{
	child->setRow(m_listChildItem.size());
	m_listChildItem.append(child);
	//m_mapChildItem.insert()
}

void DeviceTreeItem::updateChannel()
{
	removeChilds();
	if (m_nType != NODE_DEVICE)return;
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(m_Variant.toString());
	for each (auto chObj in things.chList) {
		DeviceTreeItem* pChannelItem = new DeviceTreeItem(chObj.channel, this);
		pChannelItem->setType(NODE_CHANNEL);
		this->appendChild(pChannelItem);
	}
}

void DeviceTreeItem::removeChilds()
{
	while (childCount() > 0) {
		DeviceTreeItem* childItem = m_listChildItem.takeAt(0);
		if (childItem != nullptr) {
			delete childItem;
		}
	}
}

DeviceTreeItem* DeviceTreeItem::removeItem(int row)
{
	qDebug() << "removeItem" << data() << m_listChildItem .size()<< row;
	if (row < 0 && row >= m_listChildItem.size()) {
		return nullptr;
	}
	DeviceTreeItem* item = m_listChildItem.at(row);
	m_listChildItem.removeAt(row);
	for (int i = row; i < m_listChildItem.size(); i++) {
		DeviceTreeItem* childItem = m_listChildItem.at(i);
		childItem->setRow(i);
	}
	return item;
}

DeviceTreeItem* DeviceTreeItem::child(int row)
{
	if (row < 0 && row >= m_listChildItem.size()) {
		return nullptr;
	}
	return m_listChildItem.at(row);
}

DeviceTreeItem* DeviceTreeItem::parentItem()
{
	return m_pParentItem;
}

void DeviceTreeItem::setParentItem(DeviceTreeItem* parent)
{
	m_pParentItem = parent;
}

int DeviceTreeItem::childCount() const
{
	return m_listChildItem.size();
}

int DeviceTreeItem::row() const
{
	return m_nRow;
}

void getDeviceCount(DeviceTreeItem* parentItem, DeviceTreeCount& count)
{
	for (int i = 0; i < parentItem->childCount(); i++) {
		DeviceTreeItem* childItem = parentItem->child(i);
		int nodeType = childItem->type();
		if (nodeType == NODE_GROUP) {
			getDeviceCount(childItem, count);
		}
		else if (nodeType == NODE_DEVICE && !childItem->isHide()) {
			if (jyApp->getThings()->getThingsObj(childItem->name()).online) {
				count.online++;
			}
			count.all++;
		}
	}
}

QVariant DeviceTreeItem::data(int column) const
{
	if ((m_nLevel < 1) || column > 0) {
		return QVariant();
	}
	QString name = m_Variant.toString();
	if (m_nType == NODE_DEVICE) {
		name = jyApp->getThings()->getThingsObj(m_Variant.toString()).name;
	}
	else if (m_nType == NODE_CHANNEL) {
		name = QString("%1%2").arg(tr("Channel")).arg(m_Variant.toInt());
	}
	else if (m_nType == NODE_GROUP) {
		DeviceTreeCount count{0};
		getDeviceCount((DeviceTreeItem*)this, count);
		name += QString(" (%1/%2)").arg(count.online).arg(count.all);
	}
	else if (m_nType == NODE_DEVICE_MATRIX) {
		name = m_Variant.value<tagEcoderInfo>().devName;
	}
	else if (m_nType == NODE_CHANNEL_MATRIX) {
		name = m_Variant.value<CHANNEL_OBJ>().channelName;
	}
	QVariant var;
	var.setValue(name);
	return var;
}

QString DeviceTreeItem::getIconName()
{
	QString name = "";
	if (m_nType == NODE_DEVICE) {
		name = getDeviceIconName(m_bPlaying);
	}
	else if (m_nType == NODE_CHANNEL) {
		name = getChannelIconName();
	}
	else if (m_nType == NODE_DEVICE_MATRIX) {
		name = getIconName(QString(""), m_Variant.value<tagEcoderInfo>().isOnline, false);
	}
	else if (m_nType == NODE_CHANNEL_MATRIX) {
		name = "../Common/Skin/video/deviceTree/ch_online.png";
	}
	return name;
}

int GetCharType(const WCHAR ch)
{
	// ASIIC码值 > 127 定义为中文字符
	if (ch >= '0' && ch <= '9') {
		return 0;
	}
	else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
		return 1;
	}
	else if ((ch > 127)) {
		return 2;
	}
	return 3;   // 其他字符
}

bool DeviceTreeItem::operator<(DeviceTreeItem* other) const
{
	int thisNodeType = this->m_nType;
	int otherNodeType = other->m_nType;
	QString thisStr = this->data().toString();
	QString otherStr = other->data().toString();
	if (thisStr.isEmpty() || thisNodeType == NODE_CHANNEL || otherNodeType == NODE_CHANNEL || thisStr == otherStr || otherStr.isEmpty()) {
		return false;
	}
	//if (otherStr.isEmpty()) {
	//	return true;
	//}
	const std::string name1 = thisStr.toStdString();
	const std::string name2 = otherStr.toStdString();
	WCHAR ch1 = name1.at(0);
	WCHAR ch2 = name2.at(0);
	int nType = GetCharType(ch1) - GetCharType(ch2);
	//数字 < 字母 < 汉字 <特殊字符
	if (0 != nType) {
		//qDebug() << (nType < 0);
		return (nType < 0);
	}
	const wchar_t* wstrText = reinterpret_cast<const wchar_t*>(thisStr.utf16());
	const wchar_t* wstrOtherText = reinterpret_cast<const wchar_t*>(otherStr.utf16());
	int result = StrCmpLogicalW((wchar_t*)wstrText, (wchar_t*)wstrOtherText);
	//qDebug() << (result == -1);
	return (result == -1);
}

static bool compareByName(DeviceTreeItem* srcA, DeviceTreeItem* srcB)
{
	//return (srcA < srcB);
	int thisNodeType = srcA->type();
	int otherNodeType = srcB->type();
	QString thisStr = srcA->data().toString();
	QString otherStr = srcB->data().toString();
	if (thisStr.isEmpty() || thisNodeType == NODE_CHANNEL || otherNodeType == NODE_CHANNEL || thisStr == otherStr || otherStr.isEmpty()) {
		return false;
	}
	//if (otherStr.isEmpty()) {
	//	return true;
	//}
	const std::string name1 = thisStr.toStdString();
	const std::string name2 = otherStr.toStdString();
	WCHAR ch1 = name1.at(0);
	WCHAR ch2 = name2.at(0);
	int nType = GetCharType(ch1) - GetCharType(ch2);
	//数字 < 字母 < 汉字 <特殊字符
	if (0 != nType) {
		//qDebug() << (nType < 0);
		return (nType < 0);
	}
	const wchar_t* wstrText = reinterpret_cast<const wchar_t*>(thisStr.utf16());
	const wchar_t* wstrOtherText = reinterpret_cast<const wchar_t*>(otherStr.utf16());
	int result = StrCmpLogicalW((wchar_t*)wstrText, (wchar_t*)wstrOtherText);
	//qDebug() << (result == -1);
	return (result == -1);
}

void DeviceTreeItem::sortByName(Qt::SortOrder order)
{
	if (order == Qt::AscendingOrder) {
		qSort(m_listChildItem.begin(), m_listChildItem.end(), compareByName);
	}
	else {
		qSort(m_listChildItem.rbegin(), m_listChildItem.rend(), compareByName);
	}
	for (int i = 0; i < m_listChildItem.size(); i++) {
		DeviceTreeItem* childItem = m_listChildItem.at(i);
		childItem->setRow(i);
	}
}

void DeviceTreeItem::filter(QString& text)
{
	if (data().toString().contains(text)) {
		m_bHide = false;
		DeviceTreeItem* parentItem = this->parentItem();
		while (parentItem) {
			parentItem->setHide(false);
			parentItem = parentItem->parentItem();
		}
	}
	else {
		m_bHide = true;
	}
	for (int i = 0; i < m_listChildItem.size(); i++) {
		DeviceTreeItem* childItem = m_listChildItem.at(i);
		childItem->setHide(m_bHide);
	}
}

void DeviceTreeItem::filter(bool online)
{
	if (m_nType != NODE_DEVICE)return;
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(m_Variant.toString());
	if (things.online == online) {
		m_bHide = false;
		DeviceTreeItem* parentItem = this->parentItem();
		while (parentItem) {
			parentItem->setHide(false);
			parentItem = parentItem->parentItem();
		}
	}
	else {
		m_bHide = true;
	}
	for (int i = 0; i < m_listChildItem.size(); i++) {
		DeviceTreeItem* childItem = m_listChildItem.at(i);
		childItem->setHide(m_bHide);
	}
}

QString DeviceTreeItem::getDeviceIconName(bool playing)
{
	//TObject* tModel = ((TObject*)m_pPtr)->member("session.1.runtime.profile.model");
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(m_Variant.toString());
	//QString model = things.model;/*tModel ? tModel->to_string() : "";*/
	//TObject* tOnline = ((TObject*)m_pPtr)->member("online");
	//bool online = ;/*tOnline ? tOnline->to_int() : false;*/
	//model = model.toUpper();
	return getIconName(things.model.toUpper(), things.online, playing);
}

QString DeviceTreeItem::getIconName(QString& model, bool online, bool playing)
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

QString DeviceTreeItem::getChannelIconName()
{
	QString iconName = "ch_";
	//QString path = QString("session.1.runtime.devs.videos.ch%1").arg(m_sName.toInt());
	//TObject* tChannel = ((TObject*)m_pPtr)->member(path.toStdString().c_str());
	//if (!tChannel) {
	//	path = QString("session.1.runtime.devs.audios.ch%1").arg(m_sName.toInt());
	//	tChannel = ((TObject*)m_pPtr)->member(path.toStdString().c_str());
	//}
	THINGS_OBJ things = jyApp->getThings()->getThingsObj(this->parentItem()->name());
	bool online = false;
	bool enable = false;
	for each (auto chObj in things.chList){
		if (chObj.channel == m_Variant.toInt()) {
			online = chObj.online;
			enable = chObj.enable;
			break;
		}
	}
	//if (tChannel) {
	//	TObject* tChannelOnline = tChannel->member("online");
	//	TObject* tChannelEnable = tChannel->member("enable");
	//	online = tChannelOnline ? tChannelOnline->to_int() : true;
	//	enable = tChannelEnable ? tChannelEnable->to_int() : true;
	//}
	if (online && enable) {
		iconName += "online.png";
	}
	else {
		iconName += "offline.png";
	}
	iconName = "../Common/Skin/video/deviceTree/" + iconName;
	return iconName;
}
