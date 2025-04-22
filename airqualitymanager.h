#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QList>
#include <QMap>
#include <QDateTime>

struct Station {
    int id;
    QString name;
    double latitude;
    double longitude;
};

struct Sensor {
    int stationId;
    int id;
    QString paramName;
};

struct Measurement {
    int stationId;
    QString paramName;
    double value;
    QDateTime dateTime;
};

class AirQualityManager : public QObject {
    Q_OBJECT

public:
    explicit AirQualityManager(QObject *parent = nullptr);
    void fetchStations();
    void fetchSensors(int stationId);
    void fetchSensorData(int sensorId);
    void fetchCoordinates(const QString &address);
    QMap<int, QList<Measurement>> getOnlineMeasurements() const;

signals:
    void stationsFetched(const QList<Station> &stations);
    void sensorsFetched(const QList<Sensor> &sensors);
    void measurementsFetched(const QList<Measurement> &measurements);
    void coordinatesFetched(double latitude, double longitude);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *networkManager;
    QMap<int, QList<Measurement>> onlineMeasurements;
};
