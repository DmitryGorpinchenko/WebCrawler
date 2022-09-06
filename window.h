#pragma once

#include <QWidget>
#include <QPointer>

class Crawler;

class QTabWidget;
class QTextBrowser;
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

    void onUrlFound(const QString& url);
    void onUrlError(const QString& url, const QString& err_msg);
    void onProgressChanged(double val);

    QPushButton* start_btn;
    QPushButton* abort_btn;
    QPushButton* pause_btn;
    QPushButton* reset_btn;
    QLineEdit* start_url;
    QLineEdit* query_str;
    QLineEdit* urls_to_scan;
    QTextBrowser* result_list;
    QTextBrowser* issues_list;
    QTabWidget* tab_widget;
    double progress;

    QPointer<Crawler> crawler;
};
