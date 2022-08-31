#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//#include <QListWidgetItem>
#include <rectitem.h>

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include <animationworker.h>

#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QTime>

#include <statisticsworker.h>

#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QPointer>

class AboutDialog;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_toolButtonRandomSize_clicked();

    void on_comboBoxElements_currentIndexChanged(int index);

    void on_toolButtonRandomElements_clicked();

    void on_toolButtonPlayPause_clicked();

    void on_toolButtonStop_clicked();

    void on_toolButtonReplay_clicked();

    void on_horizontalSliderSpeed_valueChanged(int value);

    void on_toolButtonZoomIn_clicked();

    void on_toolButtonZoomOut_clicked();

    void on_toolButtonZoomReset_clicked();

    void on_comboBoxAlgorithm_currentIndexChanged(int index);

    void on_lineEditElements_textEdited(const QString &text);

    void on_radioButtonAscending_clicked(bool checked);

    void on_radioButtonDescending_clicked(bool checked);

    void on_tabWidget_currentChanged(int index);

    void on_horizontalSliderSize_valueChanged(int value);

    void on_pushButtonGenerate_clicked();

    void on_checkBoxSelectAll_stateChanged(int state);

    void on_pushButtonStart_clicked();

    void on_pushButtonStop_clicked();

    void on_actionAbout_triggered();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;

    //VISUALIZATION:
    void loadGraphicsScene();
    void generateElements(const int &index);
    void generateItems();
    int calculateItemOffset();
    void delay(int mSecs);

    void stopAnimationWorker();
    void updateAnimationElapsedTime();

    void loadCodeOverlay();
    void prepareCodeOverlay(const int &algorithmIndex);
    void updateCodeOverlay(const int &checkpoint, const int &stepCount, const int &compareCount, const int &swapCount);

    void loadStatsOverlay();
    void prepareStatsOverlay();

    void processAnimation(RectItem *item_i, RectItem *item_j, int duration);

    enum WorkerState {Playing, Paused, Stopped } animationState, statisticsState;

    QGraphicsScene *scene;

    int size;
    QStringList *numbers;
    QList<RectItem *> items;

    AnimationWorker *animationWorker;

    double currentSpeedFactor;
    double currentScale;

    QTimer *animationTimer;
    QTime *currentTimeElapsed;
    int animationTimeElapsed;

    QGridLayout *viewGridLayout;

    QGroupBox *groupBoxStats;
    QVBoxLayout *vBoxLayoutStats;
    QLabel *labelElapsedTime;
    QLabel *labelStepCount;
    QLabel *labelCompareCount;
    QLabel *labelSwapCount;

    QGroupBox *groupBoxCode;
    QVBoxLayout *vBoxLayoutCode;
    QList <QLabel *> *currentAlgorithmCode;

    int currentAlgorithm;

    enum CurrentOrder {ASC, DESC} currentOrder;

    //STATISTICS:
    void loadStatisticsGraph();
    void prepareStatisticsWorkers();

    void stopStatisticsWorkers(const int &algorithm);

    void updateStats(const int &algorithm, const unsigned long long &steps, const unsigned long long &comps, const unsigned long long &swaps, const long &time);

    void resetStats();

    void updateAlgorithmCheckboxes(const int &state);

    QList<unsigned int> arr;

    QList<StatisticsWorker *> statisticsWorkers;

    //GRAPH:

    //steps
    QBarSet *stepsSet0;
    QBarSet *stepsSet1;
    QBarSet *stepsSet2;
    QBarSet *stepsSet3;
    QBarSet *stepsSet4;

    QValueAxis *stepsAxisY;

    unsigned long long stepsMaxVal;

    //comps
    QBarSet *compsSet0;
    QBarSet *compsSet1;
    QBarSet *compsSet2;
    QBarSet *compsSet3;
    QBarSet *compsSet4;

    QValueAxis *compsAxisY;

    unsigned long long compsMaxVal;

    //swaps
    QBarSet *swapsSet0;
    QBarSet *swapsSet1;
    QBarSet *swapsSet2;
    QBarSet *swapsSet3;
    QBarSet *swapsSet4;

    QValueAxis *swapsAxisY;

    unsigned long long swapsMaxVal;

    //duration
    QBarSet *timeSet0;
    QBarSet *timeSet1;
    QBarSet *timeSet2;
    QBarSet *timeSet3;
    QBarSet *timeSet4;

    QValueAxis *timeAxisY;

    long timeMaxVal;

    //ABOUT

    QPointer<AboutDialog> aboutDialog;
};
#endif // MAINWINDOW_H
