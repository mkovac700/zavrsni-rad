#include "statisticsworker.h"

StatisticsWorker::StatisticsWorker(QObject *parent)
    : QThread{parent}
{
    quit = false;
//    arr = new QList<int>;
}

StatisticsWorker::~StatisticsWorker()
{
    qDebug() << "StatisticsWorker object destructed.";
}

void StatisticsWorker::stop()
{
    if(!quit){
        qDebug() << "StatisticsWorker stopping...";

        mutex.lock();
        quit = true;
        mutex.unlock();

        wait();

        qDebug() << "StatisticsWorker stopped.";
    }
    else{
        qDebug() << "StatisticsWorker already stopped.";
    }
}

void StatisticsWorker::run()
{
    qDebug() << "StatisticsWorker running...";

    quit = false;
    flag = false;

    steps = 0, comps = 0, swaps = 0;

    clock_t t1, t2;

//    QTimer *timer = new QTimer();
//    connect(timer, &QTimer::timeout, this, &StatisticsWorker::emitUpdateSignal);
//    timer->start(1000);

//    qDebug() << "Nesortirano: ";
//    for(int i = 0; i < arr->count(); i++) qDebug() << arr->at(i);

    switch (algorithm) {
    case 0: //izborom
    {
        t1 = clock();

        for(int i = arr.count()-1; i > 0; i--){
            int max = 0;
            steps++;
            for(int j = 1; j <= i; j++){
                mutex.lock();
                if(quit) flag = true;
                mutex.unlock();
                if(flag) return;

                steps++;
                comps++;

                if(arr.at(j) > arr.at(max)) max = j;
            }
            if(i != max){
                swaps++;
                arr.swapItemsAt(i,max);
            }
        }

        t2 = clock();

        emit updateStats(algorithm,steps,comps,swaps,t2-t1);

//        qDebug() << "Sortirano: ";
//        for(int i = 0; i < arr->count(); i++) qDebug() << arr->at(i);

        break;
    }
    case 1: //zamjenom
    {
        t1 = clock();

        for(int i = arr.count()-1; i > 0; i--){
            steps++;
            for(int j = 0; j < i; j++){

                mutex.lock();
                if(quit) flag = true;
                mutex.unlock();
                if(flag) return;

                steps++;
                comps++;
                if(arr.at(j) > arr.at(i)){
                    swaps++;
                    arr.swapItemsAt(i,j);
                }
            }
        }

        t2 = clock();

        emit updateStats(algorithm,steps,comps,swaps,t2-t1);

//        qDebug() << "Sortirano: ";
//        for(int i = 0; i < arr.count(); i++) qDebug() << arr.at(i);

        break;
    }
    case 2: //umetanjem
    {
        t1 = clock();

        for(int i = 1; i < arr.count(); i++){
            steps++;
            comps++;
            for(int j = i; j > 0 && arr.at(j) < arr.at(j-1); j--){

                mutex.lock();
                if(quit) flag = true;
                mutex.unlock();
                if(flag) return;

                steps++;
                comps++;
                swaps++;

                arr.swapItemsAt(j,j-1);
            }
        }

        t2 = clock();

        emit updateStats(algorithm,steps,comps,swaps,t2-t1);

//        qDebug() << "Sortirano: ";
//        for(int i = 0; i < arr.count(); i++) qDebug() << arr.at(i);

        break;
    }
    case 3: //mjehuricasto
    {
        t1 = clock();

        bool swapped = true;

        for(int i = arr.count()-1; swapped && i > 0; i--){
            swapped = false;
            steps++;
            for(int j = 0; j < i; j++){

                mutex.lock();
                if(quit) flag = true;
                mutex.unlock();
                if(flag) return;

                steps++;
                comps++;

                if(arr.at(j) > arr.at(j+1)){
                    swapped = true;
                    swaps++;

                    arr.swapItemsAt(j,j+1);
                }

            }
        }

        t2 = clock();

        emit updateStats(algorithm,steps,comps,swaps,t2-t1);

//        qDebug() << "Sortirano: ";
//        for(int i = 0; i < arr.count(); i++) qDebug() << arr.at(i);

        break;
    }
    case 4: //dvostruko mjehuricasto
    {
        t1 = clock();

        int start = 0, end = arr.count()-1;
        bool swapped = true;

        for(int i = 1; swapped && i <= arr.count(); i++){
            swapped = false;
            steps++;
            if(i%2 == 0){
                for(int j = start; j < end; j++){

                    mutex.lock();
                    if(quit) flag = true;
                    mutex.unlock();
                    if(flag) return;

                    steps++;
                    comps++;

                    if(arr.at(j) > arr.at(j+1)){
                        swapped = true;
                        swaps++;
                        arr.swapItemsAt(j,j+1);
                    }
                }
                end--;
            }
            else{
                for(int j = end; j > start; j--){

                    mutex.lock();
                    if(quit) flag = true;
                    mutex.unlock();
                    if(flag) return;

                    steps++;
                    comps++;

                    if(arr.at(j) < arr.at(j-1)){
                        swapped = true;
                        swaps++;
                        arr.swapItemsAt(j,j-1);
                    }
                }

                start++;
            }
        }

        t2 = clock();

        emit updateStats(algorithm,steps,comps,swaps,t2-t1);

//        qDebug() << "Sortirano: ";
//        for(int i = 0; i < arr.count(); i++) qDebug() << arr.at(i);

        break;
    }
    default:
        break;
    }

    emit stopStatisticsWorkers(algorithm);

    qDebug() << "StatisticsWorker finished!";
}

void StatisticsWorker::emitUpdateSignal()
{
    //qDebug() << "test";
    emit updateStats(algorithm,steps,comps,swaps,0);
}
