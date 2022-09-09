#pragma once

#include <QObject>

#include <memory>

class QString;

class Crawler : public QObject {
    Q_OBJECT

public:
    explicit Crawler(const QString& start_url, const QString& query, int max_urls, int threads_num);
    ~Crawler() override;

    void start();
    void suspend();
    void resume();
    void quit();
    void wait();

signals:
    void urlLoading(const QString& url);
    void urlFound(const QString& url);
    void urlNotFound(const QString& url);
    void urlError(const QString& url, const QString& err_msg);
    void progress(double val);
    void finished();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};
