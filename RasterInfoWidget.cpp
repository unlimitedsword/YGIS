#include "RasterInfoWidget.h"  
#include <QMainWindow>
#include <QTableView>
#include <QStandardItemModel>  
#include <QStringListModel>  
#include <QHeaderView>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogrsf_frmts.h>
#include <qDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

RasterInfoWidget::RasterInfoWidget(QWidget* parent)  
 : QMainWindow(parent), m_tableView(nullptr) {  // 初始化 m_tableView 为 nullptr
     // 设置中心窗口  


     m_tableView = new QTableView(this);  
     QStandardItemModel* model = new QStandardItemModel(7, 2, this); // 设置模型，7 行 2 列  
     setCentralWidget(m_tableView);
     m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
     m_tableView->horizontalHeader()->setVisible(false);
     m_tableView->verticalHeader()->setVisible(false);


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

// 公共重采样函数
bool RasterInfoWidget::ResampleRaster(const QString& inputPath,
    const QString& outputPath,
    GDALResampleAlg resampleAlg,
    double scaleFactor)
{
    qDebug() << "\n====== 开始重采样操作 ======";
    qDebug() << "GDAL版本:" << GDALVersionInfo("RELEASE_NAME");

    GDALAllRegister();

    // 打开输入数据集
    qDebug() << "\n[1/7] 打开输入文件...";
    GDALDataset* srcDS = (GDALDataset*)GDALOpen(inputPath.toUtf8().constData(), GA_ReadOnly);
    if (!srcDS) {
        qCritical() << "错误：无法打开输入文件";
        QMessageBox::critical(this, "错误", "无法打开输入文件");
        return false;
    }

    // 获取输入参数
    const int srcWidth = srcDS->GetRasterXSize();
    const int srcHeight = srcDS->GetRasterYSize();
    const int dstWidth = static_cast<int>(srcWidth * scaleFactor);
    const int dstHeight = static_cast<int>(srcHeight * scaleFactor);
    const GDALDataType dataType = srcDS->GetRasterBand(1)->GetRasterDataType();

    qDebug() << "输入参数:";
    qDebug() << "  原始尺寸:" << srcWidth << "x" << srcHeight;
    qDebug() << "  目标尺寸:" << dstWidth << "x" << dstHeight;
    qDebug() << "  数据类型:" << GDALGetDataTypeName(dataType);

    // 创建输出数据集
    qDebug() << "\n[2/7] 创建输出文件...";
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) {
        qCritical() << "错误：无法获取GTiff驱动";
        GDALClose(srcDS);
        return false;
    }

    // 设置创建选项（GDAL 2.0兼容参数）
    char** options = nullptr;
    options = CSLSetNameValue(options, "TILED", "YES");
    options = CSLSetNameValue(options, "COMPRESS", "LZW");
    options = CSLSetNameValue(options, "BIGTIFF", "IF_NEEDED");

    GDALDataset* dstDS = driver->Create(
        outputPath.toUtf8().constData(),
        dstWidth,
        dstHeight,
        srcDS->GetRasterCount(),
        dataType,
        options
    );

    CSLDestroy(options);

    if (!dstDS) {
        qCritical() << "错误：无法创建输出文件";
        GDALClose(srcDS);
        return false;
    }

    // 设置地理参考参数
    qDebug() << "\n[3/7] 设置地理参考...";
    double adfGeoTransform[6];
    if (srcDS->GetGeoTransform(adfGeoTransform) == CE_None) {
        adfGeoTransform[1] /= scaleFactor;  // 调整X分辨率
        adfGeoTransform[5] /= scaleFactor;  // 调整Y分辨率

        if (dstDS->SetGeoTransform(adfGeoTransform) != CE_None) {
            qWarning() << "警告：设置地理变换失败";
        }
    }

    // 设置投影
    const char* proj = srcDS->GetProjectionRef();
    if (proj && strlen(proj) > 0) {
        dstDS->SetProjection(proj);
    }

    // 配置重采样参数（GDAL 2.0.2兼容）
    qDebug() << "\n[4/7] 配置重采样参数...";
    GDALWarpOptions* warpOptions = GDALCreateWarpOptions();

    // 关键参数
    warpOptions->hSrcDS = srcDS;
    warpOptions->hDstDS = dstDS;
    warpOptions->nBandCount = srcDS->GetRasterCount();
    warpOptions->eResampleAlg = resampleAlg;

    // 波段映射（GDAL 2.0需要显式设置）
    warpOptions->panSrcBands = (int*)CPLMalloc(sizeof(int) * warpOptions->nBandCount);
    warpOptions->panDstBands = (int*)CPLMalloc(sizeof(int) * warpOptions->nBandCount);
    for (int i = 0; i < warpOptions->nBandCount; i++) {
        warpOptions->panSrcBands[i] = i + 1;
        warpOptions->panDstBands[i] = i + 1;
    }

    // 坐标转换器（GDAL 2.0必需）
    warpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(
        srcDS,
        srcDS->GetProjectionRef(),
        dstDS,
        dstDS->GetProjectionRef(),
        FALSE, 0.0, 0);
    warpOptions->pfnTransformer = GDALGenImgProjTransform;

    if (!warpOptions->pTransformerArg) {
        qCritical() << "坐标转换器创建失败：" << CPLGetLastErrorMsg();
        GDALDestroyWarpOptions(warpOptions);
        GDALClose(srcDS);
        GDALClose(dstDS);
        return false;
    }

    // 执行重采样
    qDebug() << "\n[5/7] 执行重采样...";
    GDALWarpOperation warpOperation;
    CPLErr err = warpOperation.Initialize(warpOptions);

    if (err == CE_None) {
        err = warpOperation.ChunkAndWarpImage(0, 0, dstWidth, dstHeight);
        qDebug() << "重采样结果:" << (err == CE_None ? "成功" : "失败");
    }
    else {
        qCritical() << "初始化失败：" << CPLGetLastErrorMsg();
    }

    // 计算统计信息
    qDebug() << "\n[6/7] 计算统计信息...";
    for (int i = 1; i <= dstDS->GetRasterCount(); i++) {
        GDALRasterBand* band = dstDS->GetRasterBand(i);
        CPLErr statsErr = band->ComputeStatistics(
            false,  // 不强制计算
            nullptr, nullptr, nullptr, nullptr,  // 不需要详细进度
            GDALDummyProgress, nullptr
        );
        if (statsErr == CE_None) {
            double min, max, mean, stddev;
            band->GetStatistics(false, true, &min, &max, &mean, &stddev);
            qDebug() << "波段" << i << "范围: [" << min << ", " << max << "]";
        }
    }

    // 清理资源
    qDebug() << "\n[7/7] 清理资源...";
    if (warpOptions->pTransformerArg) {
        GDALDestroyGenImgProjTransformer(warpOptions->pTransformerArg);
    }
    GDALDestroyWarpOptions(warpOptions);
    GDALClose(srcDS);
    GDALClose(dstDS);

    if (err != CE_None) {
        QMessageBox::critical(this, "错误", QString("重采样失败:\n%1").arg(CPLGetLastErrorMsg()));
        return false;
    }

    qDebug() << "====== 操作成功完成 ======\n";
    return true;
}

