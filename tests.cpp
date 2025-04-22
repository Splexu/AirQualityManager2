#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "MainWindow.h"
#include "AirQualityManager.h"
#include "MapWidget.h"

class TestAirQualityMonitor : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
    }

    void cleanupTestCase() {
    }

    void testCalculateDistance() {
        MainWindow window;
        double distance = window.calculateDistance(0.0, 0.0, 1.0, 1.0);
        QVERIFY2(qAbs(distance - 157.0) < 1.0, "Calculated distance is incorrect");
    }

    void testAnalyzeMeasurements() {
        MainWindow window;
        QList<Measurement> measurements;

        Measurement m1;
        m1.paramName = "PM10";
        m1.value = 25.0;
        m1.dateTime = QDateTime::fromString("2025-04-10T12:00:00", Qt::ISODate);
        measurements.append(m1);

        Measurement m2;
        m2.paramName = "PM10";
        m2.value = 30.0;
        m2.dateTime = QDateTime::fromString("2025-04-10T13:00:00", Qt::ISODate);
        measurements.append(m2);

        window.analyzeMeasurements(measurements);

        QString analysisText = window.findChild<QTextEdit*>("analysisTextEdit")->toPlainText();
        QVERIFY(analysisText.contains("Najmniejsza wartość: 25"));
        QVERIFY(analysisText.contains("Największa wartość: 30"));
        QVERIFY(analysisText.contains("Średnia wartość: 27.5"));
        QVERIFY(analysisText.contains("Trend: Rosnący"));
    }

    void testFetchStations() {
        AirQualityManager manager;
        QSignalSpy stationsSpy(&manager, &AirQualityManager::stationsFetched);
        QSignalSpy errorSpy(&manager, &AirQualityManager::errorOccurred);

        manager.fetchStations();

        QVERIFY(stationsSpy.isValid());
        QVERIFY(errorSpy.isValid());

        QList<Station> stations;
        Station s;
        s.id = 1;
        s.name = "Test Station";
        s.latitude = 52.0;
        s.longitude = 21.0;
        stations.append(s);
        emit manager.stationsFetched(stations);

        QVERIFY(stationsSpy.count() == 1);
        QList<QVariant> arguments = stationsSpy.takeFirst();
        QList<Station> receivedStations = arguments.at(0).value<QList<Station>>();
        QVERIFY(receivedStations.size() == 1);
        QVERIFY(receivedStations[0].id == 1);
        QVERIFY(receivedStations[0].name == "Test Station");
    }

    void testLatLonToPixel() {
        QList<Station> stations;
        QMap<int, QList<Measurement>> offline, online;
        MapWidget mapWidget(stations, offline, online);

        QPointF pixel = mapWidget.latLonToPixel(51.9, 19.1);
        QVERIFY(pixel.x() >= 0 && pixel.y() >= 0);

        double expectedXRatio = (19.1 - 14.1) / (24.1 - 14.1);
        double expectedYRatio = 1.0 - (51.9 - 49.0) / (54.8 - 49.0);
        QVERIFY(qAbs(pixel.x() / mapWidget.width() - expectedXRatio) < 0.1);
        QVERIFY(qAbs(pixel.y() / mapWidget.height() - expectedYRatio) < 0.1);
    }

    void testSaveStationsToJson() {
        MainWindow window;
        QList<Station> stations;
        Station s;
        s.id = 1;
        s.name = "Test Station";
        s.latitude = 52.0;
        s.longitude = 21.0;
        stations.append(s);

        window.saveStationsToJson(stations);

        QFile file("stations.json");
        QVERIFY(file.exists());
        QVERIFY(file.open(QIODevice::ReadOnly));
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        QJsonArray arr = doc.array();
        QVERIFY(arr.size() == 1);
        QJsonObject obj = arr[0].toObject();
        QVERIFY(obj["id"].toInt() == 1);
        QVERIFY(obj["name"].toString() == "Test Station");
        QVERIFY(qAbs(obj["latitude"].toDouble() - 52.0) < 0.01);
        QVERIFY(qAbs(obj["longitude"].toDouble() - 21.0) < 0.01);

        file.remove();
    }
};

QTEST_MAIN(TestAirQualityMonitor)
#include "tests.moc"
