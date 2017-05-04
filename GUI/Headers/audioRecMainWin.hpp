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
#include <QtMultimedia/QAudioDecoder>
#include <QtMultimedia/QAudioBuffer>
/* Libreria personale */
#include <ui_audioRecMain.h>
#include <Headers/AudioProcess.hpp>

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
	 @param [in] dir Input path (directory or audio file)
	 @return The list of audio files found in the path.
	 */
	QSharedPointer<QStringList> listFilesInDir(QSharedPointer<QDir> dir,
											   const bool& absolutePath = true);
	
	private slots:
	/** Input browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonInputBrowse_clicked();
	
	/** Output browse button callback.
	 Opens a file dialog when the output browse button is pressed
	 */
	void on_ButtonOutputBrowse_clicked();
	
	/** Compute button callback, starts audio processing.
	 Start processing the input data on the maximum number of threads available.
	 On each opened thread an instance of AudioProcess runs.
	 */
	void on_ButtonCompute_clicked();
	
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
	void on_SpinRecLength_valueChanged(int val) {Application::setParameter(ParRecLength, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinMARad_valueChanged(int val) {Application::setParameter(ParMovAvgRadius, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinLowFreq_valueChanged(double val) {Application::setParameter(ParLowpassFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinMinFreq_valueChanged(double val) {Application::setParameter(ParMinFilterFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinMaxFreq_valueChanged(double val) {Application::setParameter(ParMaxFilterFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinMASpecRad_valueChanged(int val) {Application::setParameter(ParMovAvgSpecRad, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinEstBackMARad_valueChanged(int val) {Application::setParameter(ParEstBackAveRadius, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinEstBackMinRad_valueChanged(int val) {Application::setParameter(ParEstBackMinRadius, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinStartFreq_valueChanged(double val) {Application::setParameter(ParIntervalStartFreq, val);}
	
	/** Slot to respond to parameters change */
	void on_SpinFreqIntWidth_valueChanged(double val) {Application::setParameter(ParIntervalWidthFreq, val);}
	
	/** Send to charts the command to display the next record */
	void on_ButtonNextRecord_clicked();
	
	/** Send to charts the command to display the previous record */
	void on_ButtonPreviousRecord_clicked();
	
signals:
	/** Signal emitted when a file path has been selected */
	void readyToDisplay(QSharedPointer<QDir> dir);
};

#endif // audioRecMainWin_h
