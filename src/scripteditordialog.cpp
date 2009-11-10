#include "scripteditordialog.h"
#include "scripteditorcommandpython.cpp"

ScriptResult runPython(const QString &script, const QString &fileName)
{
    return pythonEngine->runPython(script, false, fileName);
}

ScriptResult runPythonExpression(const QString &expression)
{
    return pythonEngine->runPython(expression, true);
}

QString createPythonFromModel()
{
    QString str;

    // model
    str += "# model\n";
    str += QString("newdocument(\"%1\", \"%2\", \"%3\", %4, %5, \"%6\", %7, %8, %9, \"%10\", %11, %12, %13)").
           arg(Util::scene()->problemInfo()->name).
           arg(problemTypeToStringKey(Util::scene()->problemInfo()->problemType)).
           arg(physicFieldToStringKey(Util::scene()->problemInfo()->physicField())).
           arg(Util::scene()->problemInfo()->numberOfRefinements).
           arg(Util::scene()->problemInfo()->polynomialOrder).
           arg(adaptivityTypeToStringKey(Util::scene()->problemInfo()->adaptivityType)).
           arg(Util::scene()->problemInfo()->adaptivitySteps).
           arg(Util::scene()->problemInfo()->adaptivityTolerance).
           arg(Util::scene()->problemInfo()->frequency).
           arg(analysisTypeToStringKey(Util::scene()->problemInfo()->analysisType)).
           arg(Util::scene()->problemInfo()->timeStep).
           arg(Util::scene()->problemInfo()->timeTotal).
           arg(Util::scene()->problemInfo()->initialCondition)
           + "\n";
    str += "\n";

    // boundaries
    str += "# boundaries\n";
    for (int i = 1; i<Util::scene()->edgeMarkers.count(); i++)
    {
        str += Util::scene()->edgeMarkers[i]->script() + "\n";
    }
    str += "\n";

    // materials
    str += "# materials\n";
    for (int i = 1; i<Util::scene()->labelMarkers.count(); i++)
    {
        str += Util::scene()->labelMarkers[i]->script() + "\n";
    }
    str += "\n";

    // edges
    str += "# edges\n";
    for (int i = 0; i<Util::scene()->edges.count(); i++)
    {
        str += QString("addedge(%1, %2, %3, %4, %5, \"%6\")").
               arg(Util::scene()->edges[i]->nodeStart->point.x).
               arg(Util::scene()->edges[i]->nodeStart->point.y).
               arg(Util::scene()->edges[i]->nodeEnd->point.x).
               arg(Util::scene()->edges[i]->nodeEnd->point.y).
               arg(Util::scene()->edges[i]->angle).
               arg(Util::scene()->edges[i]->marker->name) + "\n";
    }
    str += "\n";

    // labels
    str += "# labels\n";
    for (int i = 0; i<Util::scene()->labels.count(); i++)
    {
        str += QString("addlabel(%1, %2, %3, \"%4\")").
               arg(Util::scene()->labels[i]->point.x).
               arg(Util::scene()->labels[i]->point.y).
               arg(Util::scene()->labels[i]->area).
               arg(Util::scene()->labels[i]->marker->name) + "\n";
    }

    return str;
}

ScriptEngineRemote::ScriptEngineRemote()
{
    // server
    m_server = new QLocalServer();
    QLocalServer::removeServer("agros2d-server");
    if (!m_server->listen("agros2d-server"))
    {
        cout << tr("Error: Unable to start the server (agros2d-server): %1.").arg(m_server->errorString()).toStdString() << endl;
        return;
    }

    connect(m_server, SIGNAL(newConnection()), this, SLOT(connected()));
}

ScriptEngineRemote::~ScriptEngineRemote()
{
    delete m_server;
    delete m_client_socket;
}

