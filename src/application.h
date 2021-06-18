/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Ji XiangLong <jixianglong@uniontech.com>
 *
 * Maintainer: WangYu <wangyu@uniontech.com>
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
#ifndef APPLICATION_H
#define APPLICATION_H

#define protected public
#include <DApplication>
#undef protected

#include <DGuiApplicationHelper>
#include <QStack>


#include "globaldefine.h"

class MainWindow;
class CColorPickWidget;
class TopToolbar;
class CLeftToolBar;
class CDrawScene;
class CGraphicsView;
class QMimeData;
class CShapeMimeData;

#if  defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<DApplication *>(QCoreApplication::instance()))
#define drawApp (Application::drawApplication())
#define CHECK_MOSUEACTIVE_RETURN \
    if(dApp->mouseButtons() == Qt::LeftButton)\
    {\
        auto view1 = CManageViewSigleton::GetInstance()->getCurView();\
        if(view1 != nullptr && view1->viewport()->rect().contains(view1->viewport()->mapFromGlobal(QCursor::pos())))\
        {\
            return;\
        }\
    }
#define CHECK_CURRENTTOOL_RETURN(pView) \
    CHECK_MOSUEACTIVE_RETURN \
    auto view = CManageViewSigleton::GetInstance()->getCurView();\
    if(pView != view){return;}\
    if(pView == nullptr){return;}\
    EDrawToolMode currentMode = view->getDrawParam()->getCurrentDrawToolMode();\
    if (currentMode != selection) {\
        return;\
    }

DWIDGET_USE_NAMESPACE

class Application : public QObject
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();

    /**
     * @brief drawApplication 返回顶层topToolbar
     * @return 返回画板程序的指针
     */
    static Application *drawApplication();


    DApplication *dApplication();

    void setClipBoardShapeData(QMimeData *data);
    QMimeData *clipBoardShapeData()const;


    int  execDraw(const QStringList &paths);

    /**
     * @brief topMainWindow 返回顶层mainwindow
     * @return
     */
    MainWindow *topMainWindow();
    QWidget *topMainWindowWidget();

    /**
     * @brief colorPickWidget 返回顶层colorPickWidget
     * @return
     */
    CColorPickWidget *colorPickWidget(bool creatNew = false, QWidget *pCaller = nullptr);

    /**
     * @brief topToolbar 返回顶层topToolbar
     * @return 返回顶层工具块界面指针
     */
    TopToolbar *topToolbar();

    /**
     * @brief leftToolBar 返回顶层leftToolBar
     * @return 返回左侧工具块界面指针
     */
    CLeftToolBar *leftToolBar();

    /**
     * @brief currentDrawScence 返回当前显示的画布场景
     * @return 返回当前显示的画布场景指针
     */
    CDrawScene *currentDrawScence();

    /**
     * @brief setViewCurrentTool 设置视窗场景当前的工具
     */
    void  setViewCurrentTool(CGraphicsView *pView, EDrawToolMode tool);

    /**
     * @brief isViewToolEnable 视窗场景的某个工具当前是否可用
     */
    bool  isViewToolEnable(CGraphicsView *pView, EDrawToolMode tool);

    /**
     * @brief getRightFiles 根据输入返回所有合法正确的可加载文件(并且会弹窗提示)
     * @return 返回所有合法正确的可加载文件
     */
    QStringList getRightFiles(const QStringList &files, bool notice = true);

    /**
     * @brief isFileNameLegal 判断文件是否合法(是否可加载)
     * @return 返回文件是否合法
     */
    bool isFileNameLegal(const QString &path, int *outErrorReson = nullptr);

    /**
     * @brief saveCursor 保存全局鼠标样式
     */
    void saveCursor();

    /**
     * @brief setApplicationCursor 设置全局鼠标样式
     */
    void setApplicationCursor(const QCursor &cur, bool force = false);

    /**
     * @brief restoreCursor 还原全局鼠标样式
     */
    void restoreCursor();

    /**
     * @brief setTouchFeelingEnhanceValue 设置触控感受的增强值
     */
    void setTouchFeelingEnhanceValue(int value);

    /**
     * @brief touchFeelingEnhanceValue 当前触控感受的增强值
     */
    int  touchFeelingEnhanceValue();

    /**
     * @brief supPictureSuffix 返回支持的所有图片后缀名
     */
    static QStringList &supPictureSuffix();

    /**
     * @brief supDdfStuffix 返回支持的所有ddf后缀名(暂时只有.ddf)
     */
    static QStringList &supDdfStuffix();

    /**
     * @brief systemThemeColor 获取系统主题颜色
     * @return
     */
    QColor  systemThemeColor();

    /**
     * @brief isFileExist 文件是否存在，如果是资源型路径那么会将其替换成本地路径
     */
    bool isFileExist(QString &filePath);

    /**
     * @brief setWidgetAllPosterityNoFocus 将widget的后代都设置为没有焦点
     */
    static void  setWidgetAllPosterityNoFocus(QWidget *pW);

    /**
     * @brief setWidgetAccesibleName 设置widget对象外界可访问的名字属性
     */
    static void setWidgetAccesibleName(QWidget *w, const QString &name);

    /**
     * @brief 当程序退出时会调用
     */
    void onAppQuit();

private:
    enum EFileClassEnum {ENotFile    = 0,
                         ENotExist,
                         EDrawAppNotSup,
                         EDrawAppSup,
                         EDrawAppSupAndReadable,
                         EDrawAppSupButNotReadable
                        };
    typedef QMap<EFileClassEnum, QStringList> QFileClassedMap;
    QStringList doFileClassification(const QStringList &inFilesPath, QFileClassedMap &out);

signals:
    void popupConfirmDialog();

public slots:
    void onThemChanged(DGuiApplicationHelper::ColorType themeType);
    void activateWindow();
    void showMainWindow(const QStringList &paths);
    void noticeFileRightProblem(const QStringList &problemfile,
                                Application::EFileClassEnum classTp = EDrawAppNotSup,
                                bool checkQuit = true);


public:
    enum   EMessageType {ENormalMsg, EWarningMsg, EQuestionMsg};
    Q_SLOT int exeMessage(const QString &message, EMessageType msgTp = EWarningMsg,
                          bool autoFitDialogWidth = true,
                          const QStringList &moreBtns = QStringList() << tr("OK"),
                          const QList<int> &btnType = QList<int>() << 0);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void initI18n();
    MainWindow *actWin = nullptr;

    CColorPickWidget *_colorPickWidget = nullptr;

    DApplication *_dApp = nullptr;

    int  _touchEnchValue = 7;

    static Application *s_drawApp;

    CShapeMimeData *_pClipBordData = nullptr;

    QStack<QCursor> _cursorStack;
};
#endif // APPLICATION_H
