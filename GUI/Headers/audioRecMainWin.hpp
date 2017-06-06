#ifndef audioRecMainWin_h
#define audioRecMainWin_h

#define QT_DEBUG_PLUGINS 1

/* Libreria Qt */
#include <QWidget>
#include <QLineEdit>
#include <QtGui>
#include <QFileDialog>
#include <QDrag>
#include <QThread>
#include <QMessageBox>
#include <QRegExp>
#include <QSharedPointer>
#include <QDir>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioBuffer>
#include <QtMultimedia/QAudioOutput>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
/* Libreria personale */
#include <ui_audioRecMain.h>
#include <Headers/AudioProcess.hpp>
#include <Headers/VoiceFeatures.hpp>

namespace Ui {
	class AudioRecMainWin;
}

/** Main widget that manages user interaction with the associated GUI. */
class Ui::AudioRecMainWin : public QWidget
{
	Q_OBJECT
	
public:
	explicit AudioRecMainWin(QWidget *parent = 0); /**< Sets up the main window */
	~AudioRecMainWin();
	
private:
	Ui::mainWin* ui; /**< Pointer to the user interface class */
	const QStringList supportedAudioFormats = {"*.wav"}; /**< List of supported audio file formats */
	int processed, /**< Number of audio files processed */
	fileCount; /**< Total number of audio files to be processed */
	
	/** Returns the audio files contained in the given path.
	 This function checks whether the input path is a directory or a file. In the former case
	 the list of all audio files contained is returned, whereas in the latter case the
	 file itself is returned. In case the path is invalid or no file supported format is found,
	 an empty list is returned.
	 @param[in] dir Input path (directory or audio file)
	 @return The list of audio files found in the path.
	 */
	QSharedPointer<QStringList> listFilesInDir(QSharedPointer<QDir> dir,
											   const bool& absolutePath = true,
											   const QStringList& exts = {});
	
	private slots:
	/** Input browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonInputBrowse_clicked();
	
	/** Input database browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonInputBrowseDB_clicked();
	
	/** Output browse button callback.
	 Opens a file dialog when the output browse button is pressed
	 */
	void on_ButtonOutputBrowse_clicked();
	
	/** Compute button callback, starts audio processing.
	 Start processing the input data on the maximum number of threads available.
	 On each opened thread an instance of AudioProcess runs.
	 */
	void on_ButtonCompute_clicked();
	
	/** Create database button callback.
	 Computes the distances between every pair of feature vectors.
	 */
	void on_ButtonCreateDatabase_clicked();
	
	/** Input line edit callback.
	 This callback executes whenever the text in the widget changes. It automatically updates the
	 combo box with the list of audio files found.
	 */
	void on_LineEditInput_textChanged(QString);
	
	/** Combo box for file choice callback.
	 This callback is executed whenever the user click on a choice in the combo box.
	 This function executes a thread to load the selected audio file, and then make the first record show.
	 */
	void on_ComboChooseFile_activated(QString);
	
	/** Display an error.
	 Displays an error as a popup, with optional description, without quitting the graphical interface.
	 */
	void popupError(QString errorDescription = "");

	/** Handle the processEnded signal emitted by AudioProcess.
	 This function updates the completion label every time an instance of AudioProcess finishes running.
	 */
	void on_OneProcessEnded(QSharedPointer<QDir>);
	
	/** Slot to respond to parameters change */
	void on_ComboParRecLength_activated(int idx) {Application::setParameter(ParRecLength, QLocale().toInt(ui->ComboParRecLength->currentText()));}
	
	/** Slot to respond to parameters change */
	void on_SpinParGaussFilterRad_valueChanged(int val) {Application::setParameter(ParGaussFilterRad, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstMinFilterRad_valueChanged(int val) {Application::setParameter(ParBackEstMinFilterRad, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstMaxPeakWidthAllowed_valueChanged(double val) {Application::setParameter(ParBackEstMaxPeakWidthAllowed, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstDerEstimationDiam_valueChanged(int val) {Application::setParameter(ParBackEstDerEstimationDiam, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstMaxIterations_valueChanged(int val) {Application::setParameter(ParBackEstMaxIterations, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstMaxAllowedInconsistency_valueChanged(double val) {Application::setParameter(ParBackEstMaxAllowedInconsistency, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBackEstMaxDistNodes_valueChanged(int val) {Application::setParameter(ParBackEstMaxDistNodes, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParIntervalStartFreq_valueChanged(double val) {Application::setParameter(ParIntervalStartFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParIntervalWidthFreq_valueChanged(double val) {Application::setParameter(ParIntervalWidthFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParForeGaussFilterRad_valueChanged(int val) {Application::setParameter(ParForeGaussFilterRad, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinParBinWidth_valueChanged(int val) {Application::setParameter(ParBinWidth, val);}
	
	/** Send to charts the command to display the next record */
	void on_ButtonNextRecord_clicked();
	
	/** Send to charts the command to display the previous record */
	void on_ButtonPreviousRecord_clicked();
	
	/** Handle log scale checkbox. */
	void on_CheckLogScale_stateChanged(int state){
		if (ui->CheckLogScale->isChecked()){
			ui->ChartShowRecSpectrum->setLogScale();
		} else {
			ui->ChartShowRecSpectrum->setNaturalScale();
		}
	}
	
	/** Draw the given image in the window and save it in the output folder. */
	void on_imageGenerated(QSharedPointer<QDir> inputDir, QSharedPointer<QImage> image);
	
	/** Show next image in list. */
	void on_ButtonNextImage_clicked();
	
	/** Show previous image in list. */
	void on_ButtonPrevImage_clicked();
	
	/** Clear the DB graphic view. */
	void on_ButtonCleanPlot_clicked();
	
	/** Change image description. */
	void on_changedImage(QString);
	
	/** Save the feature vector to file.
	 Activated when a new feature vector is computed.
	 */
	void on_newFeatures(QSharedPointer<QDir> dir,
						QSharedPointer<QVector<QVector<double>>> features);
	
signals:
	/** Signal emitted when a file path has been selected */
	void readyToDisplay(QSharedPointer<QDir> dir);
};

#endif // audioRecMainWin_h