void ScriptEngineRemote::connected()
{
    command = "";

    m_server_socket = m_server->nextPendingConnection();
    connect(m_server_socket, SIGNAL(readyRead()), this, SLOT(readCommand()));
    connect(m_server_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void ScriptEngineRemote::readCommand()
{
    QTextStream in(m_server_socket);
    command = in.readAll();
}

void ScriptEngineRemote::disconnected()
{
    m_server_socket->deleteLater();

    ScriptResult result;
    if (!command.isEmpty())
    {
        result = runPython(command);
    }

    m_client_socket = new QLocalSocket();
    connect(m_client_socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(displayError(QLocalSocket::LocalSocketError)));

    m_client_socket->connectToServer("agros2d-client");
    if (m_client_socket->waitForConnected(1000))
    {
        QTextStream out(m_client_socket);
        out << result.text;
        out.flush();
        m_client_socket->waitForBytesWritten();
    }
    else
    {
        displayError(QLocalSocket::ConnectionRefusedError);
    }

    delete m_client_socket;
}

void ScriptEngineRemote::displayError(QLocalSocket::LocalSocketError socketError)
{
    switch (socketError) {
    case QLocalSocket::ServerNotFoundError:
        cout << tr("Server error: The host was not found.").toStdString() << endl;
        break;
    case QLocalSocket::ConnectionRefusedError:
        cout << tr("Server error: The connection was refused by the peer. Make sure the agros2d-client server is running.").toStdString() << endl;
        break;
    default:
        cout << tr("Server error: The following error occurred: %1.").arg(m_client_socket->errorString()).toStdString() << endl;
    }
}

// ***************************************************************************************************************************

ScriptEditorWidget::ScriptEditorWidget(QWidget *parent) : QWidget(parent)
{
    file = "";

    txtEditor = new ScriptEditor(this);
    txtOutput = new QPlainTextEdit(this);
    splitter = new QSplitter(this);

    createControls();    

    QSettings settings;
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("ScriptEditorDialog/SplitterState", splitter->saveState()).toByteArray());
}

ScriptEditorWidget::~ScriptEditorWidget()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("ScriptEditorDialog/SplitterState", splitter->saveState());

    delete txtEditor;
    delete txtOutput;
    delete splitter;
}

void ScriptEditorWidget::createControls()
{
    txtOutput->setFont(QFont("Monospaced", 10));
    txtOutput->setReadOnly(true);

    // contents
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(txtEditor);
    splitter->addWidget(txtOutput);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(splitter);

    setLayout(layout);

    QSettings settings;
    splitter->restoreGeometry(settings.value("ScriptEditorDialog/Splitter", splitter->saveGeometry()).toByteArray());
}

void ScriptEditorWidget::doRunPython(const QString &script)
{
    QString scriptContent;
    if (script.isEmpty())
        scriptContent = txtEditor->toPlainText();
    else
        scriptContent = script;

    ScriptResult result = runPython(scriptContent, file);
    txtOutput->setPlainText(result.text);
}

void ScriptEditorWidget::doCreatePythonFromModel()
{
    txtEditor->setPlainText(createPythonFromModel());
}

// ***********************************************************************************************************

ScriptEditorDialog::ScriptEditorDialog(SceneView *sceneView, QWidget *parent) : QMainWindow(parent)
{
    QSettings settings;
    restoreGeometry(settings.value("ScriptEditorDialog/Geometry", saveGeometry()).toByteArray());
    recentFiles = settings.value("ScriptEditorDialog/RecentFiles").value<QStringList>();

    m_sceneView = sceneView;

    setWindowIcon(icon("script"));

    searchDialog = new SearchDialog(this);

    createActions();
    createControls();

    pythonEngine = new PythonEngine();

    restoreState(settings.value("ScriptEditorDialog/State", saveState()).toByteArray());

    setMinimumSize(600, 400);
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    QSettings settings;
    settings.setValue("ScriptEditorDialog/Geometry", saveGeometry());
    settings.setValue("ScriptEditorDialog/State", saveState());
    settings.setValue("ScriptEditorDialog/RecentFiles", recentFiles);

    delete pythonEngine;
}

void ScriptEditorDialog::showDialog()
{
    show();
    activateWindow();
    raise();
    txtEditor->setFocus();
}

