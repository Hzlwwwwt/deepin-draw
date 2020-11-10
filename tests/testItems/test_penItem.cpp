/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:     zhanghao<zhanghao@uniontech.com>
* Maintainer: zhanghao<zhanghao@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "ccentralwidget.h"
#include "clefttoolbar.h"
#include "toptoolbar.h"
#include "cgraphicsview.h"
#include "cdrawscene.h"
#include "drawshape/cdrawparamsigleton.h"
#include "drawshape/drawItems/cgraphicsitemselectedmgr.h"
#include "application.h"

#include "crecttool.h"
#include "ccuttool.h"
#include "cellipsetool.h"
#include "cmasicotool.h"
#include "cpentool.h"
#include "cpolygonalstartool.h"
#include "cpolygontool.h"
#include "ctexttool.h"
#include "ctriangletool.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include "cpictureitem.h"
#include "cgraphicsrectitem.h"
#include "cgraphicsellipseitem.h"
#include "cgraphicstriangleitem.h"
#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspolygonitem.h"
#include "cgraphicslineitem.h"
#include "cgraphicspenitem.h"
#include "cgraphicstextitem.h"
#include "cgraphicsmasicoitem.h"
#include "cgraphicscutitem.h"

#include <QDebug>
#include <QtTest>
#include <QTestEventList>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_PEN_ITEM

TEST(PenItem, TestPenItemCreateView)
{
    int i = 0;
    while (i++ < 50) {
        QTest::qWait(200);
        if (getCurView() != nullptr) {
            break;
        }
    }
    if (getCurView() == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get CGraphicsView is nullptr.";
    }
    ASSERT_NE(getCurView(), nullptr);

    QTestEventList e;
    e.addKeyClick(Qt::Key_N, Qt::ControlModifier);
    e.simulate(getCurView());
    QTest::qWait(200);

    i = 0;
    while (i++ < 50) {
        QTest::qWait(200);
        if (getCurView() != nullptr) {
            break;
        }
    }
    if (getCurView() == nullptr) {
        qDebug() << __FILE__ << __LINE__ << "get CGraphicsView is nullptr.";
    }
    ASSERT_NE(getCurView(), nullptr);
}

TEST(PenItem, TestDrawPenItem)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CCentralwidget *c = getMainWindow()->getCCentralwidget();
    ASSERT_NE(c, nullptr);

    QToolButton *tool = nullptr;
    tool = c->getLeftToolBar()->findChild<QToolButton *>("PencilTool");
    ASSERT_NE(tool, nullptr);
    tool->clicked();

    QTestEventList e;
    int pointA = 100;
    int pointB = 300;
    int addedCount = view->drawScene()->getBzItems().count();
    e.addMouseMove(QPoint(pointA * 0.5, pointA * 0.8), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(pointA * 2, pointA * 2), 100);
    e.addMouseMove(QPoint(pointB * 1.7, pointB * 1.9), 100);
    e.addMouseMove(QPoint(pointB * 1.3, pointB * 1.6), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(pointB, pointB), 100);

    // 继续连续画线
    e.addMouseMove(QPoint(pointA + 200, pointA), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(pointA + 200, pointA), 100);
    e.addMouseMove(QPoint(pointB + 200, pointB), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(pointB + 200, pointB), 100);
    e.addKeyClick(Qt::Key_Escape);
    e.addDelay(100);

    e.simulate(view->viewport());
    ASSERT_EQ(view->drawScene()->getBzItems().count(), addedCount + 2);
    ASSERT_EQ(view->drawScene()->getBzItems().first()->type(), PenType);
}


TEST(PenItem, TestCopyPenItem)
{
    keyShortCutCopyItem();
}

TEST(PenItem, TestPenItemProperty)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CGraphicsPenItem *pen = dynamic_cast<CGraphicsPenItem *>(view->drawScene()->getBzItems().first());
    ASSERT_NE(pen, nullptr);

    view->drawScene()->selectItem(pen);
    setPenWidth(4);
    ASSERT_EQ(pen->pen().width(), 4);

    // Start Type
    DComboBox *typeCombox = drawApp->topToolbar()->findChild<DComboBox *>("LineOrPenStartType");
    ASSERT_NE(typeCombox, nullptr);
    for (int i = 0; i < typeCombox->count(); i++) {
        typeCombox->setCurrentIndex(i);
        QTest::qWait(100);
        ASSERT_EQ(pen->getPenStartType(), i);
    }

    // End Type
    typeCombox = drawApp->topToolbar()->findChild<DComboBox *>("LineOrPenEndType");
    ASSERT_NE(typeCombox, nullptr);
    for (int i = 0; i < typeCombox->count(); i++) {
        typeCombox->setCurrentIndex(i);
        QTest::qWait(100);
        ASSERT_EQ(pen->getPenEndType(), i);
    }
}

TEST(PenItem, TestResizePenItem)
{
    resizeItem();
}

TEST(PenItem, TestSelectAllPenItem)
{
    selectAllItem();
}

TEST(PenItem, TestSavePenItemToFile)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CCentralwidget *c = getMainWindow()->getCCentralwidget();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString PenItemPath = QApplication::applicationDirPath() + "/test_pen.ddf";
    QFile file(PenItemPath);
    file.open(QIODevice::ReadWrite);
    file.close();
    view->getDrawParam()->setDdfSavePath(PenItemPath);
    c->slotSaveToDDF(true);
    QTest::qWait(100);

    QFileInfo info(PenItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(PenItem, TestOpenPenItemFromFile)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString PenItemPath = QApplication::applicationDirPath() + "/test_pen.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(PenItemPath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    QTest::qWait(100);

    view = getCurView();
    ASSERT_NE(view, nullptr);
    int addedCount = view->drawScene()->getBzItems(view->drawScene()->items()).count();
    ASSERT_EQ(true, addedCount == 3 ? true : false);
}

#endif
