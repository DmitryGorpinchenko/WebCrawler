#pragma once

#include "crawler.h"

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QNetworkReply>
#include <QLineEdit>
#include <QWaitCondition>

class Window : public QWidget {
    Q_OBJECT
public:
    explicit Window(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *ev) override;

private:
    enum class State : int {
        RUN,
        SUSPEND,
        RESUME,
        STOP_REQUEST,
        STOP,
        UNDEFINED
    };

    void onStateChanged(State state);

    void start();
    void suspend();
    void resume();
    void stop();
    void finish();
    void reset();

    void initUI();

    void onUrlLoading(const QString &url);
    void onStatusChanged(Crawler::Status status, const QString &text);
    void onProgressChanged(double val);

    QPushButton *start_btn;
    QPushButton *stop_btn;
    QPushButton *pause_btn;
    QPushButton *reset_btn;
    QLineEdit *start_url;
    QLineEdit *text;
    QLineEdit *max_urls_num;
    QTextEdit *scanning_list;
    double progress;
};
