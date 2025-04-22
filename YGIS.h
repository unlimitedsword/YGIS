#pragma once

#include <QtWidgets/QMainWindow>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QMenuBar>
#include "MapWidget.h"
#include "FileWidget.h"
#include "TextWidget.h"
#include "RasterInfoWidget.h"

class YGIS : public QMainWindow
{
    Q_OBJECT

public:
    YGIS(QWidget *parent = nullptr);
    ~YGIS();

    void createMenus();


private:
    QAction* m_openFileAction;
    QAction* m_refreshAction;

    MapWidget* m_mapWidget;
    FileWidget* m_fileWidget;
    TextWidget* m_textWidget;
};
