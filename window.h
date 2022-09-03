#pragma once

#include <QWidget>
#include <QPointer>

enum class RequestStatus : int;
class Crawler;

class QTextEdit;
class QLineEdit;
class QPushButton;

class Window : public QWidget {
public:
    explicit Window(QWidget* parent = nullptr);
    ~Window() override;

protected:
    void paintEvent(QPaintEvent* ev) override;

private:
    void initUI();

    void start();
    void abort();
    void pause();
    void reset();

    void updateRunningStatus(bool running);
    void updatePauseStatus(bool pause);

    void onUrlLoading(const QString& url);
    void onStatusChanged(RequestStatus status, const QString& text);
    void onProgressChanged(double val);

    QPushButton* start_btn;
    QPushButton* abort_btn;
    QPushButton* pause_btn;
    QPushButton* reset_btn;
    QLineEdit* start_url;
    QLineEdit* query_str;
    QLineEdit* urls_to_scan;
    QTextEdit* log;
    double progress;

    QPointer<Crawler> crawler;
};
