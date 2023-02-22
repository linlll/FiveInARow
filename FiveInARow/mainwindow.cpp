#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init();
    setWindowTitle(tr("五子棋"));
    setFixedSize(600, 370);
    btnStart = new QPushButton(tr("开始"), this);
    QFont font;
    font.setPointSize(8);
    btnStart->setFont(font);
    btnStart->setGeometry(400, 40, 80, 25);

    pte = new QPlainTextEdit(this);
    pte->setGeometry(400, 70, 180, 300);
    pte->setReadOnly(true);

    btnConnect = new QPushButton(tr("连接"), this);
    btnConnect->setFont(font);
    btnConnect->setGeometry(400, 10, 80, 25);
    btnDisconnect = new QPushButton(tr("断开连接"), this);
    btnDisconnect->setFont(font);
    btnDisconnect->setGeometry(500, 10, 80, 25);

    btnStart->setEnabled(false);
    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);

    tcpClient = new QTcpSocket(this);

    ConnectSignalSLots();
}

void MainWindow::ConnectSignalSLots() {
    connect(btnStart, SIGNAL(clicked()), this, SLOT(on_btnStart_clicked()));
    connect(btnConnect, SIGNAL(clicked()), this, SLOT(on_btnConnect_clicked()));
    connect(btnDisconnect, SIGNAL(clicked()), this, SLOT(on_btnDisconnect_clicked()));

    connect(tcpClient, SIGNAL(connected()), this, SLOT(on_connected()));
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(tcpClient, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(on_stateChanged()));
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(on_readyRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    drawMap(20, 10, 350);
    drawPiece(piece_pos, isFall, piece_color, 10);
    if (gameover) drawWinLine(winline);
}

void MainWindow::drawMap(int left, int top, int length) {
    // 15x15的棋盘
    // 外围线宽为4，中间的线宽为2
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(3);
    QPainter painter(this);
    painter.setPen(pen);
    // 1. 四周的线
    painter.drawLine(left, top, left, top + length);
    painter.drawLine(left, top + length, left + length, top + length);
    painter.drawLine(left + length, top + length, left + length, top);
    painter.drawLine(left + length, top, left, top);

    // 2. 中间的横线
    pen.setWidth(1);
    painter.setPen(pen);
    int l = length / 14;
    for (int i = l; i < length; i += l) {
        painter.drawLine(left, top + i, left + length, top + i);
    }
    // 3. 中间的竖线
    for (int i = l; i < length; i += l) {
        painter.drawLine(left + i, top, left + i, top + length);
    }

    // 4. 五个黑点
    QBrush brush(Qt::black, Qt::SolidPattern);
    painter.setBrush(brush);
    painter.drawEllipse(QPoint(3 * l + left, 3 * l + top), 4, 4);
    painter.drawEllipse(QPoint(3 * l + left, 11 * l + top), 4, 4);
    painter.drawEllipse(QPoint(11 * l + left, 3 * l + top), 4, 4);
    painter.drawEllipse(QPoint(11 * l + left, 11 * l + top), 4, 4);
    painter.drawEllipse(QPoint(7 * l + left, 7 * l + top), 4, 4);

    // 5. 棋子
    for(auto p : all_bpieces_pos) {
        drawPiece(p, true, Qt::black, 10);
    }
    for(auto p : all_wpieces_pos) {
        drawPiece(p, true, Qt::white, 10);
    }
}

void MainWindow::drawPiece(QPoint pos, bool isFall_, QColor color, int r) {
    if (pos.x() < 0 || pos.y() < 0) return;
    QPen pen;
    pen.setColor(color);
    pen.setWidth(2);
    QBrush brush;
    brush.setColor(color);
    Qt::BrushStyle bs = isFall_ ? Qt::SolidPattern : Qt::NoBrush;
    brush.setStyle(bs);
    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(brush);
    painter.drawEllipse(pos, r, r);
}

void MainWindow::drawWinLine(QLine line) {
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    QPainter painter(this);
    painter.setPen(pen);
    painter.drawLine(line);
}

void MainWindow::init() {
    piece_pos = QPoint(-1, -1);
    isFall = false;
    all_bpieces_pos.clear();
    all_wpieces_pos.clear();
    all_pieces = QVector<QVector<QColor>>(15, QVector<QColor>(15, Qt::yellow));
    gameover = true;
    repaint();
}

void MainWindow::on_btnStart_clicked() {
    init();
    gameover = false;
    repaint();
    pte->appendPlainText(tr("游戏开始\n"));
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (!my_turn) return;
    if (gameover) return;
    QPoint pos = event->pos();
    int x = pos.x(), y = pos.y();

    int X = int((x - 20 + 12.5) / 25);
    int Y = int((y - 10 + 12.5) / 25);

    x = X * 25 + 20;
    y = Y * 25 + 10;

    if (QPoint(x, y) == piece_pos) {
        isFall = true;
        if (isBlack) {
            all_bpieces_pos.push_back(piece_pos);
        } else {
            all_wpieces_pos.push_back(piece_pos);
        }
        all_pieces[Y][X] = piece_color;
        send(X, Y);
        my_turn = !my_turn;
        bool res = isWin(Y, X, piece_color);
        if (res) {
            gameover = true;
            pte->appendPlainText(piece_color == Qt::black ? tr("黑棋胜！") : tr("白棋胜！"));
        }
    } else {
        piece_pos.setX(x);
        piece_pos.setY(y);
        isFall = false;
    }
    repaint();
}

// 最需要判断最近的落子，附近有没有成型即可
bool MainWindow::isWin(int Y, int X, QColor color) {
    // 横
    int count = 0;
    int i, j;
    QPoint p1, p2;
    for (i = X; i >= 0 && all_pieces[Y][i] == color; i--) {
        count++;
    }
    p1.setX(i * 25 + 20); p1.setY(Y * 25 + 10);
    for (i = X; i <= 14 && all_pieces[Y][i] == color; i++) {
        count++;
    }
    p2.setX(i * 25 + 20); p2.setY(Y * 25 + 10);
    if (count == 6) {
        winline = QLine(p1, p2);
        return true;
    }
    // 竖
    count = 0;
    for (int i = Y; i >= 0 && all_pieces[i][X] == color; i--) {
        count++;
    }
    p1.setX(X * 25 + 20); p1.setY(i * 25 + 10);
    for (int i = Y; i <= 14 && all_pieces[i][X] == color; i++) {
        count++;
    }
    p2.setX(X * 25 + 20); p2.setY(i * 25 + 10);
    if (count == 6) {
        winline = QLine(p1, p2);
        return true;
    }
    // 左斜
    count = 0;
    for (i = Y, j = X; i >= 0 && j >= 0 && all_pieces[i][j] == color; i--, j--) {
        count++;
    }
    p1.setX(j * 25 + 20); p1.setY(i * 25 + 10);
    for (i = Y, j = X; i <= 14 && j <= 14 && all_pieces[i][j] == color; i++, j++) {
        count++;
    }
    p2.setX(j * 25 + 20); p2.setY(i * 25 + 10);
    if (count == 6) {
        winline = QLine(p1, p2);
        return true;
    }
    // 右斜
    count = 0;
    for (i = Y, j = X; i <= 14 && j >= 0 && all_pieces[i][j] == color; i++, j--) {
        count++;
    }
    p1.setX(j * 25 + 20); p1.setY(i * 25 + 10);
    for (i = Y, j = X; i >= 0 && j <= 14 && all_pieces[i][j] == color; i--, j++) {
        count++;
    }
    p2.setX(j * 25 + 20); p2.setY(i * 25 + 10);
    if (count == 6) {
        winline = QLine(p1, p2);
        return true;
    }
    return false;
}

void MainWindow::on_connected() {
    btnStart->setEnabled(true);
    btnConnect->setEnabled(false);
    btnDisconnect->setEnabled(true);
    pte->appendPlainText(tr("已连接"));
}

void MainWindow::on_disconnected() {
    btnStart->setEnabled(false);
    btnConnect->setEnabled(true);
    btnDisconnect->setEnabled(false);
    pte->appendPlainText(tr("已断开连接"));
    init();
    gameover = false;
    repaint();
}

void MainWindow::on_stateChanged() {

}

void MainWindow::on_readyRead() {
    while (tcpClient->canReadLine()) {
        QString msg = tcpClient->readLine();
        if (!msg.compare(QString("you first\n")) || !msg.compare(QString("you second\n"))) {
            isBlack = !msg.compare(QString("you first\n"));
            my_turn = isBlack;
            piece_color = isBlack ? Qt::black : Qt::white;
            pte->appendPlainText(msg);
            continue;
        }
        QStringList sl = msg.split(",");
        bool ok1, ok2;
        int x = sl[0].toInt(&ok1);
        if (!ok1) return;
        int y = sl[1].toInt(&ok2);
        if (!ok2) return;
        if (ok1 && ok2) my_turn = !my_turn;
        all_pieces[y][x] = isBlack ? Qt::white : Qt::black;
        bool res = isWin(y, x, isBlack ? Qt::white : Qt::black);
        if (res) {
            gameover = true;
            pte->appendPlainText(isBlack ? tr("白棋胜！") : tr("黑棋胜！"));
        }
        if (isBlack) all_wpieces_pos.push_back(QPoint(x * 25 + 20, y * 25 + 10));
        else all_bpieces_pos.push_back(QPoint(x * 25 + 20, y * 25 + 10));
        repaint();
    }
}

void MainWindow::on_btnConnect_clicked() {
    tcpClient->connectToHost(addr, port);
}

void MainWindow::on_btnDisconnect_clicked() {
    if (tcpClient->state() == QAbstractSocket::ConnectedState) {
        tcpClient->disconnectFromHost();
    }
}

void MainWindow::send(int x, int y) {
    QString msg = QString::asprintf("%d,%d", x, y);
    QByteArray ba = msg.toUtf8();
    ba.append('\n');
    tcpClient->write(ba);
}
