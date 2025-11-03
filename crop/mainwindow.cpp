#include "mainwindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QProgressDialog>
#include <QApplication>
#include <QStatusBar>
#include <QDebug>
#include <QPushButton>        
#include <QClipboard>          

#include "range_slider.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_videoLabel(new QLabel(this)),
      m_rangeSlider(new RangeSlider(Qt::Horizontal, this))
{
    createUi();
    createMenu();

    // premier affichage dans la status bar
    statusBar()->showMessage(tr("Aucune vidéo"));
}

MainWindow::~MainWindow() = default;

void MainWindow::createUi()
{
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    m_videoLabel->setMinimumSize(640, 360);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setText("Aucune vidéo");
    layout->addWidget(m_videoLabel);

    m_rangeSlider->setMinimum(0);
    m_rangeSlider->setMaximum(100);
    m_rangeSlider->setLow(0);
    m_rangeSlider->setHigh(100);
    layout->addWidget(m_rangeSlider);

    setCentralWidget(central);

    connect(m_rangeSlider, &RangeSlider::sliderMoved,
            this, &MainWindow::onRangeSliderMoved);

    m_copyBtn = new QPushButton("📋", this);
    m_copyBtn->setMaximumWidth(30);
    m_copyBtn->setToolTip(tr("Copier In/Out dans le presse-papiers"));
    m_copyBtn->setFlat(true);           // pour qu'il fasse plus "status bar"
    statusBar()->addPermanentWidget(m_copyBtn);
    connect(m_copyBtn, &QPushButton::clicked,
            this, &MainWindow::onCopyTimeline);

}

void MainWindow::createMenu()
{
    auto *fileMenu = menuBar()->addMenu(tr("Fichier"));

    auto *openAct = fileMenu->addAction(tr("Ouvrir une vidéo..."));
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenVideo);

    auto *saveAct = fileMenu->addAction(tr("Sauvegarder le segment..."));
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveSegment);

    auto *ffmpegAct = fileMenu->addAction(tr("Exporter le segment (ffmpeg)..."));
    connect(ffmpegAct, &QAction::triggered, this, &MainWindow::onExportFfmpeg);
}

void MainWindow::onOpenVideo()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Ouvrir une vidéo"),
        QString(),
        tr("Vidéos (*.mp4 *.avi *.mkv *.mov *.mpg *.mpeg);;Tous les fichiers (*)")
    );

    if (path.isEmpty())
        return;

    openVideo(path);
}

void MainWindow::openVideo(const QString &path)
{
    m_cap.release();
    m_currentVideoPath.clear();

    m_cap.open(path.toStdString());
    if (!m_cap.isOpened()) {
        QMessageBox::critical(this, tr("Erreur"),
                              tr("Impossible d'ouvrir la vidéo : %1").arg(path));
        return;
    }

    m_currentVideoPath = path;

    m_totalFrames = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_COUNT));
    m_fps = m_cap.get(cv::CAP_PROP_FPS);
    int w = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int h = static_cast<int>(m_cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_frameSize = cv::Size(w, h);

    if (m_totalFrames <= 0) {
        m_totalFrames = 1;
    }

    m_rangeSlider->setMinimum(0);
    m_rangeSlider->setMaximum(m_totalFrames - 1);
    m_rangeSlider->setLow(0);
    m_rangeSlider->setHigh(m_totalFrames - 1);

    m_currentLow = 0;
    m_currentHigh = m_totalFrames - 1;

    showFrame(0);

    // mettre les timecodes dès l'ouverture
    QString msg = QString("In: %1  |  Out: %2")
                      .arg(frameToTimecode(m_currentLow))
                      .arg(frameToTimecode(m_currentHigh));
    statusBar()->showMessage(msg);
}

void MainWindow::onRangeSliderMoved(int low, int high)
{
    bool lowChanged  = (low  != m_currentLow);
    bool highChanged = (high != m_currentHigh);

    m_currentLow = low;
    m_currentHigh = high;

    // AFFICHAGE DANS LA BARRE DU BAS
    QString msg = QString("In: %1  |  Out: %2")
                      .arg(frameToTimecode(m_currentLow))
                      .arg(frameToTimecode(m_currentHigh));
    statusBar()->showMessage(msg);

    // preview
    if (highChanged) {
        showFrame(high);
    } else if (lowChanged) {
        showFrame(low);
    } else {
        showFrame(low);
    }
}

void MainWindow::showFrame(int frameIndex)
{
    if (!m_cap.isOpened())
        return;

    m_cap.set(cv::CAP_PROP_POS_FRAMES, frameIndex);

    cv::Mat frame;
    if (!m_cap.read(frame)) {
        qWarning() << "Impossible de lire la frame" << frameIndex;
        return;
    }

    QImage img = matToQImage(frame);
    m_videoLabel->setPixmap(QPixmap::fromImage(img).scaled(
        m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    ));
}

QImage MainWindow::matToQImage(const cv::Mat &mat)
{
    if (mat.empty())
        return QImage();

    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage((const uchar*)rgb.data, rgb.cols, rgb.rows,
                      rgb.step, QImage::Format_RGB888).copy();
    } else if (mat.channels() == 1) {
        return QImage((const uchar*)mat.data, mat.cols, mat.rows,
                      mat.step, QImage::Format_Grayscale8).copy();
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGRA2RGBA);
        return QImage((const uchar*)rgb.data, rgb.cols, rgb.rows,
                      rgb.step, QImage::Format_RGBA8888).copy();
    }

    return QImage();
}

