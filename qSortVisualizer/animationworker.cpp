#include "animationworker.h"
#include <QDebug>
#include <QTime>
#include <QtCore>

AnimationWorker::AnimationWorker(QObject *parent) : QThread(parent)
{
    m_pause = false;
    quit = false;

    qDebug() << "AnimationWorker object constructed thread " << QThread::currentThread();
}

AnimationWorker::~AnimationWorker()
{
    qDebug() << "AnimationWorker object destructed and thread " << QThread::currentThread() << "terminated.";
}

void AnimationWorker::pause()
{
    qDebug() << "Main Thread " << QThread::currentThread() << " paused AnimationWorker thread.";

    m_mutex.lock();
    m_pause = true;
    m_mutex.unlock();
}

void AnimationWorker::resume()
{
    qDebug() << "Main Thread " << QThread::currentThread() << " resumed AnimationWorker thread.";

    m_mutex.lock();
    m_pause = false;
    m_mutex.unlock();

    m_pause_condition.wakeAll();
}

void AnimationWorker::stop()
{
    if(!quit){
        qDebug() << "Animation Worker: Stopping...";

        m_mutex.lock();
        quit = true;
        m_pause_condition.wakeOne();
        m_mutex.unlock();

        wait();

        qDebug() << "Animation Worker: Stopped";
    }
    else{
        qDebug() << "Animation Worker: Thread already stopped";
    }


}

