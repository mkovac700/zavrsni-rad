#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"

#include <QRandomGenerator>
#include <QTime>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //removes window resize triangle (grip)
    ui->statusbar->setSizeGripEnabled(false);

    loadGraphicsScene();

    //ui->spinBoxSize->findChild<QLineEdit *>()->setReadOnly(true);

    numbers = new QStringList;

    animationState = Stopped;

    animationWorker = new AnimationWorker(this);

    connect(animationWorker,&AnimationWorker::processAnimation,this,&MainWindow::processAnimation);

    connect(animationWorker,&AnimationWorker::stopAnimationWorker,this,&MainWindow::stopAnimationWorker);

    qDebug() << "Main Thread " << QThread::currentThread() << " running.";

    currentScale = 100;

    ui->spinBoxSize->setValue(QRandomGenerator::global()->bounded(3,21));

    generateElements(ui->comboBoxElements->currentIndex());

    currentAlgorithmCode = new QList<QLabel *>;

    currentAlgorithm = 0;

    currentOrder = MainWindow::CurrentOrder::ASC;

    generateItems();

    animationTimer = new QTimer(this);

    connect(animationTimer, &QTimer::timeout, this, &MainWindow::updateAnimationElapsedTime);

    animationTimeElapsed = 0;

    currentTimeElapsed = new QTime(0,0);

    connect(animationWorker,&AnimationWorker::updateCodeOverlay,this,&MainWindow::updateCodeOverlay);

    connect(animationWorker,&AnimationWorker::emitPauseSignal,this,[=](){
        animationTimer->stop();
        ui->statusbar->showMessage(tr("Vizualizacija: Pauzirano"));
    });

    //STATISTICS:
    loadStatisticsGraph();

    prepareStatisticsWorkers();
}

MainWindow::~MainWindow()
{
    qDebug() << "Main Thread " << QThread::currentThread() << "terminated.";

    animationWorker->stop();

    for(int i = 0; i < 5; i++){
        statisticsWorkers.at(i)->stop();
    }

    delete ui;
}

void MainWindow::loadGraphicsScene()
{
    scene = new QGraphicsScene(this);

    //scene->setSceneRect(-1024/2,-1024/2,1024,1024);
    //scene->setSceneRect(-800/2,-600/2,800,600);

    //coordinate lines
    //scene->addLine(-800,0,800,0,QPen(Qt::black,1,Qt::DashLine));
    //scene->addLine(0,-400,0,400,QPen(Qt::black,1,Qt::DashLine));

    //bounding rectangle lines
//    QLineF topLine(scene->sceneRect().topLeft(),
//                   scene->sceneRect().topRight());
//    QLineF leftLine(scene->sceneRect().topLeft(),
//                    scene->sceneRect().bottomLeft());
//    QLineF rightLine(scene->sceneRect().topRight(),
//                     scene->sceneRect().bottomRight());
//    QLineF bottomLine(scene->sceneRect().bottomLeft(),
//                      scene->sceneRect().bottomRight());

//    QPen myPen = QPen(Qt::red);

//    scene->addLine(topLine, myPen);
//    scene->addLine(leftLine, myPen);
//    scene->addLine(rightLine, myPen);
//    scene->addLine(bottomLine, myPen);

    //set scene
    ui->graphicsView->setScene(scene);

    //load overlay layout

    viewGridLayout = new QGridLayout;

    ui->graphicsView->setLayout(viewGridLayout);

    //load stats widget:
    loadStatsOverlay();

    loadCodeOverlay();
}

void MainWindow::generateElements(const int &index)
{
    numbers->clear();

    switch (index) {
    case 0: //nasumicno
    {
        ui->lineEditElements->setReadOnly(true);

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            numbers->append(QString::number(QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1))); //1,51
        }

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 1://uzlazno
    {
        ui->lineEditElements->setReadOnly(true);

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            numbers->append(QString::number(QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1)));
        }

        std::sort(numbers->begin(), numbers->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
            return numbers1.toInt() < numbers2.toInt();
        });

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 2://silazno
    {
        ui->lineEditElements->setReadOnly(true);

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            numbers->append(QString::number(QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1)));
        }

        std::sort(numbers->begin(), numbers->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
            return numbers1.toInt() > numbers2.toInt();
        });

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 3: //skoro sortirano
    {
        ui->lineEditElements->setReadOnly(true);

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            numbers->append(QString::number(QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1)));
        }

        std::sort(numbers->begin(), numbers->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
            return numbers1.toInt() < numbers2.toInt();
        });

        int range = QRandomGenerator::global()->bounded(1,(int)std::sqrt(numbers->count())+1);

        QList<int> *tmp = new QList<int>;

        for (int k = 0; k < range; k++) {
            int i=0;//,j=0;
            do {
                i = QRandomGenerator::global()->bounded(1,numbers->count());
            } while (numbers->at(i) == numbers->at(i-1) || tmp->contains(i));

            numbers->swapItemsAt(i,i-1);

            tmp->append(i);
            tmp->append(i-1);
        }

        delete tmp;

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 4://uniformno
    {
        ui->lineEditElements->setReadOnly(true);

        int pivot;

        (ui->spinBoxRange->value() <= 3) ? pivot = QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1)
                                        : pivot = QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()-1);

        //int pivot = QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()-1); //[2,50> //[1,79>

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            (pivot <= 3) ? numbers->append(QString::number(QRandomGenerator::global()->bounded(pivot,pivot+1)))
                        : numbers->append(QString::number(QRandomGenerator::global()->bounded(pivot,pivot+3)));
            //numbers->append(QString::number(QRandomGenerator::global()->bounded(pivot,pivot+3))); //[78, 78+3=81>
        }

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 5: //gaussova
    {
        ui->lineEditElements->setReadOnly(true);

        for (int i=0; i<ui->spinBoxSize->value() ; i++ ) {
            numbers->append(QString::number(QRandomGenerator::global()->bounded(1,ui->spinBoxRange->value()+1)));
        }

        QStringList *tmp1 = new QStringList, *tmp2 = new QStringList;

        for (int i = 0; i < numbers->count()/2 ; i++ ) {
            tmp1->append(numbers->at(i));
        }

        for(int i = numbers->count()/2; i < numbers->count(); i++){
            tmp2->append(numbers->at(i));
        }

        std::sort(tmp1->begin(), tmp1->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
            return numbers1.toInt() < numbers2.toInt();
        });

        std::sort(tmp2->begin(), tmp2->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
            return numbers1.toInt() > numbers2.toInt();
        });

        numbers->clear();
        numbers->append(*tmp1);
        numbers->append(*tmp2);

        delete tmp1;
        delete tmp2;

        ui->lineEditElements->setText(numbers->join(" "));

        break;
    }
    case 6: //prilagodeno
    {
        ui->lineEditElements->clear();

        ui->lineEditElements->setReadOnly(false);

        ui->lineEditElements->setPlaceholderText(tr("Moguće je unijeti najviše 20 brojeva u rasponu 1-80 odvojenih razmakom."));

        ui->spinBoxSize->setMinimum(0);
        ui->spinBoxSize->setMaximum(0);

        ui->spinBoxRange->setMinimum(0);
        ui->spinBoxRange->setMaximum(0);

        ui->toolButtonRandomElements->setDisabled(true);
        ui->toolButtonRandomSize->setDisabled(true);
        ui->toolButtonRandomRange->setDisabled(true);

        //ostatak se izvrsava putem eventa

        break;
    }
    default:
        break;
    }
}

