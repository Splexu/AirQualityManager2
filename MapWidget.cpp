#include "MapWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QDebug>
#include <cmath>

MapWidget::MapWidget(const QList<Station> &stations,
                     const QMap<int, QList<Measurement>> &offline,
                     const QMap<int, QList<Measurement>> &online,
                     QWidget *parent)
    : QWidget(parent), mapImage(":/resources/poland_map.png"),
    offlineMeasurements(offline), onlineMeasurements(online), mapLoaded(false) {
    setMinimumSize(800, 600);
    setMouseTracking(true);

    if (mapImage.isNull()) {
        qDebug() << "Error: Failed to load poland_map.png";
    } else {
        qDebug() << "Success: poland_map.png loaded, size:" << mapImage.size();
        mapLoaded = true;
    }

    if (mapLoaded) {
        initializeMarkers(stations);
    }

    closeMapButton = new QPushButton("Zamknij mapę", this);
    connect(closeMapButton, &QPushButton::clicked, this, &MapWidget::onCloseMapClicked);

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addStretch();
    layout->addWidget(closeMapButton, 0, Qt::AlignHCenter);
    layout->addSpacing(10);
}

void MapWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (!mapLoaded) {
        painter.fillRect(rect(), Qt::gray);
        painter.setPen(Qt::red);
        painter.drawText(rect(), Qt::AlignCenter, "Failed to load map image");
        return;
    }

    painter.drawPixmap(0, 0, width(), height(), mapImage);

    for (const auto &marker : markers) {
        double xRatio = marker.position.x() / mapImage.width();
        double yRatio = marker.position.y() / mapImage.height();
        QPointF scaledPos(xRatio * width(), yRatio * height());

        painter.setPen(Qt::black);
        painter.setBrush(Qt::red);
        painter.drawEllipse(scaledPos, 5, 5);
    }
}

void MapWidget::mousePressEvent(QMouseEvent *event) {
    if (!mapLoaded) {
        return;
    }

    for (const auto &marker : markers) {
        double xRatio = marker.position.x() / mapImage.width();
        double yRatio = marker.position.y() / mapImage.height();
        QPointF scaledPos(xRatio * width(), yRatio * height());

        if ((event->pos() - scaledPos).manhattanLength() < 10) {
            showStationInfo(marker.station);
            break;
        }
    }
}

QPointF MapWidget::latLonToPixel(double latitude, double longitude) {
    if (!mapLoaded) {
        return QPointF(0, 0);
    }

    const double minLat = 49.0, maxLat = 54.8;
    const double minLon = 14.1, maxLon = 24.1;

    double xRatio = (longitude - minLon) / (maxLon - minLon);
    double yRatio = 1.0 - (latitude - minLat) / (maxLat - minLat);

    double x = xRatio * mapImage.width();
    double y = yRatio * mapImage.height();

    return QPointF(x, y);
}

void MapWidget::showStationInfo(const Station &station) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Wyświetl dane",
        QString("Czy chcesz zobaczyć dane dla stacji %1?").arg(station.name),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        emit stationSelected(station.id);
        close();
    }
}

QString MapWidget::analyzeMeasurementsByParameter(const QList<Measurement> &measurements) {
    QStringList analysisLines;

    QMap<QString, QList<Measurement>> measurementsByParam;
    for (const auto &m : measurements) {
        measurementsByParam[m.paramName].append(m);
    }

    for (auto it = measurementsByParam.constBegin(); it != measurementsByParam.constEnd(); ++it) {
        const QString paramName = it.key();
        const QList<Measurement> paramMeasurements = it.value();

        if (paramMeasurements.isEmpty()) {
            analysisLines << QString("Parametr %1: Brak danych.").arg(paramName);
            continue;
        }

        double minValue = paramMeasurements.first().value;
        double maxValue = paramMeasurements.first().value;
        QDateTime minDateTime = paramMeasurements.first().dateTime;
        QDateTime maxDateTime = paramMeasurements.first().dateTime;
        double sum = 0.0;
        int validCount = 0;

        for (const auto &m : paramMeasurements) {
            if (m.value >= 0) {
                if (m.value < minValue) {
                    minValue = m.value;
                    minDateTime = m.dateTime;
                }
                if (m.value > maxValue) {
                    maxValue = m.value;
                    maxDateTime = m.dateTime;
                }
                sum += m.value;
                validCount++;
            }
        }

        double average = (validCount > 0) ? sum / validCount : 0.0;

        double trend = 0.0;
        if (validCount > 1) {
            double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumXX = 0.0;
            int i = 0;
            for (const auto &m : paramMeasurements) {
                if (m.value >= 0) {
                    double x = i;
                    double y = m.value;
                    sumX += x;
                    sumY += y;
                    sumXY += x * y;
                    sumXX += x * x;
                    i++;
                }
            }
            double n = validCount;
            trend = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX);
        }

        analysisLines << QString("Parametr: %1").arg(paramName);
        analysisLines << QString("- Najmniejsza wartość: %1 (%2)").arg(minValue).arg(minDateTime.toString());
        analysisLines << QString("- Największa wartość: %1 (%2)").arg(maxValue).arg(maxDateTime.toString());
        analysisLines << QString("- Średnia wartość: %1").arg(average, 0, 'f', 2);
        analysisLines << QString("- Trend: %1").arg(trend > 0 ? "Rosnący" : trend < 0 ? "Malejący" : "Stabilny");
        analysisLines << "";
    }

    return analysisLines.join("\n");
}

void MapWidget::onCloseMapClicked() {
    close();
}

void MapWidget::initializeMarkers(const QList<Station> &stations) {
    markers.clear();
    for (const auto &s : stations) {
        StationMarker marker;
        marker.station = s;
        marker.position = latLonToPixel(s.latitude, s.longitude);
        markers.append(marker);
    }
}
