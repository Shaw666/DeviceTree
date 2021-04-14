#pragma once

#include <QTreeView>
#include <QDragMoveEvent>

class DeviceTreeView : public QTreeView
{
	Q_OBJECT

public:
	DeviceTreeView(QWidget *parent);
	~DeviceTreeView();
protected:
	void dragMoveEvent(QDragMoveEvent* event);
};
