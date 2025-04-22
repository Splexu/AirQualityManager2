#pragma once
#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QVBoxLayout>
#include "AirQualityManager.h"

class TestAirQualityMonitor;

class MapWidget : public QWidget {
    Q_OBJECT

public:
    explicit MapWidget(const QList<Station> &stations,
                       const QMap<int, QList<Measurement>> &offlineMeasurements,
                       const QMap<int, QList<Measurement>> &onlineMeasurements,
                       QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void stationSelected(int stationId);

private slots:
    void onCloseMapClicked();

private:
    friend class TestAirQualityMonitor;

    struct StationMarker {
        Station station;
        QPointF position;
    };

    QPointF latLonToPixel(double latitude, double longitude);
    void showStationInfo(const Station &station);
    QString analyzeMeasurementsByParameter(const QList<Measurement> &measurements);
    void initializeMarkers(const QList<Station> &stations);

    QList<StationMarker> markers;
    QPixmap mapImage;
    QMap<int, QList<Measurement>> offlineMeasurements;
    QMap<int, QList<Measurement>> onlineMeasurements;
    QPushButton *closeMapButton;
    QVBoxLayout *layout;
    bool mapLoaded;
};
