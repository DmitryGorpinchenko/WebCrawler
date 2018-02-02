#pragma once

#include <QNetworkReply>
#include <QObject>
#include <QWaitCondition>

class Crawler : public QObject {
    Q_OBJECT
public:
    explicit Crawler(const QString &start_url, const QString &query, int max_urls);

    enum class Status : int {
        TEXT_FOUND,
        TEXT_ABSENT,
        ERROR
    };

    void run();
signals:
    void progressChanged(double val);
    void requestStatusChanged(Status status, const QString &text);
    void requestAboutToStart(const QString &url);
    void finished();

private:
    void processReply(QNetworkReply *reply, QQueue<QString> *queue, QSet<QString> *visited);

    QRegExp url_matcher;
    QString start_url;
    QString query;
    int max_urls;
};
