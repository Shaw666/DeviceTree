#include "DeviceTreeView.h"
#include <QMimeData>
#include <qscrollbar.h>
#include <qdebug.h>

DeviceTreeView::DeviceTreeView(QWidget *parent)
	: QTreeView(parent)
{
}

DeviceTreeView::~DeviceTreeView()
{
}

void DeviceTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	const QMimeData* pMimeData = event->mimeData();
	event->accept();
	//qDebug() << "dragMoveEvent" << event->pos() << this->height();
	int yPos = event->pos().ry();
	int v = verticalScrollBar()->value();
	if (yPos <= 30) {
		v -= 1;
	}
	else if (yPos >= (this->height() - 30)) {
		v += 1;
	}
	verticalScrollBar()->setValue(v);
	//qDebug() << v;
}