void ScriptEditorDialog::runScript(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text))
        {
            // run script
            ScriptResult result = runPython(file.readAll(), fileName);
            if (result.isError)
                QMessageBox::critical(QApplication::activeWindow(), "Error", result.text);
            else if (!result.text.isEmpty())
                QMessageBox::information(QApplication::activeWindow(), "Message", result.text);
        }
        file.close();
    }
}

void ScriptEditorDialog::runCommand(const QString &command)
{
    if (!command.isEmpty())
    {
        // run script
        ScriptResult result = runPython(command);
        if (result.isError)
            QMessageBox::critical(QApplication::activeWindow(), "Error", result.text);
        else if (!result.text.isEmpty())
            QMessageBox::information(QApplication::activeWindow(), "Message", result.text);
    }
}

void ScriptEditorDialog::createActions()
{
    actFileNew = new QAction(icon("document-new"), tr("&New"), this);
    actFileNew->setShortcuts(QKeySequence::New);
    connect(actFileNew, SIGNAL(triggered()), this, SLOT(doFileNew()));

    actFileOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actFileOpen->setShortcuts(QKeySequence::Open);
    connect(actFileOpen, SIGNAL(triggered()), this, SLOT(doFileOpen()));

    actFileSave = new QAction(icon("document-save"), tr("&Save"), this);
    actFileSave->setShortcuts(QKeySequence::Save);
    connect(actFileSave, SIGNAL(triggered()), this, SLOT(doFileSave()));

    actFileSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actFileSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actFileSaveAs, SIGNAL(triggered()), this, SLOT(doFileSaveAs()));

    actFileOpenRecentGroup = new QActionGroup(this);
    connect(actFileOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doFileOpenRecent(QAction *)));

    actFileClose = new QAction(icon(""), tr("Close"), this);
    actFileClose->setShortcuts(QKeySequence::Close);
    connect(actFileClose, SIGNAL(triggered()), this, SLOT(doFileClose()));

    actUndo = new QAction(icon("edit-undo"), tr("&Undo"), this);
    actUndo->setShortcut(QKeySequence::Undo);

    actRedo = new QAction(icon("edit-redo"), tr("&Redo"), this);
    actRedo->setShortcut(QKeySequence::Redo);

    actCut = new QAction(icon("edit-cut"), tr("Cu&t"), this);
    actCut->setShortcut(QKeySequence::Cut);
    actCut->setEnabled(false);

    actCopy = new QAction(icon("edit-copy"), tr("&Copy"), this);
    actCopy->setShortcut(QKeySequence::Copy);
    actCopy->setEnabled(false);

    actPaste = new QAction(icon("edit-paste"), tr("&Paste"), this);
    actPaste->setShortcut(QKeySequence::Paste);

    actFind = new QAction(icon("edit-find"), tr("&Find"), this);
    actFind->setShortcut(QKeySequence::Find);
    connect(actFind, SIGNAL(triggered()), this, SLOT(doFind()));

    actFindNext = new QAction(icon("edit-find"), tr("&Find next"), this);
    actFindNext->setShortcut(QKeySequence::FindNext);
    connect(actFindNext, SIGNAL(triggered()), this, SLOT(doFindNext()));

    actReplace = new QAction(icon("edit-find-replace"), tr("&Replace"), this);
    actReplace->setShortcut(QKeySequence::Replace);
    connect(actReplace, SIGNAL(triggered()), this, SLOT(doReplace()));

    actRunPython = new QAction(icon("script-python"), tr("&Run Python script"), this);
    actRunPython->setShortcut(QKeySequence(tr("Ctrl+R")));

    actCreateFromModel = new QAction(icon("script-create"), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setStatusTip(tr("Exit script editor"));
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));
}

