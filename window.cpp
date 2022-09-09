#include "window.h"
#include "crawler.h"

#include <QListWidgetItem>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QPainter>

Window::Window(QWidget* parent)
    : QWidget(parent)
{
    initUI();
}

Window::~Window()
{
    delete crawler;
}

void Window::paintEvent(QPaintEvent* ev)
{
    Q_UNUSED(ev)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int full_angle = 360 * 16;
    const int start_angle = -90 * 16;
    const int span_angle = full_angle * progress;
    const int width = 13;

    const QRectF rectangle(60, 220, 300, 300);

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
    p.drawText(rectangle, Qt::AlignCenter, QString::fromLatin1("%1 %").arg(std::round(progress * 100)));
}

void Window::initUI()
{
    setFixedSize(1000, 600);

    start_url = new QLineEdit(this);
    query_str = new QLineEdit(this);
    urls_to_scan = new QLineEdit(this);
    start_btn = new QPushButton(QLatin1String("Start"), this);
    abort_btn = new QPushButton(QLatin1String("Abort"), this);
    pause_btn = new QPushButton(QLatin1String("Pause"), this);
    reset_btn = new QPushButton(QLatin1String("Reset"), this);
    results_view = new QListWidget(this);

    start_url->setGeometry(10, 10, 400, 30);
    query_str->setGeometry(10, 50, 400, 30);
    urls_to_scan->setGeometry(10, 90, 100, 30);
    start_btn->setGeometry(140, 90, 60, 30);
    abort_btn->setGeometry(210, 90, 60, 30);
    pause_btn->setGeometry(280, 90, 60, 30);
    reset_btn->setGeometry(350, 90, 60, 30);
    results_view->setGeometry(450, 10, 500, 550);

    const QFont font(QLatin1String("Helvetica"), 10, 60);
    start_url->setFont(font);
    query_str->setFont(font);
    urls_to_scan->setFont(font);

    start_url->setPlaceholderText(QLatin1String("Start URL"));
    query_str->setPlaceholderText(QLatin1String("Query"));
    urls_to_scan->setPlaceholderText(QLatin1String("URLs to scan"));

    reset();

    connect(start_btn, &QPushButton::clicked, this, [this]() { start(); });
    connect(abort_btn, &QPushButton::clicked, this, [this]() { abort(); });
    connect(pause_btn, &QPushButton::clicked, this, [this]() { pause(); });
    connect(reset_btn, &QPushButton::clicked, this, [this]() { reset(); });
}

void Window::start()
{
    Q_ASSERT(!crawler);

    items.clear();
    results_view->clear();
    updateRunningStatus(true);
    crawler = new Crawler(start_url->text(), query_str->text(), urls_to_scan->text().toInt());

    connect(crawler, &Crawler::urlLoading, this, &Window::onUrlLoading);
    connect(crawler, &Crawler::urlFound, this, &Window::onUrlFound);
    connect(crawler, &Crawler::urlNotFound, this, &Window::onUrlNotFound);
    connect(crawler, &Crawler::urlError, this, &Window::onUrlError);
    connect(crawler, &Crawler::progress, this, &Window::onProgressChanged);
    connect(crawler, &Crawler::finished, this, [this]() {
        crawler->deleteLater();
        crawler = nullptr;
        updateRunningStatus(false);
    });

    crawler->start();
}

void Window::abort()
{
    Q_ASSERT(crawler);
    crawler->abort();
}

void Window::pause()
{
    Q_ASSERT(crawler);
    const bool pause = pause_btn->text() == QLatin1String("Pause");
    if (pause) {
        crawler->suspend();
    } else {
        crawler->resume();
    }
    updatePauseStatus(!pause);
}

void Window::reset()
{
    Q_ASSERT(!crawler);
    items.clear();
    results_view->clear();
    start_url->clear();
    query_str->clear();
    urls_to_scan->clear();
    start_btn->setEnabled(true);
    reset_btn->setEnabled(true);
    abort_btn->setEnabled(false);
    pause_btn->setEnabled(false);
    onProgressChanged(0.);
}

void Window::updateRunningStatus(bool running)
{
    start_url->setReadOnly(running);
    query_str->setReadOnly(running);
    urls_to_scan->setReadOnly(running);
    start_btn->setEnabled(!running);
    reset_btn->setEnabled(!running);
    abort_btn->setEnabled(running);
    pause_btn->setEnabled(running);
    if (!running) {
        updatePauseStatus(true);
    }
}

void Window::updatePauseStatus(bool pause)
{
    pause_btn->setText(QLatin1String(pause ? "Pause" : "Resume"));
}

void Window::onUrlLoading(const QString& url)
{
    if (!items.contains(url)) {
        items.insert(url, new QListWidgetItem(url, results_view));
        results_view->scrollToBottom();
    }
}

void Window::onUrlFound(const QString& url)
{
    if (auto it = items.find(url); it != items.end()) {
        it.value()->setForeground(QColor("green"));
    }
}

void Window::onUrlNotFound(const QString& url)
{
    if (auto it = items.find(url); it != items.end()) {
        it.value()->setForeground(QColor("yellow"));
    }
}

void Window::onUrlError(const QString& url, const QString& err_msg)
{
    if (auto it = items.find(url); it != items.end()) {
        it.value()->setForeground(QColor("red"));
        it.value()->setToolTip(err_msg);
    }
}

void Window::onProgressChanged(double val)
{
    progress = val;
    update();
}
