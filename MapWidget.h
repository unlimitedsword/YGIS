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
#include "Public.h"


class MapWidget:public QGraphicsView {
	Q_OBJECT

public:
	MapWidget();

	void bufferVector(const QString& inputPath,double radius);

	bool createBuffer(const QString& inputPath, const QString& outputPath, double bufferRadius);

public slots:

	void updateFilePathList(const QMap<QString, T_Information>& fileList);

	void updateZoomLabel(qreal scale); // 更新缩放比例标签

signals:
	void bufferCompleted(const QString& filePath);

private:
	QImage loadRaster(const QString& filePath);

	QGraphicsItem* createGraphicsItem(OGRGeometry* geom, const OGREnvelope& env,QColor color);
	QPointF mapToView(double x, double y, const OGREnvelope& env);

	MapCanvas* m_mapCanvas;
	QGraphicsScene* m_scene;       // 图形场景对象
	QLabel* m_zoomLabel; // 用于显示缩放比例的标签
	OGREnvelope envTotal;
	QMap<QString, T_Information> m_filePathList; // 文件路径对应状态

};