void ScriptEditorDialog::createControls()
{
    mnuRecentFiles = new QMenu(tr("Recent files"), this);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actFileNew);
    mnuFile->addAction(actFileOpen);
    mnuFile->addAction(actFileSave);
    mnuFile->addAction(actFileSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actFileClose);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actExit);

    mnuEdit = menuBar()->addMenu(tr("&Edit"));
    mnuEdit->addAction(actUndo);
    mnuEdit->addAction(actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCut);
    mnuEdit->addAction(actCopy);
    mnuEdit->addAction(actPaste);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actFind);
    mnuEdit->addAction(actFindNext);
    mnuEdit->addAction(actReplace);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actRunPython);
    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);

    tlbFile = addToolBar(tr("File"));
    tlbFile->setObjectName("File");
    tlbFile->addAction(actFileNew);
    tlbFile->addAction(actFileOpen);
    tlbFile->addAction(actFileSave);

    tlbEdit = addToolBar(tr("Edit"));
    tlbEdit->setObjectName("Edit");
    tlbEdit->addAction(actUndo);
    tlbEdit->addAction(actRedo);
    tlbEdit->addSeparator();
    tlbEdit->addAction(actCut);
    tlbEdit->addAction(actCopy);
    tlbEdit->addAction(actPaste);

    tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->addAction(actRunPython);
    tlbTools->addSeparator();
    tlbTools->addAction(actCreateFromModel);

    // contents
    tabWidget = new QTabWidget;
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);

    QToolButton *btnNewTab = new QToolButton(this);
    btnNewTab->setAutoRaise(true);
    btnNewTab->setToolTip(tr("Add new page"));
    btnNewTab->setIcon(icon("tabadd"));
    tabWidget->setCornerWidget(btnNewTab, Qt::TopLeftCorner);
    connect(btnNewTab, SIGNAL(clicked()), this, SLOT(doFileNew()));

    doFileNew();

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(doCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(doCurrentPageChanged(int)));

    // main widget
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(tabWidget);
    layout->setMargin(6);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    // recent files
    setRecentFiles();

    setCentralWidget(widget);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(doDataChanged()));
}

void ScriptEditorDialog::doFileNew()
{
    tabWidget->addTab(new ScriptEditorWidget(this), tr("Untitled"));
    tabWidget->setCurrentIndex(tabWidget->count()-1);
    doCurrentPageChanged(tabWidget->count()-1);
}

void ScriptEditorDialog::doFileOpen(const QString &file)
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    // open dialog
    QString fileName = file;
    if (fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "data", tr("Python files (*.py)"));

    // read text
    if (!fileName.isEmpty()) {
        for (int i = 0; i < tabWidget->count(); i++)
        {
            ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->widget(i));
            if (scriptEditorWidget->file == fileName)
            {
                tabWidget->setCurrentIndex(i);
                QMessageBox::information(this, tr("Information"), tr("Script is already opened."));
                return;
            }
        }

        // check empty document
        if (!scriptEditorWidget->txtEditor->toPlainText().isEmpty())
        {
            doFileNew();
            // new widget
            scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());
        }

        scriptEditorWidget->file = fileName;
        txtEditor->setPlainText(readFileContent(scriptEditorWidget->file));

        setRecentFiles();

        QFileInfo fileInfo(scriptEditorWidget->file);
        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());

        doCurrentPageChanged(tabWidget->currentIndex());
    }
}

void ScriptEditorDialog::doFileOpenRecent(QAction *action)
{
    QString fileName = action->text();
    if (QFile::exists(fileName))
    {
        doFileOpen(fileName);
        setRecentFiles();
    }
}

void ScriptEditorDialog::doFileSave()
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    // save dialog
    if (scriptEditorWidget->file.isEmpty())
        scriptEditorWidget->file = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Python files (*.py)"));

    // write text
    if (!scriptEditorWidget->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget->file);
        if (fileInfo.suffix() != "py") scriptEditorWidget->file += ".py";

        QFile fileName(dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget())->file);
        if (fileName.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&fileName);
            out << txtEditor->toPlainText();
            fileName.close();
        }

        setRecentFiles();

        tabWidget->setTabText(tabWidget->currentIndex(), fileInfo.baseName());
    }
}

void ScriptEditorDialog::doFileSaveAs()
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    scriptEditorWidget->file = QFileDialog::getSaveFileName(this, tr("Save file"), "data", tr("Python files (*.py)"));
    doFileSave();
}

void ScriptEditorDialog::doFileClose()
{
    if (tabWidget->count() > 1)
        doCloseTab(tabWidget->currentIndex());
    else
        hide();
}

