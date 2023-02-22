#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public: // 布局相关
    QPushButton *btnStart;
    QPlainTextEdit *pte;

    QPushButton *btnConnect;
    QPushButton *btnDisconnect;
public: // 网络相关
    QTcpSocket *tcpClient;
    const QString addr = "xxx.xxx.xxx.xxx";
    const quint16 port = 8888;

public: // 五子棋相关
    QPoint piece_pos;
    QColor piece_color;
    bool isBlack;
    bool isFall;
    bool my_turn;

    QVector<QPoint> all_bpieces_pos; // 所有黑棋的位置
    QVector<QPoint> all_wpieces_pos; // 所有白棋的位置
    QVector<QVector<QColor>> all_pieces; // 所有棋的位置

    bool gameover;
    QLine winline;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void ConnectSignalSLots();
    void drawMap(int left, int top, int length);
    void drawPiece(QPoint pos, bool isFall_, QColor color, int r);
    bool isWin(int Y, int X, QColor color);
    void drawWinLine(QLine line);
    void init();
private slots:
    void on_btnStart_clicked();
    void on_btnConnect_clicked();
    void on_btnDisconnect_clicked();

    void on_connected();
    void on_disconnected();
    void on_stateChanged();
    void on_readyRead();

    void send(int x, int y);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
