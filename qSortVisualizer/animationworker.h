#ifndef ANIMATIONWORKER_H
#define ANIMATIONWORKER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include <rectitem.h>

class AnimationWorker : public QThread
{
    Q_OBJECT
public:
    explicit AnimationWorker(QObject *parent = nullptr);
    ~AnimationWorker();

    QList<RectItem *> items;
    QStringList *numbers;

    const int DURATION = 500;
    double currentSpeedFactor = 1;

    int currentAlgorithm;
    int currentOrder;

signals:
    void processAnimation(RectItem *item_i, RectItem *item_j, int duration);
    void stopAnimationWorker();

    void updateCodeOverlay(const int &checkpoint, const int &stepCount, const int &compareCount, const int &swapCount);

public slots:
    void pause();
    void resume();
    void stop();

    // QThread interface
protected:
    void run() override;

private:
    QMutex m_mutex;
    QWaitCondition m_pause_condition;
    bool m_pause;
    bool quit;

    void delay(int mSecs);
};

#endif // ANIMATIONWORKER_H