void ScriptEditorDialog::doFind()
{
    if (searchDialog->showDialogFind() == QDialog::Accepted)
    {
        doFindNext(true);
    }
    searchDialog->hide();
}

void ScriptEditorDialog::doFindNext(bool fromBegining)
{
    if (!searchDialog->searchString().isEmpty())
    {
        QString search = searchDialog->searchString();

        QTextDocument::FindFlags flags;
        if (searchDialog->caseSensitive())
            flags |= QTextDocument::FindCaseSensitively;

        // Search
        QTextCursor cursor = txtEditor->textCursor();
        if (searchDialog->searchStringIsRegExp())
        {
            QRegExp searchReg(search);
            if (fromBegining)
                cursor = txtEditor->document()->find(searchReg, flags);
            else
                cursor = txtEditor->document()->find(searchReg, cursor, flags);
        }
        else
        {
            if (fromBegining)
                cursor = txtEditor->document()->find(search, flags);
            else
                cursor = txtEditor->document()->find(search, cursor, flags);
        }

        if (cursor.position() >= 0)
            txtEditor->setTextCursor(cursor);
        txtEditor->setFocus();
    }
}

void ScriptEditorDialog::doReplace()
{
    if (searchDialog->showDialogReplace() == QDialog::Accepted)
    {
        if (!searchDialog->searchString().isEmpty())
        {
            QString search = searchDialog->searchString();
            QString replace = searchDialog->replaceString();

            QTextCursor cursor = txtEditor->textCursor();

            QString text = txtEditor->document()->toPlainText();
            text.replace(search, replace, (searchDialog->caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive));
            txtEditor->document()->setPlainText(text);

            txtEditor->setTextCursor(cursor);
        }
    }
    searchDialog->hide();
}

void ScriptEditorDialog::doDataChanged()
{
    actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
}

void ScriptEditorDialog::doHelp()
{
    Util::helpDialog()->showPage("scripting/scripting.html");
    Util::helpDialog()->show();
}

void ScriptEditorDialog::doCloseTab(int index)
{
    tabWidget->removeTab(index);
}

void ScriptEditorDialog::doCurrentPageChanged(int index)
{
    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());
    txtEditor = scriptEditorWidget->txtEditor;

    actRunPython->disconnect();
    connect(actRunPython, SIGNAL(triggered()), scriptEditorWidget, SLOT(doRunPython()));
    actCreateFromModel->disconnect();
    connect(actCreateFromModel, SIGNAL(triggered()), scriptEditorWidget, SLOT(doCreatePythonFromModel()));

    actCut->disconnect();
    connect(actCut, SIGNAL(triggered()), txtEditor, SLOT(cut()));
    actCopy->disconnect();
    connect(actCopy, SIGNAL(triggered()), txtEditor, SLOT(copy()));
    actPaste->disconnect();
    connect(actPaste, SIGNAL(triggered()), txtEditor, SLOT(paste()));
    actUndo->disconnect();
    connect(actUndo, SIGNAL(triggered()), txtEditor, SLOT(undo()));
    actRedo->disconnect();
    connect(actRedo, SIGNAL(triggered()), txtEditor, SLOT(redo()));

    txtEditor->document()->disconnect(actUndo);
    txtEditor->document()->disconnect(actRedo);
    connect(txtEditor->document(), SIGNAL(undoAvailable(bool)), actUndo, SLOT(setEnabled(bool)));
    connect(txtEditor->document(), SIGNAL(redoAvailable(bool)), actRedo, SLOT(setEnabled(bool)));
    txtEditor->disconnect(actCut);
    txtEditor->disconnect(actCopy);
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCut, SLOT(setEnabled(bool)));
    connect(txtEditor, SIGNAL(copyAvailable(bool)), actCopy, SLOT(setEnabled(bool)));

    actUndo->setEnabled(txtEditor->document()->isUndoAvailable());
    actRedo->setEnabled(txtEditor->document()->isRedoAvailable());

    tabWidget->setTabsClosable(tabWidget->count() > 1);
    tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(true);

    QString fileName = tr("Untitled");
    if (!scriptEditorWidget->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget->file);
        fileName = fileInfo.completeBaseName();
    }
    setWindowTitle(tr("Script editor - %1").arg(fileName));

    txtEditor->setFocus();
}