void AnimationWorker::run()
{
    qDebug() << "Thread " << QThread::currentThread() << " running.";

    m_pause = false;
    quit = false;

    bool quit_indicator = false;

    double duration = DURATION;
    int calculatedDuration = 0;

    int steps = 0;
    int swaps = 0;
    int comps = 0;

    switch (currentAlgorithm) {
    case 0://izborom
    {
        int i;

        for(i = numbers->count()-1; i > 0; i--){
            int max = 0;
            items.at(max)->setColor(Qt::green);
            items.at(i)->setColor(Qt::red);

            emit updateCodeOverlay(0,steps++,comps,swaps);

            delay(duration);
            for(int j = 1; j<=i; j++){

                items.at(j)->setColor(Qt::red);

                emit updateCodeOverlay(1,steps++,comps++,swaps);

                delay(duration);

                if(currentOrder){ //DESC
                    if(items.at(j)->text().toInt() < items.at(max)->text().toInt()){
                        items.at(max)->setColor(QColor::fromRgb(179, 224, 231));
                        max = j;
                        items.at(max)->setColor(Qt::green);

                        emit updateCodeOverlay(2,steps,comps,swaps);

                        delay(duration);
                    }
                    else{
                        items.at(j)->setColor(QColor::fromRgb(179, 224, 231));
                    }
                }
                else{ //ASC
                    if(items.at(j)->text().toInt() > items.at(max)->text().toInt()){
                        items.at(max)->setColor(QColor::fromRgb(179, 224, 231));
                        max = j;
                        items.at(max)->setColor(Qt::green);

                        emit updateCodeOverlay(2,steps,comps,swaps);

                        delay(duration);
                    }
                    else{
                        items.at(j)->setColor(QColor::fromRgb(179, 224, 231));
                    }
                }


                m_mutex.lock();
                if(m_pause) m_pause_condition.wait(&m_mutex);
                if(quit) quit_indicator = true;
                duration = DURATION * currentSpeedFactor;
                calculatedDuration = duration + (std::abs(items.at(i)->geometry().x()-items.at(max)->geometry().x())) * currentSpeedFactor;
                m_mutex.unlock();

                if(quit_indicator) return;

            }
            if(i != max ){ //&& items.at(i)->text().toInt() != items.at(max)->text().toInt() --> sprjecava zamjenu istih brojeva na zadnjem koraku - maknuto zbog narusavanja originalnosti algoritma
                items.at(i)->setColor(Qt::green);
                items.swapItemsAt(i,max);

//                m_mutex.lock();
//                calculatedDuration = duration + (std::abs(items.at(i)->geometry().x()-items.at(max)->geometry().x())) * currentSpeedFactor;
//                m_mutex.unlock();

                delay(duration);

                emit updateCodeOverlay(3,steps,comps,swaps++);

                emit processAnimation(items.at(i),items.at(max),calculatedDuration);

                delay(calculatedDuration);

                //items.at(max)->setColor(Qt::yellow);
                //items.at(i)->setColor(QColor::fromRgb(179, 224, 231));//standard color
                items.at(max)->setColor(QColor::fromRgb(179, 224, 231));//standard color
            }
            delay(duration);
            items.at(i)->setColor(Qt::yellow);
            delay(duration);
        }
        items.at(i)->setColor(Qt::yellow);

        delay(duration*2);

        for(int k = 0; k < numbers->count(); k++){
            items.at(k)->setColor(Qt::red);
            delay(duration/2);
            items.at(k)->setColor(QColor::fromRgb(179, 224, 231));
        }

        break;
    }
    case 1://zamjenom
    {
        int i;

        for(i = numbers->count()-1; i > 0; i--){
            items.at(i)->setColor(Qt::red);

            emit updateCodeOverlay(0,steps++,comps,swaps);

            //sleep(duration/1000); --> thread sleep

            delay(duration);

            for(int j = 0; j < i; j++){
                //stepCount->setText(tr("Broj koraka: ")+QString::number(steps++));

                emit updateCodeOverlay(1,steps++,comps++,swaps);

                //pause next iteration j
                m_mutex.lock();
                if(m_pause) m_pause_condition.wait(&m_mutex);
                if(quit) quit_indicator = true;
                duration = DURATION * currentSpeedFactor;
                calculatedDuration = duration + (std::abs(items.at(i)->geometry().x()-items.at(j)->geometry().x())) * currentSpeedFactor;
                m_mutex.unlock();

                if(quit_indicator) return;

                items.at(j)->setColor(Qt::red);

                if(currentOrder){ //DESC
                    //compareCount->setText(tr("Broj usporedbi: ")+QString::number(comps++));
                    if(items.at(j)->text().toInt() < items.at(i)->text().toInt()){
                        delay(duration);

                        emit updateCodeOverlay(2,steps,comps,swaps++);

                        items.at(j)->setColor(Qt::green);
                        items.at(i)->setColor(Qt::green);

                        items.swapItemsAt(i,j);

                        //swapCount->setText(tr("Broj zamjena: ")+QString::number(swaps++));

                        delay(duration);

                        emit processAnimation(items.at(i),items.at(j),calculatedDuration);

                        delay(calculatedDuration);

                        items.at(j)->setColor(Qt::red);
                        items.at(i)->setColor(Qt::red);
                    }//if
                }
                else{ //ASC
                    //compareCount->setText(tr("Broj usporedbi: ")+QString::number(comps++));
                    if(items.at(j)->text().toInt() > items.at(i)->text().toInt()){
                        delay(duration);

                        emit updateCodeOverlay(2,steps,comps,swaps++);

                        items.at(j)->setColor(Qt::green);
                        items.at(i)->setColor(Qt::green);

                        items.swapItemsAt(i,j);

                        //swapCount->setText(tr("Broj zamjena: ")+QString::number(swaps++));

                        delay(duration);

                        emit processAnimation(items.at(i),items.at(j),calculatedDuration);

                        delay(calculatedDuration);

                        items.at(j)->setColor(Qt::red);
                        items.at(i)->setColor(Qt::red);
                    }//if
                }

                delay(duration);
                items.at(j)->setColor(QColor::fromRgb(179, 224, 231)); //standard color
            }//for j
            delay(duration);
            items.at(i)->setColor(Qt::yellow);
        }//for i
        delay(duration);
        items.at(i)->setColor(Qt::yellow);

        delay(duration*2);

        for(int k = 0; k < numbers->count(); k++){
            items.at(k)->setColor(Qt::red);
            delay(duration/2);
            items.at(k)->setColor(QColor::fromRgb(179, 224, 231));
        }

        break;
    }
    case 2: //umetanjem
    {
        int i;
        int j;

        items.at(0)->setColor(Qt::yellow);

        for(i = 1; i < numbers->count(); i++){

            items.at(i)->setColor(Qt::red);

            emit updateCodeOverlay(0,steps++,comps++,swaps);

            //delay(duration);

            if(currentOrder){ //DESC
                for(j = i; j > 0 && items.at(j)->text().toInt() > items.at(j-1)->text().toInt(); j--){
                    m_mutex.lock();
                    if(m_pause) m_pause_condition.wait(&m_mutex);
                    if(quit) quit_indicator = true;
                    duration = DURATION * currentSpeedFactor;
                    calculatedDuration = duration + (std::abs(items.at(j)->geometry().x()-items.at(j-1)->geometry().x())) * currentSpeedFactor;
                    m_mutex.unlock();

                    if(quit_indicator) return;

                    items.at(j)->setColor(Qt::red);
                    items.at(j-1)->setColor(Qt::green);

                    items.swapItemsAt(j,j-1);

                    delay(duration);

                    emit updateCodeOverlay(1,steps,comps++,swaps++);

                    emit processAnimation(items.at(j),items.at(j-1),calculatedDuration);

                    delay(calculatedDuration);

                    items.at(j)->setColor(Qt::yellow); //QColor::fromRgb(179, 224, 231)
                    items.at(j-1)->setColor(Qt::yellow);
                }
            }
            else{ //ASC
                for(j = i; j > 0 && items.at(j)->text().toInt() < items.at(j-1)->text().toInt(); j--){
                    m_mutex.lock();
                    if(m_pause) m_pause_condition.wait(&m_mutex);
                    if(quit) quit_indicator = true;
                    duration = DURATION * currentSpeedFactor;
                    calculatedDuration = duration + (std::abs(items.at(j)->geometry().x()-items.at(j-1)->geometry().x())) * currentSpeedFactor;
                    m_mutex.unlock();

                    if(quit_indicator) return;

                    items.at(j)->setColor(Qt::red);
                    items.at(j-1)->setColor(Qt::green);

                    items.swapItemsAt(j,j-1);

                    delay(duration);

                    emit updateCodeOverlay(1,steps,comps++,swaps++);

                    emit processAnimation(items.at(j),items.at(j-1),calculatedDuration);

                    delay(calculatedDuration);

                    items.at(j)->setColor(Qt::yellow); //QColor::fromRgb(179, 224, 231)
                    items.at(j-1)->setColor(Qt::yellow);
                }
            }

            delay(duration);
            items.at(i)->setColor(Qt::yellow);
        }
        delay(duration);
        items.at(i-1)->setColor(Qt::yellow);

        delay(duration*2);

        for(int k = 0; k < numbers->count(); k++){
            items.at(k)->setColor(Qt::red);
            delay(duration/2);
            items.at(k)->setColor(QColor::fromRgb(179, 224, 231));
        }

        break;
    }
    case 3://mjehuricasto
    {
        int i;
        int j;

        bool swapped = true;
        for(i = numbers->count()-1; swapped && i > 0; i--){
            swapped = false;

            emit updateCodeOverlay(1,steps++,comps,swaps);

            delay(duration);
            for(j = 0; j < i; j++){

                emit updateCodeOverlay(2,steps++,comps++,swaps);

//                items.at(j)->setColor(Qt::red);
//                items.at(j+1)->setColor(Qt::red);

                items.at(j)->setColor(Qt::green);
                items.at(j+1)->setColor(Qt::green);

                m_mutex.lock();
                if(m_pause) m_pause_condition.wait(&m_mutex);
                if(quit) quit_indicator = true;
                duration = DURATION * currentSpeedFactor;
                calculatedDuration = duration + (std::abs(items.at(j)->geometry().x()-items.at(j+1)->geometry().x())) * currentSpeedFactor;
                m_mutex.unlock();

                if(quit_indicator) return;

                delay(duration);

                if(currentOrder){ //DESC
                    if(items.at(j)->text().toInt() < items.at(j+1)->text().toInt()){
    //                    items.at(j)->setColor(Qt::green);
    //                    items.at(j+1)->setColor(Qt::green);

    //                    delay(duration);

                        items.swapItemsAt(j,j+1);

                        emit updateCodeOverlay(3,steps,comps,swaps++);

                        emit processAnimation(items.at(j),items.at(j+1),calculatedDuration);

                        delay(calculatedDuration);

                        swapped = true;
                    }
                }
                else{ //ASC
                    if(items.at(j)->text().toInt() > items.at(j+1)->text().toInt()){
    //                    items.at(j)->setColor(Qt::green);
    //                    items.at(j+1)->setColor(Qt::green);

    //                    delay(duration);

                        items.swapItemsAt(j,j+1);

                        emit updateCodeOverlay(3,steps,comps,swaps++);

                        emit processAnimation(items.at(j),items.at(j+1),calculatedDuration);

                        delay(calculatedDuration);

                        swapped = true;
                    }
                }

                delay(duration);

                items.at(j)->setColor(QColor::fromRgb(179, 224, 231));
                items.at(j+1)->setColor(QColor::fromRgb(179, 224, 231));

            }
            items.at(j)->setColor(Qt::yellow);
            delay(duration);
        }
        //items.at(i)->setColor(Qt::yellow);

        delay(duration*2);

        for(int k = 0; k < numbers->count(); k++){
            items.at(k)->setColor(Qt::red);
            delay(duration/2);
            items.at(k)->setColor(QColor::fromRgb(179, 224, 231));
        }

        break;
    }
    case 4://dvostruko mjehuricasto
    {
        int i;
        int j;

        int start = 0, end = numbers->count()-1;
        bool swapped = true;
        for(i = 1; swapped && i <= numbers->count(); i++){
            swapped = false;

            emit updateCodeOverlay(1,steps++,comps,swaps);

            delay(duration);
            if(i%2 == 0){
                for(j = start; j < end; j++){

    //                items.at(j)->setColor(Qt::red);
    //                items.at(j+1)->setColor(Qt::red);

                    items.at(j)->setColor(Qt::green);
                    items.at(j+1)->setColor(Qt::green);

                    m_mutex.lock();
                    if(m_pause) m_pause_condition.wait(&m_mutex);
                    if(quit) quit_indicator = true;
                    duration = DURATION * currentSpeedFactor;
                    calculatedDuration = duration + (std::abs(items.at(j)->geometry().x()-items.at(j+1)->geometry().x())) * currentSpeedFactor;
                    m_mutex.unlock();

                    if(quit_indicator) return;

                    emit updateCodeOverlay(3,steps++,comps++,swaps);

                    delay(duration);

                    if(currentOrder){ //DESC
                        if(items.at(j)->text().toInt() < items.at(j+1)->text().toInt()){
        //                    items.at(j)->setColor(Qt::green);
        //                    items.at(j+1)->setColor(Qt::green);

        //                    delay(duration);

                            items.swapItemsAt(j,j+1);

                            emit updateCodeOverlay(4,steps,comps,swaps++);

                            emit processAnimation(items.at(j),items.at(j+1),calculatedDuration);

                            delay(calculatedDuration);

                            swapped = true;
                        }
                    }
                    else{ //ASC
                        if(items.at(j)->text().toInt() > items.at(j+1)->text().toInt()){
        //                    items.at(j)->setColor(Qt::green);
        //                    items.at(j+1)->setColor(Qt::green);

        //                    delay(duration);

                            items.swapItemsAt(j,j+1);

                            emit updateCodeOverlay(4,steps,comps,swaps++);

                            emit processAnimation(items.at(j),items.at(j+1),calculatedDuration);

                            delay(calculatedDuration);

                            swapped = true;
                        }
                    }

                    delay(duration);

                    items.at(j)->setColor(QColor::fromRgb(179, 224, 231));
                    items.at(j+1)->setColor(QColor::fromRgb(179, 224, 231));

                }
                emit updateCodeOverlay(5,steps,comps,swaps);

                items.at(j)->setColor(Qt::yellow);
                delay(duration);

                end--;
            }
            else{
                for(j = end; j > start; j--){

    //                items.at(j)->setColor(Qt::red);
    //                items.at(j+1)->setColor(Qt::red);

                    items.at(j)->setColor(Qt::green);
                    items.at(j-1)->setColor(Qt::green);

                    m_mutex.lock();
                    if(m_pause) m_pause_condition.wait(&m_mutex);
                    if(quit) quit_indicator = true;
                    duration = DURATION * currentSpeedFactor;
                    calculatedDuration = duration + (std::abs(items.at(j)->geometry().x()-items.at(j-1)->geometry().x())) * currentSpeedFactor;
                    m_mutex.unlock();

                    if(quit_indicator) return;

                    emit updateCodeOverlay(7,steps++,comps++,swaps);

                    delay(duration);

                    if(currentOrder){ //DESC
                        if(items.at(j)->text().toInt() > items.at(j-1)->text().toInt()){
        //                    items.at(j)->setColor(Qt::green);
        //                    items.at(j+1)->setColor(Qt::green);

        //                    delay(duration);

                            items.swapItemsAt(j,j-1);

                            emit updateCodeOverlay(8,steps,comps,swaps++);

                            emit processAnimation(items.at(j),items.at(j-1),calculatedDuration);

                            delay(calculatedDuration);

                            swapped = true;
                        }
                    }
                    else{ //ASC
                        if(items.at(j)->text().toInt() < items.at(j-1)->text().toInt()){
        //                    items.at(j)->setColor(Qt::green);
        //                    items.at(j+1)->setColor(Qt::green);

        //                    delay(duration);

                            items.swapItemsAt(j,j-1);

                            emit updateCodeOverlay(8,steps,comps,swaps++);

                            emit processAnimation(items.at(j),items.at(j-1),calculatedDuration);

                            delay(calculatedDuration);

                            swapped = true;
                        }
                    }

                    delay(duration);

                    items.at(j)->setColor(QColor::fromRgb(179, 224, 231));
                    items.at(j-1)->setColor(QColor::fromRgb(179, 224, 231));

                }
                emit updateCodeOverlay(9,steps,comps,swaps);

                items.at(j)->setColor(Qt::yellow);
                delay(duration);

                start++;
            }
        }
        //items.at(i)->setColor(Qt::yellow);

        delay(duration*2);

        for(int k = 0; k < numbers->count(); k++){
            items.at(k)->setColor(Qt::red);
            delay(duration/2);
            items.at(k)->setColor(QColor::fromRgb(179, 224, 231));
        }

        break;
    }
    default:
        break;
    }

    emit stopAnimationWorker();
}

void AnimationWorker::delay(int mSecs)
{
    QTime dieTime = QTime::currentTime().addMSecs(mSecs);
    while(QTime::currentTime()<dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}
