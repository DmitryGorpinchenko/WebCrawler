#pragma once

#include <QObject>

#include <memory>

enum class RequestStatus : int {
    TEXT_FOUND,
    TEXT_NOT_FOUND,
    ERROR
};

class QString;

class Crawler : public QObject {
    Q_OBJECT

public:
    explicit Crawler(const QString& start_url, const QString& query, int max_urls);
    ~Crawler() override;

    void start();
    void suspend();
    void resume();
    void abort();

signals:
    void requestAboutToStart(const QString& url);
    void requestCompleted(RequestStatus status, const QString& text);
    void progress(double val);
    void finished();

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl;
};