void ScriptEditorDialog::setRecentFiles()
{
    if (!tabWidget) return;

    ScriptEditorWidget *scriptEditorWidget = dynamic_cast<ScriptEditorWidget *>(tabWidget->currentWidget());

    // recent files
    if (!scriptEditorWidget->file.isEmpty())
    {
        QFileInfo fileInfo(scriptEditorWidget->file);
        if (recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            recentFiles.move(recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);

        while (recentFiles.count() > 15) recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(recentFiles[i], this);
        actFileOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

// ******************************************************************************************************

ScriptEditor::ScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

    setFont(QFont("Monospace", 10));
    setTabStopWidth(40);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    // highlighter
    new QScriptSyntaxHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(doUpdateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(doUpdateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(doHighlightCurrentLine()));

    doUpdateLineNumberAreaWidth(0);
    doHighlightCurrentLine();
}

ScriptEditor::~ScriptEditor()
{
    delete lineNumberArea;
}

int ScriptEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::doUpdateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::doUpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        doUpdateLineNumberAreaWidth(0);
}

void ScriptEditor::doHighlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

// ***********************************************************************************************

SearchDialog::SearchDialog(QWidget *parent): QDialog(parent)
{
    // Title
    setWindowTitle(tr("Search and replace"));
    setWindowIcon(icon("edit-find"));
    setModal(true);

    // Find and replace
    QGroupBox *findReplaceGroup = new QGroupBox(this);
    findReplaceGroup->setTitle(tr("Find and replace"));

    txtFind = new QLineEdit();
    txtReplace = new QLineEdit();

    QGridLayout *findReplaceLayout = new QGridLayout();
    findReplaceGroup->setLayout(findReplaceLayout);
    findReplaceLayout->addWidget(new QLabel(tr("Search for:")), 0, 0);
    findReplaceLayout->addWidget(txtFind, 0, 1);
    findReplaceLayout->addWidget(new QLabel(tr("Replace with:")), 1, 0);
    findReplaceLayout->addWidget(txtReplace, 1, 1);

    // Options
    QGroupBox *optionsGroup = new QGroupBox(this);
    optionsGroup->setTitle(tr("Options"));

    chkSearchRegExp = new QCheckBox();
    chkSearchRegExp->setText(tr("Regular expression"));

    chkCaseSensitive = new QCheckBox(optionsGroup);
    chkCaseSensitive->setText(tr("Case sensitive"));

    QVBoxLayout *optionsLayout = new QVBoxLayout();
    optionsGroup->setLayout(optionsLayout);
    optionsLayout->addWidget(chkSearchRegExp);
    optionsLayout->addWidget(chkCaseSensitive);

    // Buttons
    btnConfirm = new QPushButton(this);
    btnConfirm->setDefault(true);
    connect(btnConfirm, SIGNAL(clicked()), this, SLOT(accept()));

    btnCancel = new QPushButton(this);
    btnCancel->setText(tr("Cancel"));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnConfirm);
    buttonsLayout->addWidget(btnCancel);

    // Layout
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(findReplaceGroup);
    layout->addWidget(optionsGroup);
    layout->addLayout(buttonsLayout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

SearchDialog::~SearchDialog()
{
    delete txtFind;
    delete txtReplace;

    delete chkSearchRegExp;
    delete chkCaseSensitive;

    delete btnCancel;
    delete btnConfirm;
}

int SearchDialog::showDialogFind()
{
    txtFind->setFocus();
    txtReplace->setEnabled(false);
    btnConfirm->setText(tr("Find"));
    chkSearchRegExp->setEnabled(true);

    return exec();
}

int SearchDialog::showDialogReplace()
{
    txtFind->setFocus();
    txtReplace->setEnabled(true);
    btnConfirm->setText(tr("Replace"));
    chkSearchRegExp->setEnabled(false);

    return exec();
}
