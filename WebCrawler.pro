TEMPLATE = app
TARGET = WebCrawler

QT = core gui

QT += network

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    window.cpp \
    crawler.cpp \
    worker_pool.cpp

HEADERS +=  \
    window.h \
    crawler.h \
    worker_pool.h
