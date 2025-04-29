#pragma once
#include <QWidget>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include "Public.h"
#include "RasterInfoWidget.h"
#include "VectorElement.h"

class FileWidget : public QDockWidget 
{
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

    void updateFileListSignal();

    void rasterResample(const QString& filePath);  //栅格重采样

    void vectorBuffer(const QString& filePath);

public slots:
    void appendFile();

    void openInfoWidget(QString filePath);

    void addResampledFile(const QString& outputPath);

    void addBufferFile(const QString& outputPath);

signals:
    void bufferPathDeliverer(const QString& filePath,double radius);

    void filePathDelivered(const QString& filePath); // 中间函数发送信号

    void fileListUpdated(const QMap<QString, T_Information>& fileList);




private:
    RasterInfoWidget* m_rasterInfoWidget;
    VectorElement* m_vectorElement;
    QWidget* m_container; // 新增容器控件
    QTreeView* m_treeView;
    QStandardItemModel* m_model;
    QModelIndex m_contextMenuIndex; // 保存右键时的项索引

private slots:
    // 新增右键菜单槽函数
    void onCustomContextMenuRequested(const QPoint& pos);
    void deleteSelectedItem();
    void toggleVisibility();
    void deliverDataPath();
    void onItemChanged(QStandardItem* item);
};