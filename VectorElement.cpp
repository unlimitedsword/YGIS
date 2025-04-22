#include "VectorElement.h"
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <QStandardItemModel>
#include <QList>

VectorElement::VectorElement(QWidget* parent)
	: QMainWindow(parent), m_elementView(nullptr) {

    QMenuBar* m_MenuBar = new QMenuBar(this);
    setMenuBar(m_MenuBar);


    // 创建菜单项
    m_deleteAction = new QAction(tr("&Detele"), this);
    m_saveAction = new QAction(tr("&Save"), this);

    // 添加菜单项到 File 菜单
    m_MenuBar->addAction(m_deleteAction);
    m_MenuBar->addAction(m_saveAction);

	m_elementView = new QTableView(this);
	setCentralWidget(m_elementView);


    connect(m_deleteAction, &QAction::triggered, this, &VectorElement::deleteElement);
    connect(m_saveAction, &QAction::triggered, this, &VectorElement::saveElement);

}

void VectorElement::vectorElementInfo(const QString filePath) {
    GDALAllRegister(); // 注册所有驱动  

    m_filePath = filePath; // 保存文件路径

    std::string s_VectorData = filePath.toStdString();
    const char* c_VectorData = s_VectorData.c_str();

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(c_VectorData, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (!poDS) {
        qDebug() << "文件打开失败";
        return;
    }

    OGRLayer* poLayer = poDS->GetLayer(0); // 获取第一个图层  
    if (!poLayer) {
        qDebug() << "无法获取图层";
        GDALClose(poDS);
        return;
    }

    // 获取字段名称
    QList<QString> fieldNames;
    OGRFeatureDefn* poFDefn = poLayer->GetLayerDefn();
    for (int iField = 0; iField < poFDefn->GetFieldCount(); iField++) {
        OGRFieldDefn* poFieldDefn = poFDefn->GetFieldDefn(iField);
        if (poFieldDefn != nullptr) {
            const char* cFieldName = poFieldDefn->GetNameRef();
            QString qFieldName = QString::fromUtf8(cFieldName);
            fieldNames.append(qFieldName);
        }
    }

    // 创建模型
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setColumnCount(fieldNames.size());
    for (int i = 0; i < fieldNames.size(); ++i) {
        model->setHeaderData(i, Qt::Horizontal, fieldNames[i]);
    }

    // 遍历要素并填充模型
    OGRFeature* poFeature = poLayer->GetNextFeature();
    int row = 0;
    while (poFeature != nullptr) {
        model->insertRow(row);
        for (int iField = 0; iField < poFeature->GetFieldCount(); iField++) {
            if (poFeature->IsFieldSet(iField)) {
                QString fieldValue = QString::fromUtf8(poFeature->GetFieldAsString(iField));
                model->setItem(row, iField, new QStandardItem(fieldValue));
            }
        }
        OGRFeature::DestroyFeature(poFeature);
        poFeature = poLayer->GetNextFeature();
        row++;
    }

    GDALClose(poDS); // 关闭数据集  

    // 将模型设置到 QTableView
    m_elementView->setModel(model);
}

void VectorElement::deleteElement() {
    // 获取当前模型
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(m_elementView->model());
    if (!model) {
        qDebug() << "模型为空，无法删除";
        return;
    }

    // 获取选中的行
    QItemSelectionModel* selectionModel = m_elementView->selectionModel();
    if (!selectionModel) {
        qDebug() << "选择模型为空，无法删除";
        return;
    }

    QModelIndexList selectedIndexes = selectionModel->selectedRows();
    if (selectedIndexes.isEmpty()) {
        qDebug() << "未选择任何行";
        return;
    }

    // 遍历选中的行并记录要素 ID
    for (const QModelIndex& index : selectedIndexes) {
        int row = index.row();

        // 假设模型的第一列存储的是要素 ID
        QString featureIdStr = model->item(row, 0)->text();
        int featureId = featureIdStr.toInt();

        // 将要素 ID 添加到删除列表
        m_deletedFeatureIds.append(featureId);

        // 从模型中移除行
        model->removeRow(row);
    }

    // 更新视图
    m_elementView->reset();

    qDebug() << "已标记删除的要素 ID：" << m_deletedFeatureIds;
}

void VectorElement::saveElement() {
    if (m_filePath.isEmpty()) {
        qDebug() << "文件路径为空，无法保存";
        return;
    }

    if (m_deletedFeatureIds.isEmpty()) {
        qDebug() << "没有要保存的更改";
        return;
    }

    // 打开矢量数据集
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(m_filePath.toStdString().c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, nullptr, nullptr, nullptr);
    if (!poDS) {
        qDebug() << "无法打开矢量数据集";
        return;
    }

    OGRLayer* poLayer = poDS->GetLayer(0); // 假设操作第一个图层
    if (!poLayer) {
        qDebug() << "无法获取图层";
        GDALClose(poDS);
        return;
    }

    // 删除标记的要素
    for (int featureId : m_deletedFeatureIds) {
        if (poLayer->DeleteFeature(featureId) != OGRERR_NONE) {
            qDebug() << "删除要素失败，ID：" << featureId;
        }
        else {
            qDebug() << "成功删除要素，ID：" << featureId;
        }
    }

    // 清空删除列表
    m_deletedFeatureIds.clear();

    // 关闭数据集
    GDALClose(poDS);

    qDebug() << "保存完成";
}