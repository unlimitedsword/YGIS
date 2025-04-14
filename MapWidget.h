#pragma once
#include <Qmainwindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QMap>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include "ogrsf_frmts.h"
#include "MapCanvas.h"

class MapWidget:public QGraphicsView {
	Q_OBJECT

public:
	MapWidget();

public slots:

	void updateFilePathList(const QMap<QString, bool>& fileList);

	void updateZoomLabel(qreal scale); // 更新缩放比例标签



private:
	QImage loadRaster(const QString& filePath);

	QGraphicsItem* createGraphicsItem(OGRGeometry* geom, const OGREnvelope& env);
	QPointF mapToView(double x, double y, const OGREnvelope& env);

	MapCanvas* mapCanvas;
	QGraphicsScene* scene;       // 图形场景对象
	QLabel* zoomLabel; // 用于显示缩放比例的标签
	OGREnvelope envTotal;
	QMap<QString, bool> filePathList; // 文件路径对应状态

};
