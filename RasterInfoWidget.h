#pragma once

#include <QMainWindow>
#include <QTableView>

class RasterInfoWidget : public QMainWindow {
    Q_OBJECT
public:
    explicit RasterInfoWidget(QWidget* parent = nullptr);

public slots:
    void showRasterInfo(QString filePath);

private:
    QTableView* m_tableView;
};