QString MainWindow::frameToTimecode(int frame) const
{
    // fps inconnu → afficher juste le numéro de frame
    if (m_fps <= 0.0) {
        return QString("frame %1").arg(frame);
    }

    double seconds = double(frame) / m_fps;

    int h = int(seconds) / 3600;
    int m = (int(seconds) % 3600) / 60;
    int s = int(seconds) % 60;
    int ms = int((seconds - int(seconds)) * 1000.0 + 0.5);

    return QString("%1:%2:%3.%4")
            .arg(h, 2, 10, QLatin1Char('0'))
            .arg(m, 2, 10, QLatin1Char('0'))
            .arg(s, 2, 10, QLatin1Char('0'))
            .arg(ms, 3, 10, QLatin1Char('0'));
}


void MainWindow::onCopyTimeline()
{
    // texte que l'on affiche dans la status bar
    QString text = QString("In: %1 | Out: %2")
                       .arg(frameToTimecode(m_currentLow))
                       .arg(frameToTimecode(m_currentHigh));

    QClipboard *cb = QGuiApplication::clipboard();
    if (cb) {
        cb->setText(text);
        statusBar()->showMessage(tr("Timecodes copiés : %1").arg(text), 3000);
    }
}

void MainWindow::onSaveSegment()
{
    if (!m_cap.isOpened()) {
        QMessageBox::warning(this, tr("Avertissement"),
                             tr("Aucune vidéo n'est ouverte."));
        return;
    }

    int low  = m_currentLow;
    int high = m_currentHigh;

    if (low < 0) low = 0;
    if (high >= m_totalFrames) high = m_totalFrames - 1;

    if (low >= high) {
        QMessageBox::warning(this, tr("Avertissement"),
                             tr("Le segment sélectionné est vide ou invalide."));
        return;
    }

    const QString outPath = QFileDialog::getSaveFileName(
        this,
        tr("Sauvegarder le segment"),
        QStringLiteral("segment.mp4"),
        tr("Vidéos (*.mp4 *.avi);;Tous les fichiers (*)")
    );
    if (outPath.isEmpty())
        return;

    int fourcc = static_cast<int>(m_cap.get(cv::CAP_PROP_FOURCC));
    if (fourcc == 0) {
        fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    }

    cv::VideoWriter writer;
    bool ok = writer.open(outPath.toStdString(),
                          fourcc,
                          m_fps > 0 ? m_fps : 25.0,
                          m_frameSize,
                          true);
    if (!ok) {
        QMessageBox::critical(this, tr("Erreur"),
                              tr("Impossible de créer le fichier vidéo de sortie."));
        return;
    }

    int frameCountToWrite = high - low + 1;
    QProgressDialog progress(tr("Sauvegarde du segment..."), tr("Annuler"), 0, frameCountToWrite, this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0);

    for (int i = low, written = 0; i <= high; ++i, ++written) {
        if (progress.wasCanceled())
            break;

        m_cap.set(cv::CAP_PROP_POS_FRAMES, i);
        cv::Mat frame;
        if (!m_cap.read(frame)) {
            qWarning() << "Lecture interrompue pendant l'export à la frame" << i;
            break;
        }

        writer.write(frame);

        progress.setValue(written);
        QApplication::processEvents();
    }

    progress.setValue(frameCountToWrite);
    writer.release();

    if (progress.wasCanceled()) {
        QMessageBox::information(this, tr("Annulé"),
                                 tr("La sauvegarde a été annulée."));
    } else {
        QMessageBox::information(this, tr("Terminé"),
                                 tr("Segment sauvegardé dans :\n%1").arg(outPath));
    }
}

void MainWindow::onExportFfmpeg()
{
    if (m_currentVideoPath.isEmpty() || !m_cap.isOpened()) {
        QMessageBox::warning(this, tr("Avertissement"),
                             tr("Aucune vidéo n'est ouverte."));
        return;
    }

    int low  = m_currentLow;
    int high = m_currentHigh;

    if (low < 0) low = 0;
    if (high >= m_totalFrames) high = m_totalFrames - 1;
    if (low >= high) {
        QMessageBox::warning(this, tr("Avertissement"),
                             tr("Le segment sélectionné est vide ou invalide."));
        return;
    }

    const QString outPath = QFileDialog::getSaveFileName(
        this,
        tr("Exporter le segment (ffmpeg)"),
        QStringLiteral("segment_ffmpeg.mp4"),
        tr("Vidéos (*.mp4 *.mkv *.mov *.avi);;Tous les fichiers (*)")
    );
    if (outPath.isEmpty())
        return;

    double fps = (m_fps > 0.0) ? m_fps : 25.0;
    double startSec = double(low) / fps;
    double endSec   = double(high) / fps;
    double duration = endSec - startSec;

    auto secToQString = [](double s) -> QString {
        return QString::number(s, 'f', 3);
    };

    QString startStr = secToQString(startSec);
    QString durStr   = secToQString(duration);

    QString inQuoted  = QString("\"%1\"").arg(m_currentVideoPath);
    QString outQuoted = QString("\"%1\"").arg(outPath);

    QString cmd = QString("ffmpeg -y -ss %1 -i %2 -t %3 -c copy %4")
                      .arg(startStr)
                      .arg(inQuoted)
                      .arg(durStr)
                      .arg(outQuoted);

    qDebug() << "Commande ffmpeg:" << cmd;

    int ret = std::system(cmd.toLocal8Bit().constData());

    if (ret == 0) {
        QMessageBox::information(this, tr("Terminé"),
                                 tr("Segment exporté avec ffmpeg dans :\n%1").arg(outPath));
    } else {
        QMessageBox::critical(this, tr("Erreur"),
                              tr("ffmpeg a retourné le code %1.\nCommande :\n%2")
                                  .arg(ret)
                                  .arg(cmd));
    }
}

