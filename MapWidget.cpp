#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QVBoxLayout>
#include "MapWidget.h"

MapWidget::MapWidget()
{
    // 初始化 QGraphicsView 和 QGraphicsScene
    mapCanvas = new MapCanvas(this);
    scene = new QGraphicsScene(this);
    mapCanvas->setScene(scene);
    mapCanvas->setRenderHint(QPainter::Antialiasing);

    // 初始化 QLabel
    zoomLabel = new QLabel("缩放比例: 100%", this);
    zoomLabel->setStyleSheet("background-color: rgba(255, 255, 255, 200);"
        "border: 1px solid black;"
        "padding: 2px;");
    zoomLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);


    // 创建布局并将 mapCanvas 添加到布局中
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距
    layout->addWidget(mapCanvas);

    // 设置滚轮缩放的锚点为视图中心
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // 设置布局到 MapWidget
    setLayout(layout);

    if (!scene || !mapCanvas) {
        qDebug() << "场景或画布初始化失败";
    }
    else {
        qDebug() << "场景和画布初始化成功";
    }
    //---------------------------------------------以下为信号槽连接------------------------------------------------//

    connect(mapCanvas, &MapCanvas::zoomChanged, this, &MapWidget::updateZoomLabel);

}

QImage MapWidget::loadRaster(const QString& filePath)
{
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly);
    if (!dataset) qDebug()<<"GDAL打开失败："<<filePath;

    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();
    int bandCount = dataset->GetRasterCount();
    if (bandCount < 1) {
        GDALClose(dataset);
        qDebug() << "GDAL打开失败：" << filePath;
    }

    if (width <= 0 || height <= 0) {
        QMessageBox::critical(this, "错误", "无效的图像尺寸: " + QString::number(width) + "x" + QString::number(height));
        GDALClose(dataset);
        qDebug() << "GDAL打开失败：" << filePath;
    }

    QImage image;
    bool processedAsRGB = false;


    if (!scene) {
        scene = new QGraphicsScene(this);
        mapCanvas->setScene(scene);
    }
    //显示前清除旧内容
    //scene->clear();


    // 处理RGB三波段
    if (bandCount >= 3) {
        GDALRasterBand* band1 = dataset->GetRasterBand(1);
        GDALRasterBand* band2 = dataset->GetRasterBand(2);
        GDALRasterBand* band3 = dataset->GetRasterBand(3);

        GDALDataType type1 = band1->GetRasterDataType();
        GDALDataType type2 = band2->GetRasterDataType();
        GDALDataType type3 = band3->GetRasterDataType();

        if (type1 == type2 && type1 == type3) {
            if (type1 == GDT_Byte) { // 8位RGB处理
                image = QImage(width, height, QImage::Format_RGB888);
                std::vector<uchar> bufferR(width * height);
                std::vector<uchar> bufferG(width * height);
                std::vector<uchar> bufferB(width * height);

                if (band1->RasterIO(GF_Read, 0, 0, width, height, bufferR.data(), width, height, GDT_Byte, 0, 0) == CE_None &&
                    band2->RasterIO(GF_Read, 0, 0, width, height, bufferG.data(), width, height, GDT_Byte, 0, 0) == CE_None &&
                    band3->RasterIO(GF_Read, 0, 0, width, height, bufferB.data(), width, height, GDT_Byte, 0, 0) == CE_None) {

                    uchar* imgBits = image.bits();
                    for (int i = 0; i < width * height; ++i) {
                        imgBits[i * 3] = bufferR[i]; // Red
                        imgBits[i * 3 + 1] = bufferG[i]; // Green
                        imgBits[i * 3 + 2] = bufferB[i]; // Blue
                    }
                    processedAsRGB = true;
                }
            }
            else if (type1 == GDT_UInt16) { // 16位RGB处理
                image = QImage(width, height, QImage::Format_RGB888);
                std::vector<uint16_t> bufferR(width * height);
                std::vector<uint16_t> bufferG(width * height);
                std::vector<uint16_t> bufferB(width * height);

                if (band1->RasterIO(GF_Read, 0, 0, width, height, bufferR.data(), width, height, GDT_UInt16, 0, 0) == CE_None &&
                    band2->RasterIO(GF_Read, 0, 0, width, height, bufferG.data(), width, height, GDT_UInt16, 0, 0) == CE_None &&
                    band3->RasterIO(GF_Read, 0, 0, width, height, bufferB.data(), width, height, GDT_UInt16, 0, 0) == CE_None) {

                    uchar* imgBits = image.bits();
                    for (int i = 0; i < width * height; ++i) {
                        imgBits[i * 3] = static_cast<uchar>(bufferR[i] / 256); // 高位转8位
                        imgBits[i * 3 + 1] = static_cast<uchar>(bufferG[i] / 256);
                        imgBits[i * 3 + 2] = static_cast<uchar>(bufferB[i] / 256);
                    }
                    processedAsRGB = true;
                }
            }
        }
    }

    // 处理单波段灰度
    if (!processedAsRGB) {
        GDALRasterBand* band = dataset->GetRasterBand(1);
        GDALDataType dataType = band->GetRasterDataType();

        if (dataType == GDT_Byte) { // 8位灰度
            image = QImage(width, height, QImage::Format_Grayscale8);
            if (band->RasterIO(GF_Read, 0, 0, width, height, image.bits(), width, height, GDT_Byte, 0, 0) != CE_None) {
                GDALClose(dataset);
                qDebug() << "GDAL打开失败：" << filePath;
            }
        }
        else if (dataType == GDT_UInt16) { // 16位灰度归一化
            std::vector<uint16_t> buffer(width * height);
            if (band->RasterIO(GF_Read, 0, 0, width, height, buffer.data(), width, height, GDT_UInt16, 0, 0) != CE_None) {
                GDALClose(dataset);
                qDebug() << "GDAL打开失败：" << filePath;
            }
            image = QImage(width, height, QImage::Format_Grayscale8);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uint16_t value = buffer[y * width + x];
                    image.setPixel(x, y, qRgb(value / 256, value / 256, value / 256));
                }
            }
        }
        else {
            GDALClose(dataset);
            qDebug() << "GDAL打开失败：" << filePath;
        }
    }
    if (image.isNull()) {
        qDebug() << "图像加载失败：" << filePath;
    }
    else {
        qDebug() << "图像加载成功：" << filePath;
    }

    return image;
}


