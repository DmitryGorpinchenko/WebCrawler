#include "crawler.h"

#include <QColor>
#include <QEventLoop>
#include <QMutex>
#include <QNetworkReply>
#include <QQueue>

extern QWaitCondition cond;
extern QMutex mutex;
extern bool paused;
extern bool stoped;

Crawler::Crawler(const QString &startUrl, const QString &query, int maxUrls)
    : url_matcher(QStringLiteral("HTTPS?://[A-Z0-9/._~:,;?!#\\[\\]@$&\\(\\)\\*\\+=-]+"), Qt::CaseInsensitive, QRegExp::RegExp)
    , start_url(startUrl)
    , query(query)
    , max_urls(maxUrls)
{
    qRegisterMetaType<Status>("Status");
}

void Crawler::run()
{
    emit progressChanged(0);

    QNetworkAccessManager http;

    QQueue<QString> queue;
    queue.enqueue(start_url);
    QSet<QString> visited;
    visited.insert(start_url);

    int scanned_urls = 0;

    while (!queue.isEmpty() && scanned_urls < max_urls) {
        {
            QMutexLocker locker(&mutex);
            if (paused) {
                cond.wait(&mutex);
            }
            if (stoped) {
                break;
            }
        }

        const auto url = queue.dequeue();
        emit requestAboutToStart(url);

        auto reply = http.get(QNetworkRequest(QUrl(url)));
        QEventLoop eventLoop;
        QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();

        processReply(reply, &queue, &visited);

        emit progressChanged(static_cast<double>(++scanned_urls) / max_urls);
    }
    emit progressChanged(1.0);
    emit finished();
}

void Crawler::processReply(QNetworkReply *reply, QQueue<QString> *queue, QSet<QString> *visited)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString content(reply->readAll());
        const bool found = content.contains(query, Qt::CaseInsensitive);
        emit requestStatusChanged(found ? Status::TEXT_FOUND : Status::TEXT_ABSENT
                                , found ? QStringLiteral("Text FOUND") : QStringLiteral("Text NOT FOUND"));
        int cursor = 0;
        while ((cursor = url_matcher.indexIn(content, cursor)) != -1) {
            const auto neigh = url_matcher.cap(0);
            if (!visited->contains(neigh)) {
                queue->enqueue(neigh);
                visited->insert(neigh);
            }
            cursor += url_matcher.matchedLength();
        }
    } else {
        emit requestStatusChanged(Status::ERROR, reply->errorString());
    }
    reply->deleteLater();
}
