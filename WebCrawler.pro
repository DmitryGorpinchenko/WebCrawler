TEMPLATE = app
TARGET = WebCrawler

QT = core gui

QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    window.cpp \
    crawler.cpp

HEADERS +=  \
    window.h \
    crawler.h