QGraphicsItem* MapWidget::createGraphicsItem(OGRGeometry* geom, const OGREnvelope& env) {
    QPainterPath path;
    switch (wkbFlatten(geom->getGeometryType())) {
    case wkbPoint: {
        OGRPoint* p = static_cast<OGRPoint*>(geom);
        QPointF pt = mapToView(p->getX(), p->getY(), env);
        auto* item = new QGraphicsEllipseItem(pt.x() - 2, pt.y() - 2, 4, 4);
        item->setBrush(Qt::red);
        return item;
    }
    case wkbLineString: {
        OGRLineString* line = static_cast<OGRLineString*>(geom);
        for (int i = 0; i < line->getNumPoints(); ++i) {
            QPointF pt = mapToView(line->getX(i), line->getY(i), env);
            (i == 0) ? path.moveTo(pt) : path.lineTo(pt);
        }
        auto* item = new QGraphicsPathItem(path);
        item->setPen(QPen(Qt::blue, 1));
        return item;
    }
    case wkbPolygon: {
        OGRPolygon* poly = static_cast<OGRPolygon*>(geom);
        OGRLinearRing* ring = poly->getExteriorRing();
        QPolygonF qpoly;
        for (int i = 0; i < ring->getNumPoints(); ++i) {
            qpoly << mapToView(ring->getX(i), ring->getY(i), env);
        }
        auto* item = new QGraphicsPolygonItem(qpoly);
        item->setBrush(QColor(0, 255, 0, 50));
        item->setPen(QPen(Qt::darkGreen, 1));
        return item;
    }
    default:
        return nullptr;
    }
}

QPointF MapWidget::mapToView(double x, double y, const OGREnvelope& env) {
    double xRatio = (x - env.MinX) / (env.MaxX - env.MinX);
    double yRatio = 1.0 - (y - env.MinY) / (env.MaxY - env.MinY); // Y轴翻转
    return QPointF(
        xRatio * mapCanvas->viewport()->width(),
        yRatio * mapCanvas->viewport()->height()
    );
}


void MapWidget::updateFilePathList(const QMap<QString, bool>& fileList) {
    filePathList = fileList; // 更新文件路径和状态的映射
    scene->clear();
    // 遍历 QMap，检查所有路径的状态
    for (auto it = filePathList.begin(); it != filePathList.end(); ++it) {
        const QString& filePath = it.key();
        bool status = it.value();

        // 判断文件类型
        QFileInfo fileInfo(filePath);
        QString fileExtension = fileInfo.suffix().toLower();
        if (fileExtension == "tif" || fileExtension == "tiff") {
            if (status) {
                // 如果状态为 true，加载图像并显示
                qDebug() << "File is visible: " << filePath;

                QPixmap pixmap = QPixmap::fromImage(loadRaster(filePath));

                //检查图像是否被添加到场景
                if (pixmap.isNull()) {
                    qDebug() << "从图像生成 Pixmap 失败：" << filePath;
                }
                else {
                    QGraphicsPixmapItem* pixmapItem = scene->addPixmap(pixmap);
                    if (!pixmapItem) {
                        qDebug() << "图像未成功添加到场景：" << filePath;
                    }
                    else {
                        pixmapItem->setData(0, filePath);
                        qDebug() << "图像已成功添加到场景：" << filePath;
                    }
                }

                QGraphicsPixmapItem* pixmapItem = scene->addPixmap(pixmap);
                pixmapItem->setData(0, filePath); // 将文件路径存储为图像项的用户数据
                mapCanvas->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
            }
            else {
                // 如果状态为 false
                qDebug() << "File is hidden: " << filePath;
            }
        }
        else if (fileExtension == "shp") {
            if (status) {
                GDALDataset* poDS = (GDALDataset*)GDALOpenEx(
                    filePath.toUtf8(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
                if (!poDS) {
                    qDebug() << "打开SHP文件失败";
                    return;
                }

                OGRLayer* poLayer = poDS->GetLayer(0);
                if (!poLayer) {
                    GDALClose(poDS);
                    return;
                }

                // 获取全局坐标范围
                poLayer->GetExtent(&envTotal);

                // 遍历要素
                OGRFeature* poFeature;
                while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
                    OGRGeometry* poGeometry = poFeature->GetGeometryRef();
                    if (!poGeometry) continue;

                    QGraphicsItem* item = createGraphicsItem(poGeometry, envTotal);
                    if (item) scene->addItem(item);

                    OGRFeature::DestroyFeature(poFeature);
                }

                GDALClose(poDS);
                mapCanvas->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);  // 自动缩放
            }
            else {
                // 如果状态为 false
                qDebug() << "File is hidden: " << filePath;
            }
        }
    }
}

void MapWidget::updateZoomLabel(qreal scale) {
    int percentage = static_cast<int>(scale * 100);
    zoomLabel->setText(QString("缩放比例: %1%").arg(percentage));
}