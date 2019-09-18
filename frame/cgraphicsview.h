/*
 * Copyright (C) 2019 ~ %YEAR% Deepin Technology Co., Ltd.
 *
 * Author:     WangXing
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CGRAPHICSVIEW_H
#define CGRAPHICSVIEW_H

#include <DGraphicsView>
#include "drawshape/csizehandlerect.h"
DWIDGET_USE_NAMESPACE

class QUndoStack;
class CDDFManager;
class CGraphicsItem;
class CPrintManager;
class CExportImageDialog;
class CGraphicsPolygonItem;
class CGraphicsPolygonalStarItem;

class CGraphicsView : public DGraphicsView
{
    Q_OBJECT
public:
    CGraphicsView(DWidget *parent = nullptr);
    void zoomOut();
    void zoomIn();
    void scale(qreal scale);
    void showExportDialog();
    void showPrintDialog();
    void clearScene();
    void showSaveDDFDialog(bool type);
    void doSaveDDF();
    void doImport();
    void setIsShowContext(bool isShowContext);


protected:
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void drawItems(QPainter *painter, int numItems,
                           QGraphicsItem *items[],
                           const QStyleOptionGraphicsItem options[]) Q_DECL_OVERRIDE;

    //virtual QPainter *sharedPainter() const Q_DECL_OVERRIDE;
signals:
    void signalSetScale(const qreal scale);

public slots:
    void itemMoved(QGraphicsItem *item, const QPointF &oldPosition );
    void itemAdded(QGraphicsItem *item );
    void itemRotate(QGraphicsItem *item, const qreal oldAngle );
    void itemResize(CGraphicsItem *item, CSizeHandleRect::EDirection handle, QPointF beginPos, QPointF endPos, bool bShiftPress, bool bALtPress);
    void itemPropertyChange(CGraphicsItem *item, QPen pen, QBrush brush, bool bPenChange, bool bBrushChange);
    void itemPolygonPointChange(CGraphicsPolygonItem *item, int oldNum);
    void itemPolygonalStarPointChange(CGraphicsPolygonalStarItem *item, int oldNum, int oldRadius);
    void slotDoCut(QRectF);

private slots:
    void slotOnCut();
    void slotOnCopy();
    void slotOnPaste();
    void slotOnSelectAll();
    void slotOnDelete();
    void slotOneLayerUp();
    void slotOneLayerDown();
    void slotBringToFront();
    void slotSendTobackAct();
    void slotQuitCutMode();

    ///文字右键菜单槽函数
    void slotOnTextCut();
    void slotOnTextCopy();
    void slotOnTextPaste();
    void slotOnTextSelectAll();
    void slotOnTextTopAlignment();
    void slotOnTextRightAlignment();
    void slotOnTextLeftAlignment();
    void slotOnTextCenterAlignment();
    ///


private:
    qreal m_scale; //记录当前缩放

    DMenu *m_contextMenu;//右键菜单
    QAction *m_cutAct;              //剪切
    QAction *m_copyAct;             //拷贝
    QAction *m_pasteAct;            //粘贴
    QAction *m_selectAllAct;        //全选
    QAction *m_deleteAct;           //删除
    QAction *m_undoAct;             //撤销
    QAction *m_redoAct;             //重做
    QAction *m_oneLayerUpAct;       //向上一层
    QAction *m_oneLayerDownAct;     //向下一层
    QAction *m_bringToFrontAct;     //置于最顶层
    QAction *m_sendTobackAct;       //置于最底层
//    QAction *m_leftAlignAct;
//    QAction *m_topAlignAct;
//    QAction *m_rightAlignAct;
//    QAction *m_centerAlignAct;

    QAction *m_quitCutMode;        //退出裁剪

    ///文字图元右键菜单
    DMenu *m_textMenu;
    QAction *m_textCutAction;
    QAction *m_textCopyAction;
    QAction *m_textPasteAction;
    QAction *m_textSelectAllAction;
    QAction *m_textLeftAlignAct;
    QAction *m_textTopAlignAct;
    QAction *m_textRightAlignAct;
    QAction *m_textCenterAlignAct;
    ///

    QUndoStack *m_pUndoStack;

    CExportImageDialog *m_exportImageDialog;
    CPrintManager *m_printManager;
    bool m_visible;

    CDDFManager *m_DDFManager;
    QString m_ddfFileSavePath;

    bool m_isShowContext;

    QRectF m_windRect;

    int m_viewWidth;

private:
    void initContextMenu();
    void initContextMenuConnection();
    void initTextContextMenu();
    void initTextContextMenuConnection();
    QPixmap getSceneImage();
    void setContextMenu();

};

#endif // CGRAPHICSVIEW_H