// 带文件选择的重采样函数模板
template<GDALResampleAlg Alg>
void RasterInfoWidget::ResampleWithDialog(const QString& inputPath, double scaleFactor) {
    QFileInfo inputFile(inputPath);
    if (!inputFile.exists()) {
        QMessageBox::critical(this, "错误", QString("输入文件不存在：\n%1").arg(inputPath));
        return;
    }

    QFileDialog saveDialog(this);
    saveDialog.setWindowTitle("保存输出文件");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setNameFilter("GeoTIFF (*.tif)");
    saveDialog.setDefaultSuffix("tif");

    if (saveDialog.exec() != QDialog::Accepted) {
        qDebug() << "用户取消了保存对话框";
        return;
    }

    const QString outputPath = saveDialog.selectedFiles().first();

    if (ResampleRaster(inputPath, outputPath, Alg, scaleFactor)) {
        QMessageBox::information(this, "完成", "重采样操作成功完成");
    }
}


// 显式实例化所有需要的模板参数
template void RasterInfoWidget::ResampleWithDialog<GRA_NearestNeighbour>(const QString&, double);
template void RasterInfoWidget::ResampleWithDialog<GRA_Bilinear>(const QString&, double);
template void RasterInfoWidget::ResampleWithDialog<GRA_Cubic>(const QString&, double);