void MainWindow::generateItems()
{
    foreach (RectItem *item, items) {
        foreach (QGraphicsItem *sceneItem, scene->items()) {
            if(item == sceneItem){
                scene->removeItem(sceneItem);
            }
        }
        items.removeOne(item);
        delete item;
    }

    int offset = calculateItemOffset();

    for(int i=0; i<numbers->count(); i++){
        int number = numbers->at(i).toInt();
        int alignment = 0;

        if(number <= 5) alignment = Qt::AlignHCenter | Qt::AlignTop;
        else alignment = Qt::AlignHCenter | Qt::AlignBottom;

        RectItem *item = new RectItem;
        item->setGeometry(QRect(offset,-number*5,30,number*5));
        item->setColor(QColor::fromRgb(179, 224, 231)); //53,193,241 //173,216,230 //25,155,226 //145, 178, 199 //179, 224, 231 -> fav //122, 197, 205
        item->setText(QString::number(number),Qt::black,alignment);
        scene->addItem(item);

        items.append(item);

        offset+=60;
    }

    //proslijeđivanje varijabli
    animationWorker->items = items;
    animationWorker->numbers = numbers;
    animationWorker->currentAlgorithm = currentAlgorithm;
    animationWorker->currentOrder = (int)currentOrder;

    prepareCodeOverlay(currentAlgorithm);
    prepareStatsOverlay();

    ui->toolButtonPlayPause->setEnabled(true);

    ui->statusbar->showMessage(tr("Vizualizacija: Spremno"));
}

int MainWindow::calculateItemOffset()
{
    if(numbers->count() % 2 == 0){
        return -45-60*((numbers->count()/2)-1);
    }
    else{
        return -15-60*(std::round((numbers->count()/(float)2))-1);
    }
}

void MainWindow::delay(int mSecs)
{
    QTime dieTime = QTime::currentTime().addMSecs(mSecs);
    while(QTime::currentTime()<dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents,100);
}

void MainWindow::stopAnimationWorker()
{
    animationTimer->stop();

    animationState = Stopped;

    ui->toolButtonPlayPause->setIcon(QIcon("://images/icons8-play-48.png"));
    ui->toolButtonPlayPause->setToolTip(tr("Pokreni"));

    ui->toolButtonPlayPause->setDisabled(true);

    if(!animationWorker->isFinished()) animationWorker->stop();

    foreach (QLabel *label, *currentAlgorithmCode) {
        label->setStyleSheet("");
    }

    //ui->groupBox->setEnabled(true);
    ui->comboBoxElements->setEnabled(true);
    if(ui->comboBoxElements->currentIndex() == 6) ui->lineEditElements->setReadOnly(false);
    ui->toolButtonRandomElements->setEnabled(true);

    ui->toolButtonRandomSize->setEnabled(true);
    ui->spinBoxSize->setMinimum(3);
    ui->spinBoxSize->setMaximum(20);

    ui->comboBoxAlgorithm->setEnabled(true);

    ui->radioButtonAscending->setEnabled(true);
    ui->radioButtonDescending->setEnabled(true);

    ui->statusbar->showMessage(tr("Vizualizacija: Završeno"));
}

void MainWindow::updateAnimationElapsedTime()
{
    labelElapsedTime->setText(tr("Proteklo vrijeme: ")+currentTimeElapsed->addSecs(animationTimeElapsed++).toString("mm:ss"));
}

void MainWindow::loadCodeOverlay()
{
    groupBoxCode = new QGroupBox;
    vBoxLayoutCode = new QVBoxLayout;

    vBoxLayoutCode->addStretch();
    vBoxLayoutCode->setDirection(QBoxLayout::TopToBottom);

    groupBoxCode->setLayout(vBoxLayoutCode);
    //groupBoxCode->setStyleSheet("border: none");
    //groupBoxCode->setStyleSheet("QGroupBox{border: none; background-color:rgba(255,255,255,0.7)}");
    groupBoxCode->setStyleSheet("QGroupBox{background-color:rgba(255,255,255,0.8)}");

    viewGridLayout->addWidget(groupBoxCode,0,0,Qt::AlignBottom | Qt::AlignRight);
}

