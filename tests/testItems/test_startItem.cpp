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
#include "frame/cgraphicsview.h"
#include "drawshape/cdrawscene.h"
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

#if TEST_START_ITEM

TEST(StartItem, TestStartItemCreateView)
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

TEST(StartItem, TestDrawStartItem)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CCentralwidget *c = getMainWindow()->getCCentralwidget();
    ASSERT_NE(c, nullptr);

    QToolButton *tool = nullptr;
    tool = c->getLeftToolBar()->findChild<QToolButton *>("StarTool");
    ASSERT_NE(tool, nullptr);
    tool->clicked();

    int addedCount = view->drawScene()->getBzItems().count();
    createItemByMouse(view);
    ASSERT_EQ(view->drawScene()->getBzItems().count(), addedCount + 1);

    ASSERT_EQ(view->drawScene()->getBzItems().first()->type(), PolygonalStarType);
}

TEST(StartItem, TestCopyStartItem)
{
    keyShortCutCopyItem();
}

TEST(StartItem, TestStartItemProperty)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CGraphicsPolygonalStarItem *start = dynamic_cast<CGraphicsPolygonalStarItem *>(view->drawScene()->getBzItems().first());
    ASSERT_NE(start, nullptr);

    // pen width
    setPenWidth(4);
    ASSERT_EQ(start->pen().width(), 4);

    // stroke color
    QColor strokeColor(Qt::red);
    setStrokeColor(strokeColor);
    ASSERT_EQ(start->pen().color(), strokeColor);

    // brush color
    QColor brushColor(Qt::green);
    setBrushColor(brushColor);
    ASSERT_EQ(start->brush().color(), brushColor);

    // Start anchor
    CSpinBox *sp = drawApp->topToolbar()->findChild<CSpinBox *>("StartAnchorNumber");
    ASSERT_NE(sp, nullptr);
    int value = sp->value();
    sp->setValue(value * 2);

    QTest::qWait(100);
    ASSERT_EQ(start->anchorNum(), sp->value());

    // Start Radius
    sp = drawApp->topToolbar()->findChild<CSpinBox *>("StartRadiusNumber");
    ASSERT_NE(sp, nullptr);
    value = sp->value();
    sp->setValue(value + 10);
    QTest::qWait(100);
    ASSERT_EQ(start->innerRadius(), sp->value());
}

TEST(StartItem, TestResizeStartItem)
{
    resizeItem();
}

TEST(StartItem, TestSelectAllStartItem)
{
    selectAllItem();
}

TEST(StartItem, TestSaveStartItemToFile)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);
    CCentralwidget *c = getMainWindow()->getCCentralwidget();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString StartItemPath = QApplication::applicationDirPath() + "/test_start.ddf";
    QFile file(StartItemPath);
    file.open(QIODevice::ReadWrite);
    file.close();
    view->getDrawParam()->setDdfSavePath(StartItemPath);
    c->slotSaveToDDF(true);
    QTest::qWait(100);

    QFileInfo info(StartItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(StartItem, TestOpenStartItemFromFile)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString StartItemPath = QApplication::applicationDirPath() + "/test_start.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(StartItemPath));
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
    ASSERT_EQ(true, addedCount == 2 ? true : false);
}

#endif
