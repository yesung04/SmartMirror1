#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QHeaderView>
#include <QTableWidgetItem>

#include <QUrlQuery>

#include <QGraphicsDropShadowEffect>

#include <QDate>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //--------------------------------
    // Network
    //--------------------------------

    manager =
        new QNetworkAccessManager(this);

    connect(manager,
            &QNetworkAccessManager::finished,
            this,
            &MainWindow::onWeatherReply);

    //--------------------------------
    // Window
    //--------------------------------

    this->setStyleSheet(
        "QMainWindow {"
        "background:qlineargradient("
        "x1:0,y1:0,x2:0,y2:1,"
        "stop:0 #87CEEB,"
        "stop:1 #EAF4FF);"
        "}");

    //--------------------------------
    // Current Weather
    //--------------------------------

    ui->labelLocation->setStyleSheet(
        "font-size:18px;"
        "font-weight:bold;"
        "color:#333;");

    ui->labelIcon->setStyleSheet(
        "font-size:48px;");

    ui->labelTemp->setStyleSheet(
        "font-size:42px;"
        "font-weight:bold;"
        "color:#111;");

    ui->labelWeather->setStyleSheet(
        "font-size:22px;"
        "color:#222;");

    ui->labelDetail->setStyleSheet(
        "font-size:15px;"
        "color:#333;");

    //--------------------------------
    // Forecast Card
    //--------------------------------

    ui->forecastCard->setStyleSheet(
        "background:rgba(255,255,255,0.65);"
        "border-radius:25px;");

    QGraphicsDropShadowEffect *shadow =
        new QGraphicsDropShadowEffect(this);

    shadow->setBlurRadius(25);

    shadow->setOffset(0,5);

    shadow->setColor(
        QColor(0,0,0,40));

    ui->forecastCard
        ->setGraphicsEffect(shadow);

    //--------------------------------
    // Table
    //--------------------------------

    ui->tableForecast->setShowGrid(false);

    ui->tableForecast->setFrameShape(
        QFrame::NoFrame);

    ui->tableForecast->setEditTriggers(
        QAbstractItemView::NoEditTriggers);

    ui->tableForecast->setSelectionMode(
        QAbstractItemView::NoSelection);

    ui->tableForecast->setFocusPolicy(
        Qt::NoFocus);

    ui->tableForecast
        ->setVerticalScrollBarPolicy(
            Qt::ScrollBarAlwaysOff);

    ui->tableForecast
        ->setHorizontalScrollBarPolicy(
            Qt::ScrollBarAlwaysOff);

    ui->tableForecast->viewport()
        ->setAutoFillBackground(false);

    //--------------------------------
    // Table Header
    //--------------------------------

    ui->tableForecast->horizontalHeader()
        ->setSectionResizeMode(
            QHeaderView::Stretch);

    ui->tableForecast->horizontalHeader()
        ->setStyleSheet(
            "QHeaderView::section {"
            "background:transparent;"
            "border:none;"
            "font-size:16px;"
            "font-weight:bold;"
            "color:#333;"
            "}");

    ui->tableForecast->verticalHeader()
        ->setStyleSheet(
            "QHeaderView::section {"
            "background:transparent;"
            "border:none;"
            "font-size:15px;"
            "font-weight:bold;"
            "color:#444;"
            "}");

    //--------------------------------
    // Table Style
    //--------------------------------

    ui->tableForecast->setStyleSheet(
        "QTableWidget {"
        "background:transparent;"
        "border:none;"
        "font-size:16px;"
        "color:#222;"
        "}"
        "QTableWidget::item {"
        "border:none;"
        "padding:10px;"
        "}");

    //--------------------------------
    // Row Height
    //--------------------------------

    for(int i = 0; i < 3; i++)
    {
        ui->tableForecast
            ->setRowHeight(i, 32);
    }

    //--------------------------------
    // Row Labels
    //--------------------------------

    QStringList rowLabels;

    rowLabels << "날씨"
              << "최고"
              << "최저";

    ui->tableForecast
        ->setVerticalHeaderLabels(
            rowLabels);

    //--------------------------------
    // Start City
    //--------------------------------

    requestWeather("Seoul");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::requestWeather(
    QString city)
{
    QString apiKey =
        "5658debc671e4dcba1e104756261905";

    QUrl url(
        "https://api.weatherapi.com/v1/forecast.json");

    QUrlQuery query;

    query.addQueryItem(
        "key",
        apiKey);

    query.addQueryItem(
        "q",
        city);

    query.addQueryItem(
        "days",
        "7");

    query.addQueryItem(
        "lang",
        "ko");

    url.setQuery(query);

    QNetworkRequest request(url);

    manager->get(request);
}

