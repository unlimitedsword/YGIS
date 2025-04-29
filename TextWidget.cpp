// TextWidget.cpp
#include "TextWidget.h"
#include <QVBoxLayout>
#include <QFileInfo>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <cpl_string.h>

TextWidget::TextWidget(QWidget* parent)
    : QDockWidget("文本编辑器", parent) // 设置停靠窗口标题
{
    // 创建容器控件
    m_container = new QWidget(this);

    // 创建文本编辑器
    m_textEdit = new QTextEdit(m_container);
    m_textEdit->setReadOnly(true); // 允许编辑
    //textEdit->setStyleSheet("background: yellow;");//背景显示为黄色，测试用

    // 设置容器布局
    QVBoxLayout* layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0); // 去除边距
    layout->addWidget(m_textEdit);

    // 将容器设置为停靠窗口的内容
    this->setWidget(m_container);

    // 设置停靠区域
    this->setAllowedAreas(Qt::BottomDockWidgetArea);
}

void TextWidget::dataPathReceived(QString datapath) {
    // 判断文件类型
    QFileInfo fileInfo(datapath);
    QString fileExtension = fileInfo.suffix().toLower();

    if (fileExtension == "tif") {

        std::string s_rasterData = datapath.toStdString();
        const char* c_rasterData = s_rasterData.c_str();

        m_textEdit->append("文件类型: 栅格文件 (.tif)\n文件路径: " + datapath);
        //GDAL读取数据
        GDALDataset* rasterDataset = (GDALDataset*)GDALOpen(c_rasterData, GA_ReadOnly);

        int width = rasterDataset->GetRasterXSize();
        QString rasterXsize = QString::number(width);

        int length = rasterDataset->GetRasterYSize();
        QString rasterYsize = QString::number(length);

        int rasterCount = rasterDataset->GetRasterCount();
        QString rasterNum = QString::number(rasterCount);

        GDALRasterBand* band = rasterDataset->GetRasterBand(1);

        QString dataType = QString::fromUtf8(GDALGetDataTypeName(band->GetRasterDataType()));

        m_textEdit->append("宽度: " + rasterYsize);
        m_textEdit->append("高度: " + rasterYsize);
        m_textEdit->append("图层数量: " + rasterNum);
        m_textEdit->append("数据类型: " + dataType);

    }
    else if (fileExtension == "shp") {
        m_textEdit->setText("文件类型: 矢量文件 (.shp)\n文件路径: " + datapath);
        std::string s_vectorData = datapath.toStdString();
        const char* c_vectorData = s_vectorData.c_str();

        GDALDataset* vectorDataset = (GDALDataset*)GDALOpenEx(c_vectorData, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
        if (!vectorDataset) {
            qDebug() << "GDAL 打开失败，文件路径：" << datapath;
            return;
        }

        int layerCount = vectorDataset->GetLayerCount();
        QString layerNum = QString::number(layerCount);

        m_textEdit->append("图层数: " + layerNum);

        // 获取投影信息
        OGRLayer* layer = vectorDataset->GetLayer(0); // 获取第一个图层
        if (layer) {
            OGRSpatialReference* spatialRef = layer->GetSpatialRef();
            if (spatialRef) {
                char* projWkt = nullptr;
                spatialRef->exportToWkt(&projWkt); // 导出为WKT格式
                m_textEdit->append("投影信息 (WKT):");
                m_textEdit->append(QString::fromUtf8(projWkt));
                CPLFree(projWkt); // 释放内存
            }
            else {
                m_textEdit->append("投影信息: 无");
            }
        }
        else {
            m_textEdit->append("无法获取图层信息");
        }

        GDALClose(vectorDataset); // 关闭数据集
    }
    else {
        m_textEdit->setText("文件类型: 未知文件类型\n文件路径: " + datapath);
    }
}