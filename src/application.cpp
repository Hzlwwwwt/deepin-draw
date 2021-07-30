﻿/*
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
#include "application.h"
#include "frame/mainwindow.h"
#include "frame/cviewmanagement.h"
#include "service/dbusdraw_adaptor.h"
#include "ccolorpickwidget.h"
#include "globaldefine.h"
#include "cviewmanagement.h"
#include "cgraphicsview.h"
#include "ccentralwidget.h"
#include "cdrawscene.h"
#include "colorpanel.h"
#include "acobjectlist.h"
#include "clefttoolbar.h"
#include "cdrawtoolmanagersigleton.h"
#include "cattributemanagerwgt.h"
#include "toptoolbar.h"
#include "cshapemimedata.h"
#include "cdrawtoolfactory.h"

#include <QFileInfo>
#include <QDBusConnection>
#include <QAccessible>
#include <DApplicationSettings>
#include <QClipboard>
#include <QTextEdit>

#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <DVtableHook>
#include <DGuiApplicationHelper>

#include <QtConcurrent>

#include <malloc.h>

#include <DLog>
#include <DApplicationHelper>

#include "config.h"

Application *Application::s_drawApp = nullptr;

Application::Application(int &argc, char **argv)
    : QObject(nullptr)
{
    if (s_drawApp == nullptr) {
        s_drawApp = this;
    }

#if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
    _dApp = new DApplication(argc, argv);
#else
    _dApp = DApplication::globalApplication(argc, argv);
#endif
#ifdef ENABLE_ACCESSIBILITY
    QAccessible::installFactory(accessibleFactory);
#endif
    _dApp->installEventFilter(this);

    _dApp->loadTranslator();
    loadTools();

    connect(_dApp, &DApplication::focusChanged, this, &Application::onFocusChanged);


    dApp->setQuitOnLastWindowClosed(true);

    //绑定主题发生变化的信号
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Application::onThemChanged);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, [ = ]() {
        setClipBoardShapeData(const_cast<QMimeData *>(QApplication::clipboard()->mimeData()));
    });

    //qRegisterMetaType<EFileClassEnum>("EFileClassEnum");
    //qRegisterMetaType<Application::EFileClassEnum>("Application::EFileClassEnum");
    qRegisterMetaType<EChangedPhase>("EChangedPhase");

    //qApp->setOverrideCursor(Qt::ArrowCursor);

    //当前handlequit虚函数在虚表中16,重写函数的调用地址
    Dtk::Core::DVtableHook::overrideVfptrFun(dApplication(), &DApplication::handleQuitAction, this, &Application::onAppQuit);
}

Application::~Application()
{
    if (_pClipBordData != nullptr) {
        delete _pClipBordData;
    }
}

Application *Application::drawApplication()
{
    return s_drawApp;
}

DApplication *Application::dApplication()
{
    return _dApp;
}

void Application::setClipBoardShapeData(QMimeData *data)
{
    if (_pClipBordData != nullptr) {
        delete _pClipBordData;
        _pClipBordData = nullptr;
    }

    if (data != nullptr) {
        auto shapeData = qobject_cast<CShapeMimeData *>(data);
        if (shapeData != nullptr) {
            _pClipBordData = shapeData;
        }
    }
}

QMimeData *Application::clipBoardShapeData() const
{
    if (_pClipBordData != nullptr)
        return _pClipBordData;

    return const_cast<QMimeData *>(QApplication::clipboard()->mimeData());
}

int Application::execDraw(const QStringList &paths)
{
    _dApp->setOrganizationName("deepin");
    _dApp->setApplicationName("deepin-draw");
    _dApp->setApplicationDisplayName(tr("Draw"));
    _dApp->setQuitOnLastWindowClosed(true);
    using namespace Dtk::Core;
    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    QLocale local(QLocale::Uighur);
    _dApp->loadTranslator(QList<QLocale>() << local);

    //CManageViewSigleton::GetInstance()->setThemeType(DGuiApplicationHelper::instance()->themeType());

    showMainWindow(paths);

    topMainWindow()->drawBoard()->initTools();

    int ret = _dApp->exec();

    delete actWin;

    actWin = nullptr;

    return ret;
}

MainWindow *Application::topMainWindow() const
{
    return actWin;
}

QWidget *Application::topMainWindowWidget() const
{
    return qobject_cast<QWidget *>(actWin);
}

TopTilte *Application::topToolbar() const
{
    if (actWin != nullptr)
        return actWin->topTitle();
    return nullptr;
}

DrawToolManager *Application::leftToolBar() const
{
    if (topMainWindow() != nullptr)
        return topMainWindow()->drawBoard()->toolManager();
    return nullptr;
}

DrawBoard *Application::drawBoard() const
{
    if (topMainWindow() != nullptr)
        return topMainWindow()->drawBoard();
    return nullptr;
}

CColorPickWidget *Application::colorPickWidget()
{
    if (drawBoard() != nullptr) {
        if (drawBoard()->attributionWidget() != nullptr) {
            auto w = drawBoard()->attributionWidget()->widgetOfAttr(EPenColor);
            if (w != nullptr) {
                CColorSettingButton *button = qobject_cast<CColorSettingButton *>(w);
                if (button != nullptr) {
                    return button->colorPick();
                }
            }
        }
    }
    return nullptr;
}

DrawAttribution::CAttributeManagerWgt *Application::attributionsWgt()
{
    return topToolbar()->attributionsWgt();
}

PageScene *Application::currentDrawScence()
{
    if (drawBoard() != nullptr && drawBoard()->currentPage() != nullptr) {
        return drawBoard()->currentPage()->view()->drawScene();
    }
    return nullptr;
}

void Application::setCurrentTool(int tool)
{
    if (drawBoard() != nullptr)
        drawBoard()->toolManager()->setCurrentTool(tool);
}

int Application::currentTool()
{
    if (drawBoard() != nullptr)
        return drawBoard()->toolManager()->currentTool();

    return -1;
}

void Application::setPageTool(Page *page, EDrawToolMode tool)
{
    if (page != nullptr) {
        page->setCurrentTool(tool);
    }
}

void Application::openFiles(QStringList files, bool asFirstPictureSize, bool addUndoRedo, bool newScence, bool appFirstExec)
{
//    if (topMainWindow() != nullptr)
//        topMainWindow()->getCCentralwidget()->openFiles(files, asFirstPictureSize,
//                                                        addUndoRedo, newScence, appFirstExec);
}

//QStringList Application::getRightFiles(const QStringList &files, bool notice)
//{
//    //过滤文件
//    //判断文件是否是可读或者可写的(图片判断是否可读 ddf判断是否可读可写)
//    Application::QFileClassedMap outFiles;
//    QStringList paths  = this->doFileClassification(files, outFiles);

//    outFiles.remove(Application::EDrawAppSup);

//    for (auto it = outFiles.begin(); it != outFiles.end(); ++it) {

//        Application::EFileClassEnum classTp = it.key();

//        QStringList &problemFiles = it.value();

//        if (!problemFiles.isEmpty() && notice) {

//            //提示有文件的有问题的文件集(为什么检查是否关闭程序的标记为是否等于第一个key而不是最后一个key呢？因为最上层弹窗是最后一次noticeFileRightProblem)
//            //所以用户点击关闭的最后一个弹窗是最先执行的noticeFileRightProblem，所以要在用户关闭完所有的弹窗后再判断是否需要推出程序。
//            QMetaObject::invokeMethod(this, "noticeFileRightProblem", Qt::QueuedConnection,
//                                      Q_ARG(const QStringList &, problemFiles),
//                                      Q_ARG(Application::EFileClassEnum, classTp),
//                                      Q_ARG(bool, classTp == outFiles.firstKey()));

//        }
//    }
//    return paths;
//}

//QStringList Application::doFileClassification(const QStringList &inFilesPath, Application::QFileClassedMap &out)
//{
//    out.clear();

//    QStringList supFiles;
//    for (int i = 0; i < inFilesPath.size(); ++i) {
//        QString path = inFilesPath.at(i);

//        if (!isFileExist(path)) {
//            out[ENotExist].append(path);
//            continue;
//        }

//        QFileInfo     info(path);
//        if (info.isFile()) {
//            const QString suffix = info.suffix().toLower();
//            if (supPictureSuffix().contains(suffix) || supDdfStuffix().contains(suffix)) {
//                out[EDrawAppSup].append(path);
//                if (!info.isReadable()) {
//                    out[EDrawAppSupButNotReadable].append(path);
//                } else {
//                    out[EDrawAppSupAndReadable].append(path);
//                    supFiles.append(path);
//                }
//            } else {
//                out[EDrawAppNotSup].append(path);
//            }
//        } else {
//            out[ENotFile].append(path);
//        }
//    }
//    return supFiles;
//}

QColor Application::systemThemeColor()
{
    auto scene = currentDrawScence();
    if (scene != nullptr) {
        DPalette pa = scene->palette();
        QBrush activeBrush = pa.brush(QPalette::Active, DPalette::Highlight);
        return  activeBrush.color();
    }
    return topMainWindow()->palette().brush(QPalette::Active, DPalette::Highlight).color();
}

void Application::setWidgetAllPosterityNoFocus(QWidget *pW)
{
    if (pW == nullptr)
        return;

    QList<QObject *> list0 = pW->children();
    for (int i  = 0; i < list0.size(); ++i) {
        QObject *o = list0[i];
        if (o->isWidgetType()) {
            QWidget *pWidget = qobject_cast<QWidget *>(o);
            if (pWidget != nullptr) {
                pWidget->setFocusPolicy(Qt::NoFocus);
                setWidgetAllPosterityNoFocus(pWidget);
            }
        }
    }
}

void Application::setWidgetAccesibleName(QWidget *w, const QString &name)
{
    if (w != nullptr) {
        w->setObjectName(name);
#ifdef ENABLE_ACCESSIBILITY
        w->setAccessibleName(name);
#endif
    }
}

bool Application::isTabletSystemEnvir()
{
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 5, 0, 0))
    return DGuiApplicationHelper::isTabletEnvironment()
#else
#ifdef ENABLE_TABLETSYSTEM
    return true;
#endif
    return false;
#endif
}

void Application::onAppQuit()
{
    emit quitRequest();
}

bool Application::isFileNameLegal(const QString &path, int *outErrorReson)
{
    if (path.isEmpty()) {
        if (outErrorReson != nullptr) {
            *outErrorReson = 0;
        }
        return false;
    }

    QRegExp regExp("[:\\*\\?\"<>\\|]");

    if (path.contains(regExp)) {

        if (outErrorReson != nullptr) {
            *outErrorReson  = 1;
        }
        return false;
    }

    QRegExp splitExp("[/\\\\]");


    int pos = splitExp.indexIn(path, 0);

    while (pos != -1) {
        QString dirStr = path.left(pos + 1);
        if (dirStr.count() > 1) {
            QDir dir(dirStr);
            if (!dir.exists()) {
                if (outErrorReson != nullptr) {
                    *outErrorReson  = 2;
                }
                return false;
            }
        }
        pos = splitExp.indexIn(path, pos + 1);
    }

    bool isdir = (path.endsWith('/') || path.endsWith('\\'));
    return !isdir;
}

void Application::onThemChanged(DGuiApplicationHelper::ColorType themeType)
{
    if (actWin != nullptr) {
        //actWin->slotOnThemeChanged(themeType);
    }
}

void Application::activateWindow()
{
    if (actWin) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}

void Application::showMainWindow(const QStringList &paths)
{
    MainWindow *w = new MainWindow(paths);

    actWin = w;

    //以dbus的方式传递命令
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("com.deepin.Draw");
    dbus.registerObject("/com/deepin/Draw", w);
    new dbusdraw_adaptor(w);

    // [BUG 27979]   need call show first otherwise due window max size icon show error
    w->show();
    w->readSettings();
}

//void Application::noticeFileRightProblem(const QStringList &problemfile, Application::EFileClassEnum classTp, bool checkQuit)
//{
//    if (problemfile.isEmpty())
//        return;

//    QWidget *pParent = actWin;

//    DDialog dia(pParent);
//    dia.setFixedSize(404, 163);
//    dia.setModal(true);
//    QString shortenFileName = QFontMetrics(dia.font()).elidedText(QFileInfo(problemfile.first()).fileName(), Qt::ElideMiddle, dia.width() / 2);

//    QString message;

//    switch (classTp) {
//    case ENotExist: {
//        message = tr("The file does not exist");
//        break;
//    }
//    case ENotFile:
//    case EDrawAppNotSup:
//        message = (problemfile.size() == 1 ?
//                   tr("Unable to open \"%1\", unsupported file format").arg(shortenFileName) :
//                   tr("Unsupported file format"));
//        break;
//    case EDrawAppSupButNotReadable:
//        message = (problemfile.size() == 1 ?
//                   tr("Unable to open the write-only file \"%1\"").arg(shortenFileName) :
//                   tr("Unable to open the write-only files"));
//        break;
//    default:
//        return;
//    }

//    dia.setMessage(message);
//    dia.setIcon(QPixmap(":/theme/common/images/deepin-draw-64.svg"));
//    dia.addButton(tr("OK"), true, DDialog::ButtonNormal);
//    //保持弹窗在主窗口中心
//    QPoint centerPos =  drawApp->topMainWindow()->mapToGlobal(drawApp->topMainWindow()->rect().center() - dia.rect().center());
//    dia.move(centerPos);
//    dia.exec();

//    if (checkQuit) {
//        //如果没有任何文件加载成功(没有view就表示没有任何文件加载成功)
//        if (pParent == nullptr) {
//            dApp->quit();
//        }
//    }
//}

void Application::onAttributionChanged(int attris, const QVariant &var,
                                       int phase, bool autoCmdStack)
{
    if (drawBoard() != nullptr && drawBoard()->currentPage() != nullptr) {
        auto tool = drawBoard()->currentPage()->currentTool_p();
        tool->setAttributionVar(attris, var, phase, autoCmdStack);

        if (var.isValid()) {
            _defaultAttriVars[currentDrawScence()][attris] = var;
            //currentDrawScence()->getDrawParam()->setDefaultAttri(attris, var);
        }
    }
}

void Application::onFocusChanged(QWidget *old, QWidget *now)
{
    qDebug() << "old = " << old << "now = " << now;
}

QVariant Application::defaultAttriVar(void *sceneKey, int attris)
{
    auto itF = _defaultAttriVars.find(sceneKey);
    if (itF != _defaultAttriVars.end()) {
        auto itff = itF.value().find(attris);
        if (itff != itF.value().end()) {
            return itff.value();
        }
    }
    return drawBoard()->attributionWidget()->defaultAttriVar(attris);
}

QVariant Application::currenDefaultAttriVar(int attris)
{
    return defaultAttriVar(currentDrawScence(), attris);
}

int Application::exeMessage(const QString &message,
                            Application::EMessageType msgTp,
                            bool autoFitDialogWidth,
                            const QStringList &moreBtns,
                            const QList<int> &btnType)
{
    DDialog dia(this->topMainWindowWidget());
    dia.setFixedSize(404, 163);
    dia.setModal(true);
    QString shortenFileName = autoFitDialogWidth ?
                              QFontMetrics(dia.font()).elidedText(message, Qt::ElideMiddle, dia.width() / 2) : message;
    dia.setMessage(shortenFileName);
    QString iconSvg;
    switch (msgTp) {
    case ENormalMsg:
        iconSvg = ":/theme/common/images/deepin-draw-64.svg";
        break;
    case EWarningMsg:
        iconSvg = ":/icons/deepin/builtin/Bullet_window_warning.svg";
        break;
    case EQuestionMsg:
        iconSvg = ":/icons/deepin/builtin/Bullet_window_warning.svg";
        break;
    }
    dia.setIcon(QPixmap(iconSvg));

    if (moreBtns.size() == btnType.size())
        for (int i = 0; i < moreBtns.size(); ++i)
            dia.addButton(moreBtns.at(i), false, DDialog::ButtonType(btnType.at(i)));

    return dia.exec();
}

bool Application::eventFilter(QObject *o, QEvent *e)
{
    return QObject::eventFilter(o, e);
}

void Application::loadTools()
{
    //defualt tools
    CDrawToolFactory::installTool(CDrawToolFactory::Create(selection));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(picture));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(rectangle));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(ellipse));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(triangle));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(polygonalStar));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(polygon));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(line));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(pen));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(text));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(blur));
    CDrawToolFactory::installTool(CDrawToolFactory::Create(cut));

#ifdef LOAD_TOOL_PLUGINS
    //load more tool plugin
    loadPluginTools();
#endif
}

void Application::loadPluginTools()
{
#ifdef LOAD_TOOL_PLUGINS
    //PLUGINTRANSPATH PLUGINPATH define in config.h(read from cmake var)

    //1.load translations first
    QDir dir(PLUGINTRANSPATH);
    if (dir.exists()) {
        QDirIterator qmIt(PLUGINTRANSPATH, QStringList() << QString("*%1.qm").arg(QLocale::system().name()), QDir::Files);
        while (qmIt.hasNext()) {
            qmIt.next();
            QFileInfo finfo = qmIt.fileInfo();
            QTranslator *translator = new QTranslator;
            if (translator->load(finfo.baseName(), finfo.absolutePath())) {
                _dApp->installTranslator(translator);
            }
        }
    }

    //2.load plugin
    QDir pluginLibDir(PLUGINPATH);
    if (pluginLibDir.exists()) {
        QDirIterator soIt(PLUGINPATH, QStringList() << "*.so", QDir::Files);
        while (soIt.hasNext()) {
            soIt.next();
            QFileInfo finfo = soIt.fileInfo();
            CDrawToolFactory::installTool(CDrawToolFactory::loadToolPlugin(finfo.filePath()));
        }
    }
#endif
}

bool Application::isFileExist(QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists()) {
        QUrl url(filePath);
        bool isExist = false;
        if (url.isLocalFile()) {
            filePath = url.toLocalFile();
            QFileInfo newf(filePath);
            if (newf.exists()) {
                isExist = true;
            }
        }
        return isExist;
    }
    return true;
}