void MainWindow::onWeatherReply(
    QNetworkReply *reply)
{
    QByteArray data =
        reply->readAll();

    QJsonDocument doc =
        QJsonDocument::fromJson(data);

    if(doc.isNull())
        return;

    QJsonObject obj =
        doc.object();

    //--------------------------------
    // Current Weather
    //--------------------------------

    QJsonObject current =
        obj["current"].toObject();

    double temp =
        current["temp_c"].toDouble();

    double feelsLike =
        current["feelslike_c"].toDouble();

    int humidity =
        current["humidity"].toInt();

    double wind =
        current["wind_kph"].toDouble();

    QString weatherText =
        current["condition"]
            .toObject()["text"]
            .toString();

    QString icon = "⛅";

    if(weatherText.contains("맑"))
        icon = "☀";

    else if(weatherText.contains("구름"))
        icon = "☁";

    else if(weatherText.contains("비"))
        icon = "🌧";

    else if(weatherText.contains("눈"))
        icon = "❄";

    ui->labelIcon->setText(icon);

    ui->labelTemp->setText(
        QString::number(temp)
        + "°C");

    ui->labelWeather->setText(
        weatherText);

    ui->labelDetail->setText(
        QString(
            "체감 %1°   습도 %2%   풍속 %3 km/h")
            .arg(feelsLike)
            .arg(humidity)
            .arg(wind));

    //--------------------------------
    // Location
    //--------------------------------

    QString city =
        obj["location"]
            .toObject()["name"]
            .toString();

    ui->labelLocation->setText(
        "현재 위치 : "
        + koreanCityName(city));

    //--------------------------------
    // Forecast
    //--------------------------------

    QJsonArray forecastday =
        obj["forecast"]
            .toObject()["forecastday"]
            .toArray();

    QStringList week =
    {
        "일","월","화","수",
        "목","금","토"
    };

    for(int i = 0;
        i < forecastday.size();
        i++)
    {
        QJsonObject dayObj =
            forecastday[i].toObject();

        QString date =
            dayObj["date"].toString();

        QDate qdate =
            QDate::fromString(
                date,
                "yyyy-MM-dd");

        QString dayName =
            week[qdate.dayOfWeek() % 7];

        ui->tableForecast
            ->setHorizontalHeaderItem(
                i,
                new QTableWidgetItem(dayName));

        QJsonObject day =
            dayObj["day"].toObject();

        QString weather =
            day["condition"]
                .toObject()["text"]
                .toString();

        QString dayIcon = "⛅";

        if(weather.contains("맑"))
            dayIcon = "☀";

        else if(weather.contains("구름"))
            dayIcon = "☁";

        else if(weather.contains("비"))
            dayIcon = "🌧";

        else if(weather.contains("눈"))
            dayIcon = "❄";

        double maxTemp =
            day["maxtemp_c"].toDouble();

        double minTemp =
            day["mintemp_c"].toDouble();

        //--------------------------------
        // Icon
        //--------------------------------

        QTableWidgetItem *iconItem =
            new QTableWidgetItem(dayIcon);

        iconItem->setTextAlignment(
            Qt::AlignCenter);

        ui->tableForecast->setItem(
            0,
            i,
            iconItem);

        //--------------------------------
        // Max Temp
        //--------------------------------

        QTableWidgetItem *maxItem =
            new QTableWidgetItem(
                QString::number(maxTemp)
                + "°");

        maxItem->setTextAlignment(
            Qt::AlignCenter);

        maxItem->setForeground(
            QColor("#E53935"));

        ui->tableForecast->setItem(
            1,
            i,
            maxItem);

        //--------------------------------
        // Min Temp
        //--------------------------------

        QTableWidgetItem *minItem =
            new QTableWidgetItem(
                QString::number(minTemp)
                + "°");

        minItem->setTextAlignment(
            Qt::AlignCenter);

        minItem->setForeground(
            QColor("#1E88E5"));

        ui->tableForecast->setItem(
            2,
            i,
            minItem);
    }

    reply->deleteLater();
}

QString MainWindow::koreanCityName(
    QString city)
{
    if(city == "Seoul")
        return "서울";

    if(city == "Busan")
        return "부산";

    if(city == "Tokyo")
        return "도쿄";

    if(city == "London")
        return "런던";

    if(city == "Paris")
        return "파리";

    return city;
}
