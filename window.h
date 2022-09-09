#pragma once

#include <QWidget>
#include <QPointer>
#include <QHash>

class Crawler;

class QListWidgetItem;
class QListWidget;
class QLineEdit;
class QPushButton;

class Window : public QWidget {
public:
    explicit Window(QWidget* parent = nullptr);
    ~Window() override;

protected:
    void paintEvent(QPaintEvent* ev) override;

private:
    void init();

    void start();
    void abort();
    void pause();
    void reset();

    void updateRunningStatus(bool running);
    void updatePauseStatus(bool pause);

    void onUrlLoading(const QString& url);
    void onUrlFound(const QString& url);
    void onUrlNotFound(const QString& url);
    void onUrlError(const QString& url, const QString& err_msg);
    void onProgressChanged(double val);

    QPushButton* start_btn;
    QPushButton* abort_btn;
    QPushButton* pause_btn;
    QPushButton* reset_btn;
    QLineEdit* start_url;
    QLineEdit* query_str;
    QLineEdit* urls_to_scan;
    QLineEdit* threads_num;
    QListWidget* results_view;
    QHash<QString, QListWidgetItem*> items;
    double progress;

    QPointer<Crawler> crawler;
};
