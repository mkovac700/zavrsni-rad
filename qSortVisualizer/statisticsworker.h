#ifndef STATISTICSWORKER_H
#define STATISTICSWORKER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include <QDebug>
#include <QtCore>

class StatisticsWorker : public QThread
{
    Q_OBJECT
public:
    explicit StatisticsWorker(QObject *parent = nullptr);
    ~StatisticsWorker();

    void generateArray(const int &size);

    QList<unsigned int> arr;
    int algorithm;

signals:
    void updateStats(const int &algorithm, const unsigned long long &steps, const unsigned long long &comps, const unsigned long long &swaps, const long &time);
    void stopStatisticsWorkers(const int &algorithm);
public slots:
    void stop();

protected:
    void run() override;

private:
    QMutex mutex;
    bool quit, flag;

};

#endif // STATISTICSWORKER_H