void MainWindow::prepareCodeOverlay(const int &algorithmIndex)
{
    currentAlgorithmCode->clear();

    //brisanje labela s groupboxa
    QLayoutItem *child;
    while ((child = vBoxLayoutCode->takeAt(0)) != nullptr) {

        delete child->widget(); // delete the widget
        delete child;   // delete the layout item
    }

    switch (algorithmIndex) {
    case 0: //izborom
    {
        currentOrder ? currentAlgorithmCode->append(new QLabel("for(i = N-1; i > 0; i--)\n\tmin = 0;"))
                     : currentAlgorithmCode->append(new QLabel("for(i = N-1; i > 0; i--)\n\tmax = 0;"));
        currentAlgorithmCode->append(new QLabel("\tfor(j = 1; j <= i; j++)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\t\tif(A[j] < A[min])\n\t\t\tmin = j;"))
                     : currentAlgorithmCode->append(new QLabel("\t\tif(A[j] > A[max])\n\t\t\tmax = j;"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\tif(i != min)\n\t\tswap(i,min);"))
                     : currentAlgorithmCode->append(new QLabel("\tif(i != max)\n\t\tswap(i,max);"));

        foreach (QLabel *label, *currentAlgorithmCode) {
            label->setFont(QFont("Courier",9,QFont::Weight::Normal));
            vBoxLayoutCode->addWidget(label);
        }

        break;
    }
    case 1: //zamjenom
    {
        currentAlgorithmCode->append(new QLabel("for(i = N-1; i > 0; i--)"));
        currentAlgorithmCode->append(new QLabel("\tfor(j = 0; j < i; j++)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\t\tif(A[j] < A[i])\n\t\t\tswap(i,j);"))
                     : currentAlgorithmCode->append(new QLabel("\t\tif(A[j] > A[i])\n\t\t\tswap(i,j);"));

        foreach (QLabel *label, *currentAlgorithmCode) {
            label->setFont(QFont("Courier",9,QFont::Weight::Normal));
            vBoxLayoutCode->addWidget(label);
        }

        break;
    }
    case 2: //umetanjem
    {
        currentAlgorithmCode->append(new QLabel("for(i = 1; i < N; i++)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\tfor(j = i; j > 0 && A[j] > A[j-1]; j--)\n\t\tswap(j,j-1);"))
                     : currentAlgorithmCode->append(new QLabel("\tfor(j = i; j > 0 && A[j] < A[j-1]; j--)\n\t\tswap(j,j-1);"));

        foreach (QLabel *label, *currentAlgorithmCode) {
            label->setFont(QFont("Courier",9,QFont::Weight::Normal));
            vBoxLayoutCode->addWidget(label);
        }

        break;
    }
    case 3: //mjehuricasto
    {
        currentAlgorithmCode->append(new QLabel("swapped = true;"));
        currentAlgorithmCode->append(new QLabel("for(i = N-1; swapped && i > 0; i--)\n\tswapped = false;"));
        currentAlgorithmCode->append(new QLabel("\tfor(j = 0; j < i; j++)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\t\tif(A[j] < A[j+1])\n\t\t\tswap(j,j+1);\n\t\t\tswapped = true;"))
                     : currentAlgorithmCode->append(new QLabel("\t\tif(A[j] > A[j+1])\n\t\t\tswap(j,j+1);\n\t\t\tswapped = true;"));

        foreach (QLabel *label, *currentAlgorithmCode) {
            label->setFont(QFont("Courier",9,QFont::Weight::Normal));
            vBoxLayoutCode->addWidget(label);
        }

        break;
    }
    case 4: //dvostruko mjehuricasto
    {
        currentAlgorithmCode->append(new QLabel("int start = 0, end = N-1;\nswapped = true;"));
        currentAlgorithmCode->append(new QLabel("for(i = 1; swapped && i <= N; i++)\n\tswapped = false;"));

        currentAlgorithmCode->append(new QLabel("\tif(i%2 == 0)"));
        currentAlgorithmCode->append(new QLabel("\t\tfor(j = start; j < end; j++)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\t\t\tif(A[j] < A[j+1])\n\t\t\t\tswap(j,j+1);\n\t\t\t\tswapped = true;"))
                     : currentAlgorithmCode->append(new QLabel("\t\t\tif(A[j] > A[j+1])\n\t\t\t\tswap(j,j+1);\n\t\t\t\tswapped = true;"));
        currentAlgorithmCode->append(new QLabel("\t\tend--;"));

        currentAlgorithmCode->append(new QLabel("\telse"));
        currentAlgorithmCode->append(new QLabel("\t\tfor(j = end; j > start; j--)"));
        currentOrder ? currentAlgorithmCode->append(new QLabel("\t\t\tif(A[j] > A[j-1])\n\t\t\t\tswap(j,j-1);\n\t\t\t\tswapped = true;"))
                     : currentAlgorithmCode->append(new QLabel("\t\t\tif(A[j] < A[j-1])\n\t\t\t\tswap(j,j-1);\n\t\t\t\tswapped = true;"));
        currentAlgorithmCode->append(new QLabel("\t\tstart++;"));

        foreach (QLabel *label, *currentAlgorithmCode) {
            label->setFont(QFont("Courier",9,QFont::Weight::Normal));
            vBoxLayoutCode->addWidget(label);
        }

        break;
    }
    default:
        break;
    }
}

void MainWindow::updateCodeOverlay(const int &checkpoint, const int &stepCount, const int &compareCount, const int &swapCount)
{
    foreach (QLabel *label, *currentAlgorithmCode) {
        label->setStyleSheet("");
    }
    currentAlgorithmCode->at(checkpoint)->setStyleSheet("QLabel{background-color:rgba(0,255,0,0.3)}");

    labelStepCount->setText(tr("Broj koraka: ").append(QString::number(stepCount)));
    labelCompareCount->setText(tr("Broj usporedbi: ").append(QString::number(compareCount)));
    labelSwapCount->setText(tr("Broj zamjena: ").append(QString::number(swapCount)));
}

void MainWindow::loadStatsOverlay()
{
    groupBoxStats = new QGroupBox;
    vBoxLayoutStats = new QVBoxLayout;

    vBoxLayoutStats->addStretch();
    vBoxLayoutStats->setDirection(QBoxLayout::TopToBottom);

    QFont font("Helvetica [Cronyx]", 11, QFont::Weight::Normal);

    labelElapsedTime = new QLabel;
    labelStepCount = new QLabel;
    labelCompareCount = new QLabel;
    labelSwapCount = new QLabel;

    labelElapsedTime->setFont(font);
    labelStepCount->setFont(font);
    labelCompareCount->setFont(font);
    labelSwapCount->setFont(font);

    labelElapsedTime->setText(tr("Proteklo vrijeme: 00:00"));
    labelStepCount->setText(tr("Broj koraka: 0"));
    labelCompareCount->setText(tr("Broj usporedbi: 0"));
    labelSwapCount->setText(tr("Broj zamjena: 0"));

    vBoxLayoutStats->addWidget(labelElapsedTime);
    vBoxLayoutStats->addWidget(labelStepCount);
    vBoxLayoutStats->addWidget(labelCompareCount);
    vBoxLayoutStats->addWidget(labelSwapCount);

    groupBoxStats->setLayout(vBoxLayoutStats);
    //groupBoxStats->setStyleSheet("border: none");
    groupBoxStats->setStyleSheet("QGroupBox{background-color:rgba(255,255,255,0.8)}");

    viewGridLayout->addWidget(groupBoxStats,0,0,Qt::AlignBottom | Qt::AlignLeft);
}

void MainWindow::prepareStatsOverlay()
{
    labelElapsedTime->setText(tr("Proteklo vrijeme: 00:00"));
    labelStepCount->setText(tr("Broj koraka: 0"));
    labelCompareCount->setText(tr("Broj usporedbi: 0"));
    labelSwapCount->setText(tr("Broj zamjena: 0"));
}

void MainWindow::processAnimation(RectItem *item_i, RectItem *item_j, int duration)
{
    QPropertyAnimation *jAnimation = new QPropertyAnimation(item_j,"geometry");
    jAnimation->setDuration(duration);
    jAnimation->setStartValue(item_j->geometry());
    jAnimation->setEndValue(QRect(item_i->geometry().x(),item_j->geometry().y(),30,item_j->geometry().height()));

    QPropertyAnimation *iAnimation = new QPropertyAnimation(item_i,"geometry");
    iAnimation->setDuration(duration);
    iAnimation->setStartValue(item_i->geometry());
    iAnimation->setEndValue(QRect(item_j->geometry().x(),item_i->geometry().y(),30,item_i->geometry().height()));

    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup;
    animationGroup->addAnimation(jAnimation);
    animationGroup->addAnimation(iAnimation);
    animationGroup->start();
}

void MainWindow::loadStatisticsGraph()
{

    QVBoxLayout *layout0 = new QVBoxLayout;
    QVBoxLayout *layout1 = new QVBoxLayout;
    QVBoxLayout *layout2 = new QVBoxLayout;
    QVBoxLayout *layout3 = new QVBoxLayout;

    ui->groupBoxSteps->setLayout(layout0);
    ui->groupBoxComps->setLayout(layout1);
    ui->groupBoxSwaps->setLayout(layout2);
    ui->groupBoxDuration->setLayout(layout3);

    //steps

    //![1]
        stepsSet0 = new QBarSet(tr("Izborom"));
        stepsSet1 = new QBarSet(tr("Zamjenom"));
        stepsSet2 = new QBarSet(tr("Umetanjem"));
        stepsSet3 = new QBarSet(tr("Mjehuričasto"));
        stepsSet4 = new QBarSet(tr("Dvostruko mjehuričasto"));

        *stepsSet0 << 0;
        *stepsSet1 << 0;
        *stepsSet2 << 0;
        *stepsSet3 << 0;
        *stepsSet4 << 0;
    //![1]

    //![2]
        QBarSeries *stepsSeries = new QBarSeries();
        stepsSeries->append(stepsSet0);
        stepsSeries->append(stepsSet1);
        stepsSeries->append(stepsSet2);
        stepsSeries->append(stepsSet3);
        stepsSeries->append(stepsSet4);

        stepsSeries->setBarWidth(0.3);
    //![2]

    //![3]
        QChart *stepsChart = new QChart();
        stepsChart->addSeries(stepsSeries);
        stepsChart->setTitle(tr("Usporedba algoritama prema broju koraka"));
        stepsChart->setAnimationOptions(QChart::NoAnimation); //QChart::SeriesAnimations //QChart::NoAnimation
    //![3]

    //![4]
        //stepsCategories = new QStringList();
        stepsCategories << tr("Broj koraka") ;
        stepsAxisX = new QBarCategoryAxis();
        stepsAxisX->append(stepsCategories);
        stepsChart->addAxis(stepsAxisX, Qt::AlignBottom);
        stepsSeries->attachAxis(stepsAxisX);

        stepsAxisY = new QValueAxis();
        stepsChart->addAxis(stepsAxisY, Qt::AlignLeft);
        stepsSeries->attachAxis(stepsAxisY);
    //![4]

    //![5]
        stepsChart->legend()->setVisible(true);
        stepsChart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *stepsChartView = new QChartView(stepsChart);
        stepsChartView->setRenderHint(QPainter::Antialiasing);
    //![6]


    //comps

    //![1]
        compsSet0 = new QBarSet(tr("Izborom"));
        compsSet1 = new QBarSet(tr("Zamjenom"));
        compsSet2 = new QBarSet(tr("Umetanjem"));
        compsSet3 = new QBarSet(tr("Mjehuričasto"));
        compsSet4 = new QBarSet(tr("Dvostruko mjehuričasto"));

        *compsSet0 << 0;
        *compsSet1 << 0;
        *compsSet2 << 0;
        *compsSet3 << 0;
        *compsSet4 << 0;
    //![1]

    //![2]
        QBarSeries *compsSeries = new QBarSeries();
        compsSeries->append(compsSet0);
        compsSeries->append(compsSet1);
        compsSeries->append(compsSet2);
        compsSeries->append(compsSet3);
        compsSeries->append(compsSet4);

        compsSeries->setBarWidth(0.3);

    //![2]

    //![3]
        QChart *compsChart = new QChart();
        compsChart->addSeries(compsSeries);
        compsChart->setTitle(tr("Usporedba algoritama prema broju usporedbi"));
        compsChart->setAnimationOptions(QChart::NoAnimation);
    //![3]

    //![4]
        //compsCategories = new QStringList();
        compsCategories << tr("Broj usporedbi") ;
        compsAxisX = new QBarCategoryAxis();
        compsAxisX->append(compsCategories);
        compsChart->addAxis(compsAxisX, Qt::AlignBottom);
        compsSeries->attachAxis(compsAxisX);

        compsAxisY = new QValueAxis();
        compsChart->addAxis(compsAxisY, Qt::AlignLeft);
        compsSeries->attachAxis(compsAxisY);
    //![4]

    //![5]
        compsChart->legend()->setVisible(true);
        compsChart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *compsChartView = new QChartView(compsChart);
        compsChartView->setRenderHint(QPainter::Antialiasing);
    //![6]


    //swaps

    //![1]
        swapsSet0 = new QBarSet(tr("Izborom"));
        swapsSet1 = new QBarSet(tr("Zamjenom"));
        swapsSet2 = new QBarSet(tr("Umetanjem"));
        swapsSet3 = new QBarSet(tr("Mjehuričasto"));
        swapsSet4 = new QBarSet(tr("Dvostruko mjehuričasto"));

        *swapsSet0 << 0;
        *swapsSet1 << 0;
        *swapsSet2 << 0;
        *swapsSet3 << 0;
        *swapsSet4 << 0;
    //![1]

    //![2]
        QBarSeries *swapsSeries = new QBarSeries();
        swapsSeries->append(swapsSet0);
        swapsSeries->append(swapsSet1);
        swapsSeries->append(swapsSet2);
        swapsSeries->append(swapsSet3);
        swapsSeries->append(swapsSet4);

        swapsSeries->setBarWidth(0.3);

    //![2]

    //![3]
        QChart *swapsChart = new QChart();
        swapsChart->addSeries(swapsSeries);
        swapsChart->setTitle(tr("Usporedba algoritama prema broju zamjena"));
        swapsChart->setAnimationOptions(QChart::NoAnimation);
    //![3]

    //![4]
        //swapsCategories = new QStringList();
        swapsCategories << tr("Broj zamjena") ;
        swapsAxisX = new QBarCategoryAxis();
        swapsAxisX->append(swapsCategories);
        swapsChart->addAxis(swapsAxisX, Qt::AlignBottom);
        swapsSeries->attachAxis(swapsAxisX);

        swapsAxisY = new QValueAxis();
        swapsChart->addAxis(swapsAxisY, Qt::AlignLeft);
        swapsSeries->attachAxis(swapsAxisY);
    //![4]

    //![5]
        swapsChart->legend()->setVisible(true);
        swapsChart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *swapsChartView = new QChartView(swapsChart);
        swapsChartView->setRenderHint(QPainter::Antialiasing);
    //![6]


    //duration

    //![1]
        timeSet0 = new QBarSet(tr("Izborom"));
        timeSet1 = new QBarSet(tr("Zamjenom"));
        timeSet2 = new QBarSet(tr("Umetanjem"));
        timeSet3 = new QBarSet(tr("Mjehuričasto"));
        timeSet4 = new QBarSet(tr("Dvostruko mjehuričasto"));

        *timeSet0 << 0;
        *timeSet1 << 0;
        *timeSet2 << 0;
        *timeSet3 << 0;
        *timeSet4 << 0;
    //![1]

    //![2]
        QBarSeries *timeSeries = new QBarSeries();
        timeSeries->append(timeSet0);
        timeSeries->append(timeSet1);
        timeSeries->append(timeSet2);
        timeSeries->append(timeSet3);
        timeSeries->append(timeSet4);

        timeSeries->setBarWidth(0.3);

    //![2]

    //![3]
        QChart *timeChart = new QChart();
        timeChart->addSeries(timeSeries);
        timeChart->setTitle(tr("Usporedba algoritama prema trajanju"));
        timeChart->setAnimationOptions(QChart::NoAnimation);
    //![3]

    //![4]
        QStringList timeCategories;
        timeCategories << tr("Trajanje (msec)") ;
        QBarCategoryAxis *timeAxisX = new QBarCategoryAxis();
        timeAxisX->append(timeCategories);
        timeChart->addAxis(timeAxisX, Qt::AlignBottom);
        timeSeries->attachAxis(timeAxisX);

        timeAxisY = new QValueAxis();
        timeChart->addAxis(timeAxisY, Qt::AlignLeft);
        timeSeries->attachAxis(timeAxisY);
    //![4]

    //![5]
        timeChart->legend()->setVisible(true);
        timeChart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *timeChartView = new QChartView(timeChart);
        timeChartView->setRenderHint(QPainter::Antialiasing);
    //![6]

        layout0->addWidget(stepsChartView);
        layout1->addWidget(compsChartView);
        layout2->addWidget(swapsChartView);
        layout3->addWidget(timeChartView);
}

void MainWindow::prepareStatisticsWorkers()
{
    for(int i = 0; i < ui->groupBoxAlgorithms->findChildren<QCheckBox *>().count(); i++){
        statisticsWorkers.append(new StatisticsWorker);
        connect(statisticsWorkers.at(i),&StatisticsWorker::updateStats,this,&MainWindow::updateStats);
        connect(statisticsWorkers.at(i),&StatisticsWorker::stopStatisticsWorkers,this,&MainWindow::stopStatisticsWorkers);

        //statistics timer trigger
        statisticsTimers.append(new QTimer);
        connect(statisticsTimers.at(i),&QTimer::timeout,statisticsWorkers.at(i),&StatisticsWorker::emitUpdateSignal);
    }
}

void MainWindow::stopStatisticsWorkers(const int &algorithm)
{
    statisticsWorkers.at(algorithm)->stop();
    statisticsTimers.at(algorithm)->stop();

    bool allStopped = true;

//    for(int i = 0; i < 5; i++){
//        if(!statisticsWorkers.at(i)->isFinished()) allStopped = false;
//    }

    for(int i = 0; i < ui->groupBoxAlgorithms->findChildren<QCheckBox *>().count(); i++){

        if(ui->groupBoxAlgorithms->findChildren<QCheckBox *>().at(i)->isChecked() && !statisticsWorkers.at(i)->isFinished()){
            allStopped = false;
        }
    }

    if(allStopped){
        statisticsState = Stopped;

        ui->pushButtonStart->setEnabled(true);
        ui->pushButtonGenerate->setEnabled(true);
        //ui->groupBoxAlgorithms->setEnabled(true);
        ui->checkBoxSelectAll->setEnabled(true);
        ui->horizontalSliderSize->setEnabled(true);

        ui->horizontalSliderRefreshFreq->setEnabled(true);
        ui->pushButtonClearReports->setEnabled(true);

        ui->checkBoxLogarithmize->setEnabled(true);

        if(ui->checkBoxSelectAll->isChecked()) updateAlgorithmCheckboxes(Qt::Checked);
        else updateAlgorithmCheckboxes(Qt::Unchecked);

        ui->statusbar->showMessage(tr("Usporedba algoritama: Završeno"),10000);
    }
}

void MainWindow::updateStats(const int &algorithm, const unsigned long long &steps, const unsigned long long &comps, const unsigned long long &swaps, const long &time)
{
    if(logarithmized){
        //qDebug() << "test1";
        stepsCategories.clear();
        compsCategories.clear();
        swapsCategories.clear();
        stepsCategories << tr("Broj koraka (logaritmirano)") ;
        compsCategories << tr("Broj usporedbi (logaritmirano)") ;
        swapsCategories << tr("Broj zamjena (logaritmirano)") ;

        stepsAxisX->clear();
        stepsAxisX->append(stepsCategories);

        compsAxisX->clear();
        compsAxisX->append(compsCategories);

        swapsAxisX->clear();
        swapsAxisX->append(swapsCategories);
    }
    else{
        //qDebug() << "test2";
        stepsCategories.clear();
        compsCategories.clear();
        swapsCategories.clear();
        stepsCategories << tr("Broj koraka") ;
        compsCategories << tr("Broj usporedbi") ;
        swapsCategories << tr("Broj zamjena") ;

        stepsAxisX->clear();
        stepsAxisX->append(stepsCategories);

        compsAxisX->clear();
        compsAxisX->append(compsCategories);

        swapsAxisX->clear();
        swapsAxisX->append(swapsCategories);
    }

    switch (algorithm) {
    case 0:
    {
        ui->labelSelectionStepCount->setText(tr("Broj koraka: ")+QString::number(steps));
        ui->labelSelectionCompareCount->setText(tr("Broj usporedbi: ")+QString::number(comps));
        ui->labelSelectionSwapCount->setText(tr("Broj zamjena: ")+QString::number(swaps));
        ui->labelSelectionDuration->setText(tr("Trajanje: ")+QString::number((double)time/1000));

        //axisY->setRange(0,std::max({steps,comps,swaps}));

        //steps

        if(steps > stepsMaxVal) {
            logarithmized ? stepsMaxVal = std::ceil(std::log10(steps))
                          : stepsMaxVal = steps;

            stepsAxisY->setRange(0,stepsMaxVal);
        }

        logarithmized ? stepsSet0->replace(0,std::log10(steps))
                      : stepsSet0->replace(0,steps);

        //comps

        if(comps > compsMaxVal) {

            logarithmized ? compsMaxVal = std::ceil(std::log10(comps))
                          : compsMaxVal = comps;

            compsAxisY->setRange(0,compsMaxVal);
        }

        logarithmized ? compsSet0->replace(0,std::log10(comps))
                      : compsSet0->replace(0,comps);

        //swaps

        if(swaps > swapsMaxVal) {

            logarithmized ? swapsMaxVal = std::ceil(std::log10(swaps))
                          : swapsMaxVal = swaps;

            swapsAxisY->setRange(0,swapsMaxVal);
        }

        logarithmized ? swapsSet0->replace(0,std::log10(swaps))
                      : swapsSet0->replace(0,swaps);

        //duration

        if(time > timeMaxVal) {
            timeMaxVal = time;
            timeAxisY->setRange(0,timeMaxVal);
        }

        timeSet0->replace(0,time);

        break;
    }
    case 1:
    {
        ui->labelExchangeStepCount->setText(tr("Broj koraka: ")+QString::number(steps));
        ui->labelExchangeCompareCount->setText(tr("Broj usporedbi: ")+QString::number(comps));
        ui->labelExchangeSwapCount->setText(tr("Broj zamjena: ")+QString::number(swaps));
        ui->labelExchangeDuration->setText(tr("Trajanje: ")+QString::number((double)time/1000));

        //axisY->setRange(0,std::max({steps,comps,swaps}));

        //steps

        if(steps > stepsMaxVal) {
            logarithmized ? stepsMaxVal = std::ceil(std::log10(steps))
                          : stepsMaxVal = steps;

            stepsAxisY->setRange(0,stepsMaxVal);
        }

        logarithmized ? stepsSet1->replace(0,std::log10(steps))
                      : stepsSet1->replace(0,steps);

        //comps

        if(comps > compsMaxVal) {

            logarithmized ? compsMaxVal = std::ceil(std::log10(comps))
                          : compsMaxVal = comps;

            compsAxisY->setRange(0,compsMaxVal);
        }

        logarithmized ? compsSet1->replace(0,std::log10(comps))
                      : compsSet1->replace(0,comps);

        //swaps

        if(swaps > swapsMaxVal) {

            logarithmized ? swapsMaxVal = std::ceil(std::log10(swaps))
                          : swapsMaxVal = swaps;

            swapsAxisY->setRange(0,swapsMaxVal);
        }

        logarithmized ? swapsSet1->replace(0,std::log10(swaps))
                      : swapsSet1->replace(0,swaps);

        //duration

        if(time > timeMaxVal) {
            timeMaxVal = time;
            timeAxisY->setRange(0,timeMaxVal);
        }

        timeSet1->replace(0,time);

        break;
    }
    case 2:
    {
        ui->labelInsertionStepCount->setText(tr("Broj koraka: ")+QString::number(steps));
        ui->labelInsertionCompareCount->setText(tr("Broj usporedbi: ")+QString::number(comps));
        ui->labelInsertionSwapCount->setText(tr("Broj zamjena: ")+QString::number(swaps));
        ui->labelInsertionDuration->setText(tr("Trajanje: ")+QString::number((double)time/1000));

        //axisY->setRange(0,std::max({steps,comps,swaps}));

        //steps

        if(steps > stepsMaxVal) {
            logarithmized ? stepsMaxVal = std::ceil(std::log10(steps))
                          : stepsMaxVal = steps;

            stepsAxisY->setRange(0,stepsMaxVal);
        }

        logarithmized ? stepsSet2->replace(0,std::log10(steps))
                      : stepsSet2->replace(0,steps);

        //comps

        if(comps > compsMaxVal) {

            logarithmized ? compsMaxVal = std::ceil(std::log10(comps))
                          : compsMaxVal = comps;

            compsAxisY->setRange(0,compsMaxVal);
        }

        logarithmized ? compsSet2->replace(0,std::log10(comps))
                      : compsSet2->replace(0,comps);

        //swaps

        if(swaps > swapsMaxVal) {

            logarithmized ? swapsMaxVal = std::ceil(std::log10(swaps))
                          : swapsMaxVal = swaps;

            swapsAxisY->setRange(0,swapsMaxVal);
        }

        logarithmized ? swapsSet2->replace(0,std::log10(swaps))
                      : swapsSet2->replace(0,swaps);

        //duration

        if(time > timeMaxVal) {
            timeMaxVal = time;
            timeAxisY->setRange(0,timeMaxVal);
        }

        timeSet2->replace(0,time);

        break;
    }
    case 3:
    {
        ui->labelBubbleStepCount->setText(tr("Broj koraka: ")+QString::number(steps));
        ui->labelBubbleCompareCount->setText(tr("Broj usporedbi: ")+QString::number(comps));
        ui->labelBubbleSwapCount->setText(tr("Broj zamjena: ")+QString::number(swaps));
        ui->labelBubbleDuration->setText(tr("Trajanje: ")+QString::number((double)time/1000));

        //axisY->setRange(0,std::max({steps,comps,swaps}));

        //steps

        if(steps > stepsMaxVal) {
            logarithmized ? stepsMaxVal = std::ceil(std::log10(steps))
                          : stepsMaxVal = steps;

            stepsAxisY->setRange(0,stepsMaxVal);
        }

        logarithmized ? stepsSet3->replace(0,std::log10(steps))
                      : stepsSet3->replace(0,steps);

        //comps

        if(comps > compsMaxVal) {

            logarithmized ? compsMaxVal = std::ceil(std::log10(comps))
                          : compsMaxVal = comps;

            compsAxisY->setRange(0,compsMaxVal);
        }

        logarithmized ? compsSet3->replace(0,std::log10(comps))
                      : compsSet3->replace(0,comps);

        //swaps

        if(swaps > swapsMaxVal) {

            logarithmized ? swapsMaxVal = std::ceil(std::log10(swaps))
                          : swapsMaxVal = swaps;

            swapsAxisY->setRange(0,swapsMaxVal);
        }

        logarithmized ? swapsSet3->replace(0,std::log10(swaps))
                      : swapsSet3->replace(0,swaps);

        //duration

        if(time > timeMaxVal) {
            timeMaxVal = time;
            timeAxisY->setRange(0,timeMaxVal);
        }

        timeSet3->replace(0,time);

        break;
    }
    case 4:
        ui->labelCocktailStepCount->setText(tr("Broj koraka: ")+QString::number(steps));
        ui->labelCocktailCompareCount->setText(tr("Broj usporedbi: ")+QString::number(comps));
        ui->labelCocktailSwapCount->setText(tr("Broj zamjena: ")+QString::number(swaps));
        ui->labelCocktailDuration->setText(tr("Trajanje: ")+QString::number((double)time/1000));

        //axisY->setRange(0,std::max({steps,comps,swaps}));

        //steps

        if(steps > stepsMaxVal) {
            logarithmized ? stepsMaxVal = std::ceil(std::log10(steps))
                          : stepsMaxVal = steps;

            stepsAxisY->setRange(0,stepsMaxVal);
        }

        logarithmized ? stepsSet4->replace(0,std::log10(steps))
                      : stepsSet4->replace(0,steps);

        //comps

        if(comps > compsMaxVal) {

            logarithmized ? compsMaxVal = std::ceil(std::log10(comps))
                          : compsMaxVal = comps;

            compsAxisY->setRange(0,compsMaxVal);
        }

        logarithmized ? compsSet4->replace(0,std::log10(comps))
                      : compsSet4->replace(0,comps);

        //swaps

        if(swaps > swapsMaxVal) {

            logarithmized ? swapsMaxVal = std::ceil(std::log10(swaps))
                          : swapsMaxVal = swaps;

            swapsAxisY->setRange(0,swapsMaxVal);
        }

        logarithmized ? swapsSet4->replace(0,std::log10(swaps))
                      : swapsSet4->replace(0,swaps);

        //duration

        if(time > timeMaxVal) {
            timeMaxVal = time;
            timeAxisY->setRange(0,timeMaxVal);
        }

        timeSet4->replace(0,time);

        break;

    default:
        break;
    }
}

void MainWindow::resetStats()
{
    ui->labelSelectionStepCount->setText(tr("Broj koraka: ")+QString::number(0));
    ui->labelSelectionCompareCount->setText(tr("Broj usporedbi: ")+QString::number(0));
    ui->labelSelectionSwapCount->setText(tr("Broj zamjena: ")+QString::number(0));
    ui->labelSelectionDuration->setText(tr("Trajanje: ")+QString::number(0));

    ui->labelExchangeStepCount->setText(tr("Broj koraka: ")+QString::number(0));
    ui->labelExchangeCompareCount->setText(tr("Broj usporedbi: ")+QString::number(0));
    ui->labelExchangeSwapCount->setText(tr("Broj zamjena: ")+QString::number(0));
    ui->labelExchangeDuration->setText(tr("Trajanje: ")+QString::number(0));

    ui->labelInsertionStepCount->setText(tr("Broj koraka: ")+QString::number(0));
    ui->labelInsertionCompareCount->setText(tr("Broj usporedbi: ")+QString::number(0));
    ui->labelInsertionSwapCount->setText(tr("Broj zamjena: ")+QString::number(0));
    ui->labelInsertionDuration->setText(tr("Trajanje: ")+QString::number(0));

    ui->labelBubbleStepCount->setText(tr("Broj koraka: ")+QString::number(0));
    ui->labelBubbleCompareCount->setText(tr("Broj usporedbi: ")+QString::number(0));
    ui->labelBubbleSwapCount->setText(tr("Broj zamjena: ")+QString::number(0));
    ui->labelBubbleDuration->setText(tr("Trajanje: ")+QString::number(0));

    ui->labelCocktailStepCount->setText(tr("Broj koraka: ")+QString::number(0));
    ui->labelCocktailCompareCount->setText(tr("Broj usporedbi: ")+QString::number(0));
    ui->labelCocktailSwapCount->setText(tr("Broj zamjena: ")+QString::number(0));
    ui->labelCocktailDuration->setText(tr("Trajanje: ")+QString::number(0));

//    set0->replace(0,0);
//    set0->replace(1,0);
//    set0->replace(2,0);

//    set1->replace(0,0);
//    set1->replace(1,0);
//    set1->replace(2,0);

//    set2->replace(0,0);
//    set2->replace(1,0);
//    set2->replace(2,0);

//    set3->replace(0,0);
//    set3->replace(1,0);
//    set3->replace(2,0);

//    set4->replace(0,0);
//    set4->replace(1,0);
//    set4->replace(2,0);

//    maxVal = 0;

    stepsSet0->replace(0,0);
    stepsSet1->replace(0,0);
    stepsSet2->replace(0,0);
    stepsSet3->replace(0,0);
    stepsSet4->replace(0,0);

    stepsMaxVal = 0;

    //stepsAxisY->setRange(0,0);

    compsSet0->replace(0,0);
    compsSet1->replace(0,0);
    compsSet2->replace(0,0);
    compsSet3->replace(0,0);
    compsSet4->replace(0,0);

    compsMaxVal = 0;

    //compsAxisY->setRange(0,0);

    swapsSet0->replace(0,0);
    swapsSet1->replace(0,0);
    swapsSet2->replace(0,0);
    swapsSet3->replace(0,0);
    swapsSet4->replace(0,0);

    swapsMaxVal = 0;

    //swapsAxisY->setRange(0,0);

    timeSet0->replace(0,0);
    timeSet1->replace(0,0);
    timeSet2->replace(0,0);
    timeSet3->replace(0,0);
    timeSet4->replace(0,0);

    timeMaxVal = 0;

    //timeAxisY->setRange(0,0);
}

void MainWindow::updateAlgorithmCheckboxes(const int &state)
{
    if(state == Qt::Unchecked){
        foreach (QCheckBox *cbx, ui->groupBoxAlgorithms->findChildren<QCheckBox *>()) cbx->setChecked(false);
        ui->groupBoxAlgorithms->setEnabled(true);
    }
    else{
        foreach (QCheckBox *cbx, ui->groupBoxAlgorithms->findChildren<QCheckBox *>()) cbx->setChecked(true);
        ui->groupBoxAlgorithms->setDisabled(true);
    }
}

void MainWindow::on_toolButtonRandomSize_clicked()
{
    do{
        size = QRandomGenerator::global()->bounded(3,21);
    }while(size == ui->spinBoxSize->value());

    ui->spinBoxSize->setValue(size);
}


void MainWindow::on_comboBoxElements_currentIndexChanged(int index)
{

    if(index != 6){
        //resetiranje na default nakon vracanja iz "prilagodeno"
        ui->spinBoxSize->setMinimum(3);
        ui->spinBoxSize->setMaximum(20);

        ui->spinBoxSize->setValue(QRandomGenerator::global()->bounded(3,21));

        ui->spinBoxRange->setMinimum(1);
        ui->spinBoxRange->setMaximum(80);

        ui->spinBoxRange->setValue(50);

        ui->toolButtonRandomElements->setEnabled(true);
        ui->toolButtonRandomSize->setEnabled(true);
        ui->toolButtonRandomRange->setEnabled(true);

        ui->lineEditElements->setPlaceholderText("");
    }

    generateElements(index);
    generateItems();
}


void MainWindow::on_toolButtonRandomElements_clicked()
{
    generateElements(ui->comboBoxElements->currentIndex());
    generateItems();
}

void MainWindow::on_toolButtonPlayPause_clicked()
{
    if(numbers->count() < 3){
        QMessageBox::critical(this,tr("Greška"),tr("Neispravan unos! Unos mora sadržavati najmanje 3 i najviše 20 brojeva za vizualizaciju."));
        return;
    }

    ui->comboBoxElements->setDisabled(true);
    if(ui->comboBoxElements->currentIndex() == 6) ui->lineEditElements->setReadOnly(true);
    ui->toolButtonRandomElements->setDisabled(true);

    ui->toolButtonRandomSize->setDisabled(true);
    ui->spinBoxSize->setMinimum(numbers->count());
    ui->spinBoxSize->setMaximum(numbers->count());

    ui->radioButtonAscending->setDisabled(true);
    ui->radioButtonDescending->setDisabled(true);

    if(animationState == Stopped){ //start
        animationState = Playing;

        ui->toolButtonPlayPause->setIcon(QIcon("://images/icons8-pause-48.png"));
        ui->toolButtonPlayPause->setToolTip(tr("Pauziraj"));

        animationWorker->start(QThread::Priority::TimeCriticalPriority);

        ui->comboBoxAlgorithm->setDisabled(true);

        prepareStatsOverlay();

        animationTimeElapsed = 0;

        labelElapsedTime->setText(tr("Proteklo vrijeme: ")+currentTimeElapsed->addSecs(animationTimeElapsed++).toString("mm:ss"));

        animationTimer->start(1000);

        ui->statusbar->showMessage(tr("Vizualizacija: Pokrenuto"));

        return;
    }

    if(animationState == Playing){ //pause
        animationState = Paused;

        ui->toolButtonPlayPause->setIcon(QIcon("://images/icons8-play-48.png"));
        ui->toolButtonPlayPause->setToolTip(tr("Pokreni"));

        animationWorker->pause();

        //prebaceno u signal koji salje dretva da je pauzirana
        //animationTimer->stop();

        ui->statusbar->showMessage(tr("Vizualizacija: Pauziranje..."));

        return;
    }

    if(animationState == Paused){ //resume
        animationState = Playing;

        ui->toolButtonPlayPause->setIcon(QIcon("://images/icons8-pause-48.png"));
        ui->toolButtonPlayPause->setToolTip(tr("Pauziraj"));

        animationWorker->resume();

        animationTimer->start(1000);

        ui->statusbar->showMessage(tr("Vizualizacija: Pokrenuto"));

        return;
    }
}


void MainWindow::on_toolButtonStop_clicked()
{   
    if(animationState != Stopped) stopAnimationWorker();

    //ui->statusbar->showMessage(tr("Vizualizacija: Zaustavljeno"));
}


void MainWindow::on_toolButtonReplay_clicked()
{
    if(animationState != Stopped) stopAnimationWorker();
    generateItems();

    //ui->statusbar->showMessage(tr("Vizualizacija: Spremno"));
}


void MainWindow::on_horizontalSliderSpeed_valueChanged(int value)
{
    currentSpeedFactor = (double)value/100;
    ui->labelSpeed->setText(QString::number(currentSpeedFactor,'f',2)+"x");
    animationWorker->currentSpeedFactor = 1/currentSpeedFactor;
}


void MainWindow::on_toolButtonZoomIn_clicked()
{
    double scaleFactor = (currentScale+25)/currentScale;
    if(currentScale<300){
        ui->graphicsView->scale(scaleFactor,scaleFactor);
        currentScale*=scaleFactor;
        ui->labelZoom->setText(QString::number(currentScale)+"%");
    }
}


void MainWindow::on_toolButtonZoomOut_clicked()
{
    double scaleFactor = (currentScale-25)/currentScale;
    if(currentScale>25){
        ui->graphicsView->scale(scaleFactor,scaleFactor);
        currentScale*=scaleFactor;
        ui->labelZoom->setText(QString::number(currentScale)+"%");
    }
}


void MainWindow::on_toolButtonZoomReset_clicked()
{
    ui->graphicsView->resetTransform();
    currentScale=100;
    ui->labelZoom->setText(QString::number(currentScale)+"%");
}


void MainWindow::on_comboBoxAlgorithm_currentIndexChanged(int index)
{
    animationWorker->currentAlgorithm = index;
    prepareCodeOverlay(index);
    currentAlgorithm = index;
}


void MainWindow::on_lineEditElements_textEdited(const QString &text)
{
    bool isValid = true;
    static QString lastValid;

    static QRegularExpression regex("^[0-9 ]+$");

    if(!regex.match(text.trimmed()).hasMatch()){
        isValid = false;
    }

    numbers->clear();

    numbers->append(text.trimmed().split(" ", Qt::SkipEmptyParts));

    for (int i = 0; i < numbers->count(); i++) {
        if(numbers->at(i).toInt() < 1 || numbers->at(i).toInt() > 80){
            isValid = false;
            break;
        }
    }

    if(numbers->count() > 20) isValid = false;

    if(text.isEmpty()) isValid = true;

    if(!isValid){
        QMessageBox::critical(this,tr("Greška"),tr("Neispravan unos! Unos može sadržavati najviše 20 brojeva u rasponu 1-80."));
        ui->lineEditElements->setText(lastValid);
        ui->lineEditElements->setCursorPosition(lastValid.length());
    }
    else{
        lastValid = text.trimmed();
        generateItems();

        ui->spinBoxSize->setValue(numbers->count());
        ui->spinBoxSize->setMinimum(numbers->count());
        ui->spinBoxSize->setMaximum(numbers->count());

        if(!numbers->empty()){
            auto maxelem = std::max_element(numbers->begin(), numbers->end(), [=](const QString &numbers1, const QString &numbers2)->bool{
                return numbers1.toInt() < numbers2.toInt();
            });

            ui->spinBoxRange->setMinimum(numbers->at(std::distance(numbers->begin(), maxelem)).toInt());
            ui->spinBoxRange->setMaximum(numbers->at(std::distance(numbers->begin(), maxelem)).toInt());
        }
        else{
            ui->spinBoxRange->setMinimum(0);
            ui->spinBoxRange->setMaximum(0);
        }

    }
}


void MainWindow::on_radioButtonAscending_clicked(bool checked)
{
    if(checked){
        currentOrder = MainWindow::CurrentOrder::ASC;
        animationWorker->currentOrder = (int)currentOrder;
        prepareCodeOverlay(currentAlgorithm);
    }
}


void MainWindow::on_radioButtonDescending_clicked(bool checked)
{
    if(checked){
        currentOrder = MainWindow::CurrentOrder::DESC;
        animationWorker->currentOrder = (int)currentOrder;
        prepareCodeOverlay(currentAlgorithm);
    }
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch (index) {
    case 0:

        ui->statusbar->showMessage(visualizationLastMessage);

        break;

    case 1:
        if(animationState == Playing) {
            animationState = Paused;

            ui->toolButtonPlayPause->setIcon(QIcon("://images/icons8-play-48.png"));
            ui->toolButtonPlayPause->setToolTip(tr("Pokreni"));

            animationWorker->pause();

            //prebaceno u signal koji salje dretva da je pauzirana
            //animationTimer->stop();

            visualizationLastMessage = tr("Vizualizacija: Pauzirano");

            //ui->statusbar->clearMessage();
        }
        else{
            visualizationLastMessage = ui->statusbar->currentMessage();

            ui->statusbar->clearMessage();
        }

        break;

    default:
        break;
    }
}


void MainWindow::on_horizontalSliderSize_valueChanged(int value)
{
    ui->labelSize->setText(QString::number(value));
}


void MainWindow::on_pushButtonGenerate_clicked()
{
    arr.clear();
    qDebug() << "Generiranje...";
    ui->statusbar->showMessage(tr("Usporedba algoritama: Generiranje..."));
    //QList<int> *tmp = new QList<int>;
    for(int i = 0; i < ui->horizontalSliderSize->value(); i++){
        arr.append(QRandomGenerator::global()->bounded(QRandomGenerator::max())); //QRandomGenerator::max()
    }
    //ui->pushButtonStart->setEnabled(true);
    qDebug() << "Generirano!";
    ui->statusbar->showMessage(tr("Usporedba algoritama: Generirano"),10000);
}


void MainWindow::on_checkBoxSelectAll_stateChanged(int state)
{
    updateAlgorithmCheckboxes(state);
}


void MainWindow::on_pushButtonStart_clicked()
{
//    qDebug() << ui->groupBoxAlgorithms->findChildren<QCheckBox *>().count();
//    qDebug() << statisticsWorkers.count();
//    if(statisticsWorkers.empty()) prepareStatisticsWorkers();

    if(arr.empty()){
        QMessageBox::warning(this,tr("Greška"),tr("Ništa nije generirano!"));
        return;
    }

    resetStats();

    bool isAnyChecked = false;

    for(int i = 0; i < ui->groupBoxAlgorithms->findChildren<QCheckBox *>().count(); i++){

        if(ui->groupBoxAlgorithms->findChildren<QCheckBox *>().at(i)->isChecked()){
            statisticsWorkers.at(i)->arr = arr;
            statisticsWorkers.at(i)->algorithm = i;

            statisticsWorkers.at(i)->start(QThread::HighestPriority);
            if(refreshFreq) statisticsTimers.at(i)->start(refreshFreq*1000);

            isAnyChecked = true;
        }
    }

    if(!isAnyChecked) {
        QMessageBox::warning(this,tr("Greška"),tr("Nije odabran niti jedan algoritam!"));
        return;
    }

    statisticsState = Playing;

    ui->pushButtonStart->setDisabled(true);
    ui->pushButtonGenerate->setDisabled(true);
    ui->groupBoxAlgorithms->setDisabled(true);
    ui->checkBoxSelectAll->setDisabled(true);
    ui->horizontalSliderSize->setDisabled(true);

    ui->horizontalSliderRefreshFreq->setDisabled(true);
    ui->pushButtonClearReports->setDisabled(true);

    ui->checkBoxLogarithmize->setDisabled(true);

    ui->statusbar->showMessage(tr("Usporedba algoritama: Pokrenuto"));

    ui->labelSize->setText(QString::number(arr.size()));
    ui->horizontalSliderSize->setValue(arr.size());
}


void MainWindow::on_pushButtonStop_clicked()
{
    for(int i = 0; i < 5; i++){
        if(!statisticsWorkers.at(i)->isFinished()){
            statisticsWorkers.at(i)->stop();
            statisticsTimers.at(i)->stop();
        }
    }

    statisticsState = Stopped;

    ui->pushButtonStart->setEnabled(true);
    ui->pushButtonGenerate->setEnabled(true);
    //ui->groupBoxAlgorithms->setEnabled(true);
    ui->checkBoxSelectAll->setEnabled(true);
    ui->horizontalSliderSize->setEnabled(true);

    ui->horizontalSliderRefreshFreq->setEnabled(true);
    ui->pushButtonClearReports->setEnabled(true);

    ui->checkBoxLogarithmize->setEnabled(true);

    if(ui->checkBoxSelectAll->isChecked()) updateAlgorithmCheckboxes(Qt::Checked);
    else updateAlgorithmCheckboxes(Qt::Unchecked);

    ui->statusbar->showMessage(tr("Usporedba algoritama: Zaustavljeno"),10000);
}


void MainWindow::on_actionAbout_triggered()
{
    if(aboutDialog){
        aboutDialog->activateWindow();
    }
    else{
        aboutDialog = new AboutDialog(this);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
        aboutDialog->setWindowFlags(aboutDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        aboutDialog->show();
    }
}


void MainWindow::on_actionExit_triggered()
{
    close();
}


void MainWindow::on_toolButtonRandomRange_clicked()
{
    do{
        range = QRandomGenerator::global()->bounded(1,81);
    }while(range == ui->spinBoxRange->value());

    ui->spinBoxRange->setValue(range);
}


void MainWindow::on_checkBoxLogarithmize_stateChanged(int arg1)
{
    logarithmized = arg1;
}


void MainWindow::on_horizontalSliderRefreshFreq_valueChanged(int value)
{
    ui->labelRefreshFreq->setText(QString::number(value)+" s");
    refreshFreq = value;
}


void MainWindow::on_pushButtonClearReports_clicked()
{
    resetStats();
}

