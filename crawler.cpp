#include "crawler.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegExp>
#include <QString>
#include <QQueue>
#include <QSet>

#include <thread>
#include <mutex>

//

struct Crawler::Impl {
    Crawler& owner;

    const QString query;
    const int max_urls;
    const int threads_num;

    QQueue<QString> queue;
    QSet<QString> visited;
    QSet<QNetworkReply*> pending_replies;
    int scanned_urls;

    bool wait_for_resume;
    bool request_quit;
    std::condition_variable cv;
    std::mutex m;
    std::vector<std::thread> workers;

    Impl(Crawler& _owner, const QString& _start_url, const QString& _query, int _max_urls, int _threads_num);
    ~Impl();

    void start();
    void suspend();
    void resume();
    void quit();
    void wait();
};

Crawler::Impl::Impl(Crawler& _owner, const QString& _start_url, const QString& _query, int _max_urls, int _threads_num)
    : owner(_owner)
    , query(_query)
    , max_urls(_max_urls)
    , threads_num(_threads_num)
    , scanned_urls(0)
    , wait_for_resume(false)
    , request_quit(false)
{
    Q_ASSERT(max_urls > 0);
    Q_ASSERT(threads_num > 0);

    queue.enqueue(_start_url);
    visited.insert(_start_url);
}

Crawler::Impl::~Impl()
{
    quit();
    wait();
}

void Crawler::Impl::start()
{
    emit owner.progress(0);

    struct Finalizer {
        Finalizer(Crawler& _crawler)
            : crawler(_crawler)
        {
        }
        ~Finalizer()
        {
            emit crawler.finished();
        }
        Crawler& crawler;
    };
    auto fin = std::make_shared<Finalizer>(owner); // last exited worker will destroy it

    for (int i = 0; i < threads_num; ++i) {
        workers.push_back(std::thread([this, fin]() mutable {
            const QRegExp matcher(QLatin1String("HTTPS?://[A-Z0-9/._~:,;?!#\\[\\]@$&\\(\\)\\*\\+=-]+"), Qt::CaseInsensitive, QRegExp::RegExp);
            QNetworkAccessManager http;

            struct Response {
                QString data;
                QNetworkReply::NetworkError error;
            };
            const auto performGetRequest = [this, &http](const QString& url) {
                emit owner.urlLoading(url);
                auto reply = http.get(QNetworkRequest(QUrl(url)));
                QEventLoop event_loop;
                QObject::connect(reply, &QNetworkReply::finished, &event_loop, &QEventLoop::quit);
                {
                    std::lock_guard guard(m);
                    pending_replies.insert(reply);
                }
                event_loop.exec();
                {
                    std::lock_guard guard(m);
                    pending_replies.remove(reply);
                }
                const auto error = reply->error();
                Response res;
                res.data = (error == QNetworkReply::NoError) ? reply->readAll() : reply->errorString();
                res.error = error;
                reply->deleteLater();
                return res;
            };
            const auto getNextUrl = [this]() {
                std::unique_lock lock(m);
                cv.wait(lock, [this]() { return (!queue.isEmpty()) || request_quit; });
                return (!request_quit) ? queue.dequeue() : QString();
            };
            const auto addLinks = [this, &matcher](const Response& response) {
                for (int cursor = matcher.indexIn(response.data);
                     cursor != -1;
                     cursor = matcher.indexIn(response.data, cursor + matcher.matchedLength()))
                {
                    const auto link = matcher.cap(0);
                    {
                        std::lock_guard guard(m);
                        if (visited.size() == max_urls) {
                            break;
                        }
                        if (visited.contains(link)) {
                            continue;
                        }
                        visited.insert(link);
                        queue.enqueue(link);
                    }
                    cv.notify_one();
                }
            };
            const auto notifyUrlStatus = [this](const QString& url, const Response& response) {
                {
                    std::unique_lock lock(m);
                    cv.wait(lock, [this]() { return (!wait_for_resume) || request_quit; });
                    if (request_quit) {
                        return;
                    }
                }
                if (response.error == QNetworkReply::NoError) {
                    if (response.data.contains(query, Qt::CaseInsensitive)) {
                        emit owner.urlFound(url);
                    } else {
                        emit owner.urlNotFound(url);
                    }
                } else {
                    emit owner.urlError(url, response.data);
                }
            };
            const auto notifyProgress = [this]() {
                {
                    std::lock_guard guard(m);
                    emit owner.progress(static_cast<double>(++scanned_urls) / max_urls);
                    if (scanned_urls < max_urls) {
                        return;
                    }
                    request_quit = true;
                }
                cv.notify_all();
            };
            const auto processUrl = [performGetRequest, addLinks, notifyUrlStatus, notifyProgress](const QString& url) {
                const auto response = performGetRequest(url);
                notifyUrlStatus(url, response);
                if (response.error == QNetworkReply::NoError) {
                    addLinks(response);
                }
                notifyProgress();
            };

            while (true) {
                if (const auto url = getNextUrl(); !url.isNull()) {
                    processUrl(url);
                } else {
                    break;
                }
            }

            fin.reset();
        }));
    }
}

void Crawler::Impl::suspend()
{
    std::lock_guard lock(m);
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

void Crawler::Impl::quit()
{
    {
        std::lock_guard lock(m);
        for (auto r : pending_replies) {
            r->close();
        }
        request_quit = true;
    }
    cv.notify_all();
}

void Crawler::Impl::wait()
{
    for (auto& w : workers) {
        if (w.joinable()) {
            w.join();
        }
    }
}

//

Crawler::Crawler(const QString& start_url, const QString& query, int max_urls, int threads_num)
    : pimpl(new Impl(*this, start_url, query, max_urls, threads_num))
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

void Crawler::quit()
{
    pimpl->quit();
}

void Crawler::wait()
{
    pimpl->wait();
}
