#pragma once

#include <QMainWindow>
#include <QTableView>
#include <gdal_priv.h>
#include <gdalwarper.h>

class RasterInfoWidget : public QMainWindow {
    Q_OBJECT
public:
    explicit RasterInfoWidget(QWidget* parent = nullptr);

    template<GDALResampleAlg Alg>
    void ResampleWithDialog(const QString& inputPath, double scaleFactor);

    void ResampleNearest(const QString& path) {
        ResampleWithDialog<GRA_NearestNeighbour>(path, 2.0);
    }

    void ResampleBilinear(const QString& path) {
        ResampleWithDialog<GRA_Bilinear>(path, 0.5);
    }

    void ResampleCubic(const QString& path) {
        ResampleWithDialog<GRA_Cubic>(path, 1.5);
    }

    bool ResampleRaster(const QString& inputPath,
        const QString& outputPath,
        GDALResampleAlg resampleAlg,
        double scaleFactor = 1.0);


signals:
    void resampleCompleted(const QString& outputPath);


public slots:
    void showRasterInfo(QString filePath);

private:
    QTableView* m_tableView;
};