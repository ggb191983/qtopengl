#ifndef QMAINWINDOW_H
#define QMAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;


class qmainwindow : public QMainWindow
{
    Q_OBJECT

public:
    qmainwindow();

private:
    QPlainTextEdit *textEdit;
};

#endif // QMAINWINDOW_H
