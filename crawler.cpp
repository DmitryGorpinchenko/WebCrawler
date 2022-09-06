#include "crawler.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#include <QString>
#include <QQueue>
#include <QSet>

#include <future>
#include <atomic>
#include <mutex>

// Crawler::Impl

struct Crawler::Impl {
    Crawler& owner;

    const QRegExp url_matcher;
    const QString query;
    const int max_urls;

    QNetworkAccessManager http;
    QQueue<QString> queue;
    QSet<QString> visited;

    std::atomic<bool> wait_for_resume;
    std::atomic<bool> request_abort;
    std::condition_variable cv;
    std::mutex m;
    std::future<void> ftr;

    Impl(Crawler& _owner, const QString& _start_url, const QString& _query, int _max_urls);
    ~Impl();

    void start();
    void suspend();
    void resume();
    void abort();

    void run();
    QNetworkReply* performGetRequest(const QString& url);
    void processUrl(const QString& url);
};

Crawler::Impl::Impl(Crawler& _owner, const QString& _start_url, const QString& _query, int _max_urls)
    : owner(_owner)
    , url_matcher(QLatin1String("HTTPS?://[A-Z0-9/._~:,;?!#\\[\\]@$&\\(\\)\\*\\+=-]+"), Qt::CaseInsensitive, QRegExp::RegExp)
    , query(_query)
    , max_urls(_max_urls)
    , wait_for_resume(false)
    , request_abort(false)
{
    queue.enqueue(_start_url);
    visited.insert(_start_url);
}

Crawler::Impl::~Impl()
{
    abort();
}

void Crawler::Impl::start()
{
    ftr = std::async(std::launch::async, [this]() { run(); });
}

void Crawler::Impl::suspend()
{
    wait_for_resume = true;
}

void Crawler::Impl::resume()
{
    {
        std::lock_guard lock(m);
        wait_for_resume = false;
    }
    cv.notify_all();
}

void Crawler::Impl::abort()
{
    {
        std::lock_guard lock(m);
        request_abort = true;
    }
    cv.notify_all();
}

void Crawler::Impl::run()
{
    emit owner.progress(0);

    int scanned_urls = 0;
    while (!queue.isEmpty() && scanned_urls < max_urls) {
        if (wait_for_resume) {
            std::unique_lock lock(m);
            cv.wait(lock, [this]() { return request_abort || (!wait_for_resume); });
        }
        if (request_abort) {
            break;
        }

        processUrl(queue.dequeue());

        emit owner.progress(static_cast<double>(++scanned_urls) / max_urls);
    }

    emit owner.progress(1);
    emit owner.finished();
}

QNetworkReply* Crawler::Impl::performGetRequest(const QString& url)
{
    auto reply = http.get(QNetworkRequest(QUrl(url)));
    QEventLoop eventLoop;
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    return reply;
}

void Crawler::Impl::processUrl(const QString& url)
{
    emit owner.urlLoading(url);
    auto reply = performGetRequest(url);
    if (reply->error() == QNetworkReply::NoError) {
        const QString content(reply->readAll());
        if (content.contains(query, Qt::CaseInsensitive)) {
            emit owner.urlFound(url);
        } else {
            emit owner.urlNotFound(url);
        }
        int cursor = 0;
        while ((cursor = url_matcher.indexIn(content, cursor)) != -1) {
            const auto neigh = url_matcher.cap(0);
            if (!visited.contains(neigh)) {
                queue.enqueue(neigh);
                visited.insert(neigh);
            }
            cursor += url_matcher.matchedLength();
        }
    } else {
        emit owner.urlError(url, reply->errorString());
    }
    reply->deleteLater();
}

// Crawler

Crawler::Crawler(const QString& start_url, const QString& query, int max_urls)
    : pimpl(new Impl(*this, start_url, query, max_urls))
{
}

Crawler::~Crawler() = default;

void Crawler::start()
{
    pimpl->start();
}

void Crawler::suspend()
{
    pimpl->suspend();
}

void Crawler::resume()
{
    pimpl->resume();
}

void Crawler::abort()
{
    pimpl->abort();
}
