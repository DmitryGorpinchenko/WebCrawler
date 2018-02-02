#include "window.h"

#include <QPushButton>
#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QMutex>
#include <QPainter>
#include <QQueue>
#include <QUrl>

#include <thread>

extern QWaitCondition cond;
extern QMutex mutex;
extern bool paused;
extern bool stoped;

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    initUI();
}

void Window::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int full_angle = 360 * 16;
    int start_angle = -90 * 16;
    int span_angle = full_angle * progress;
    int width = 13;

    QRectF rectangle(60, 220, 300, 300);

    QPen pen;
    pen.setWidth(width);
    pen.setColor(Qt::white);
    p.setPen(pen);
    p.drawArc(rectangle, start_angle, full_angle);
    pen.setColor(Qt::darkGray);
    p.setPen(pen);
    p.drawArc(rectangle, start_angle, span_angle);

    QFont font;
    font.setPixelSize(40);
    p.setFont(font);
    p.drawText(rectangle, Qt::AlignCenter, QString("%1 %").arg(std::round(progress * 100)));
}

void Window::onStateChanged(State state) {
    switch (state) {
    case State::RUN:          start();   break;
    case State::SUSPEND:      suspend(); break;
    case State::RESUME:       resume();  break;
    case State::STOP_REQUEST: stop();    break;
    case State::STOP:         finish();  break;
    case State::UNDEFINED:    reset();   break;
    default:                  Q_UNREACHABLE();
    }
}

void Window::start()
{
    {
        QMutexLocker locker(&mutex);
        paused = false;
        stoped = false;
    }
    scanning_list->clear();
    start_url->setReadOnly(true);
    text->setReadOnly(true);
    max_urls_num->setReadOnly(true);
    start_btn->setEnabled(false);
    reset_btn->setEnabled(false);
    stop_btn->setEnabled(true);
    pause_btn->setEnabled(true);
    pause_btn->setText(QStringLiteral("Pause"));

    std::thread([this]() {
        Crawler crawler(start_url->text(), text->text(), max_urls_num->text().toInt());

        connect(&crawler, &Crawler::finished,             this, &Window::finish);
        connect(&crawler, &Crawler::progressChanged,      this, &Window::onProgressChanged);
        connect(&crawler, &Crawler::requestStatusChanged, this, &Window::onStatusChanged);
        connect(&crawler, &Crawler::requestAboutToStart,  this, &Window::onUrlLoading);

        crawler.run();
    }).detach();
}

void Window::suspend()
{
    {
        QMutexLocker locker(&mutex);
        paused = true;
    }
    pause_btn->setText(QStringLiteral("Resume"));
}

void Window::resume()
{
    {
        QMutexLocker locker(&mutex);
        paused = false;
        cond.wakeAll();
    }
    pause_btn->setText(QStringLiteral("Pause"));
}

void Window::stop()
{
    {
        QMutexLocker locker(&mutex);
        paused = false;
        stoped = true;
        cond.wakeAll();
    }
    pause_btn->setText(QStringLiteral("Pause"));
    pause_btn->setEnabled(false);
    stop_btn->setEnabled(false);
}

void Window::finish()
{
    start_url->setReadOnly(false);
    text->setReadOnly(false);
    max_urls_num->setReadOnly(false);
    start_btn->setEnabled(true);
    reset_btn->setEnabled(true);
    stop_btn->setEnabled(false);
    pause_btn->setEnabled(false);
}

void Window::reset()
{
    scanning_list->clear();
    start_url->clear();
    text->clear();
    max_urls_num->clear();
    start_btn->setEnabled(true);
    reset_btn->setEnabled(true);
    stop_btn->setEnabled(false);
    pause_btn->setEnabled(false);
    onProgressChanged(0.);
}

void Window::initUI()
{
    setFixedSize(1000, 600);

    start_btn = new QPushButton(QStringLiteral("Start"), this);
    stop_btn = new QPushButton(QStringLiteral("Stop"), this);
    pause_btn = new QPushButton(QStringLiteral("Pause"), this);
    reset_btn = new QPushButton(QStringLiteral("Reset"), this);

    const int w = 60, h = 30;
    start_btn->setGeometry(140, 110, w, h);
    stop_btn->setGeometry(210, 110, w, h);
    pause_btn->setGeometry(280, 110, w, h);
    reset_btn->setGeometry(350, 110, w, h);

    QFont font("Helvetica", 10, 60);

    start_url = new QLineEdit(this);
    start_url->setFont(font);
    start_url->setPlaceholderText("Start URL");
    start_url->setGeometry(10, 10, 400, 30);

    text = new QLineEdit(this);
    text->setFont(font);
    text->setPlaceholderText("Search text");
    text->setGeometry(10, 50, 400, 50);

    max_urls_num = new QLineEdit(this);
    max_urls_num->setFont(font);
    max_urls_num->setPlaceholderText("URLs to scan");
    max_urls_num->setGeometry(10, 110, 100, 30);

    scanning_list = new QTextEdit(this);
    scanning_list->setGeometry(450, 10, 500, 550);
    scanning_list->setFontWeight(70);
    scanning_list->setFontPointSize(10);
    scanning_list->setReadOnly(true);

    onStateChanged(State::UNDEFINED);

    connect(reset_btn, &QPushButton::clicked, this, [this]() { onStateChanged(State::UNDEFINED); });
    connect(start_btn, &QPushButton::clicked, this, [this]() { onStateChanged(State::RUN); });
    connect(stop_btn,  &QPushButton::clicked, this, [this]() { onStateChanged(State::STOP_REQUEST); });
    connect(pause_btn, &QPushButton::clicked, this, [this]() { onStateChanged(pause_btn->text() == QStringLiteral("Pause") ? State::SUSPEND : State::RESUME); });
}

void Window::onUrlLoading(const QString &url)
{
    scanning_list->setTextColor(Qt::darkGray);
    scanning_list->append(url);
    scanning_list->setTextColor(Qt::darkCyan);
    scanning_list->append(QStringLiteral("Loading ..."));
}

void Window::onStatusChanged(Crawler::Status status, const QString &text)
{
    // set cursor at the begining of line
    scanning_list->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    scanning_list->moveCursor(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
    scanning_list->moveCursor(QTextCursor::End, QTextCursor::KeepAnchor);
    scanning_list->textCursor().removeSelectedText();
    scanning_list->textCursor().deletePreviousChar();

    switch (status) {
    case Crawler::Status::TEXT_FOUND:  scanning_list->setTextColor(Qt::darkGreen);  break;
    case Crawler::Status::TEXT_ABSENT: scanning_list->setTextColor(Qt::darkYellow); break;
    case Crawler::Status::ERROR:       scanning_list->setTextColor(Qt::darkRed);    break;
    default:                           Q_UNREACHABLE();
    }

    scanning_list->append(text);
    scanning_list->append("");
}

void Window::onProgressChanged(double val)
{
    progress = val;
    update();
}
