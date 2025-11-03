#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <opencv2/opencv.hpp>

class QLabel;
class RangeSlider;
class QPushButton;   // <- AJOUT

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openVideo(const QString &path);

private slots:
    void onOpenVideo();
    void onSaveSegment();
    void onExportFfmpeg();
    void onRangeSliderMoved(int low, int high);
    void onCopyTimeline();           // <- AJOUT

private:
    void createUi();
    void createMenu();
    void showFrame(int frameIndex);
    QImage matToQImage(const cv::Mat &mat);
    QString frameToTimecode(int frame) const;

    void updateStatusBarText();      // <- AJOUT pour factoriser

private:
    QLabel *m_videoLabel;
    RangeSlider *m_rangeSlider;
    QPushButton *m_copyBtn;          // <- AJOUT

    cv::VideoCapture m_cap;
    QString m_currentVideoPath;

    int m_totalFrames = 0;
    double m_fps = 0.0;
    cv::Size m_frameSize;

    int m_currentLow = 0;
    int m_currentHigh = 0;
};

#endif // MAINWINDOW_H

