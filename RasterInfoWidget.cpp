#include "RasterInfoWidget.h"  
#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>  
#include <QStringListModel>  
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <qDebug>
#include <QFileInfo>

RasterInfoWidget::RasterInfoWidget(QWidget* parent)  
 : QMainWindow(parent), m_tableView(nullptr) {  // 初始化 m_tableView 为 nullptr
     // 设置中心窗口  


     m_tableView = new QTableView(this);  
     QStandardItemModel* model = new QStandardItemModel(7, 2, this); // 设置模型，7 行 2 列  
     setCentralWidget(m_tableView);

     QStringList first_column_content;  
     first_column_content << "文件名" << "路径" << "分辨率" << "投影" << "波段数" << "均值" << "方差";  
     for (int row = 0; row < first_column_content.size(); ++row) {  
         model->setItem(row, 0, new QStandardItem(first_column_content[row]));  
     }  
     m_tableView->setColumnWidth(0, 50);
     m_tableView->setColumnWidth(1, 550);

     m_tableView->setModel(model);
}

void RasterInfoWidget::showRasterInfo(QString filePath) {  
  std::string s_rasterData = filePath.toStdString();  
  const char* c_rasterData = s_rasterData.c_str();  

  // 打开栅格文件  
  GDALDataset* poDataset = (GDALDataset*)GDALOpen(c_rasterData, GA_ReadOnly);  
  if (!poDataset) {  
      qDebug() << "无法打开文件！";  
      return;  
  }  

  // 获取分辨率
  double adfGeoTransform[6];
  double xResolution = 0, yResolution = 0;
  if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
      xResolution = adfGeoTransform[1];
      yResolution = abs(adfGeoTransform[5]);
  }

  // 获取投影
  const char* pszProj = poDataset->GetProjectionRef();

  // 获取波段总数
  int bandCount = poDataset->GetRasterCount();

  // 关闭数据集以释放资源  
  GDALClose(poDataset);  

  // 更新表格模型
  QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_tableView->model());
  if (model) {
      model->setItem(0, 1, new QStandardItem(QFileInfo(filePath).fileName())); // 文件名
      model->setItem(1, 1, new QStandardItem(filePath)); // 路径
      model->setItem(2, 1, new QStandardItem(QString("%1 x %2").arg(xResolution).arg(yResolution))); // 分辨率
      model->setItem(3, 1, new QStandardItem(pszProj)); // 投影
      model->setItem(4, 1, new QStandardItem(QString::number(bandCount))); // 波段数
  }
  m_tableView->setColumnWidth(0, 50);
  m_tableView->setColumnWidth(1, 550);
}