#pragma once

#include <QtWidgets/QMainWindow>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QMenuBar>

#include "MapWidget.h"
#include "FileWidget.h"
#include "TextWidget.h"

class YGIS : public QMainWindow
{
    Q_OBJECT

public:
    YGIS(QWidget *parent = nullptr);
    ~YGIS();

    void createMenus();


private:
    QAction* openFile;
    QAction* refresh;

    MapWidget* mapWidget;
    FileWidget* fileWidget;
    TextWidget* textWidget;
};
