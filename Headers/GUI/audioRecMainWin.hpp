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
#include <AA/AudioProcess.hpp>
#include <AA/VoiceFeatures.hpp>
#include <AA/DBInterpreter.hpp>
#include <GUI/DatabaseChart.hpp>
#include <AA/Matching.hpp>

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
	
	/** Display an error.
	 Displays an error as a popup, with optional description, without quitting the graphical interface.
	 */
	void popupError(QString errorDescription = "");
	
	/** Handle log scale checkbox. */
	void on_CheckLogScale_stateChanged(int state);
	
	// Features Creation tab callbacks *********************************
	
	/** Input browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonInputBrowse_clicked();
	
	/** Input line edit callback.
	 This callback executes whenever the text in the widget changes. It automatically updates the
	 combo box with the list of audio files found.
	 */
	void on_LineEditInput_textChanged(QString);
	
	/** Output browse button callback.
	 Opens a file dialog when the output browse button is pressed
	 */
	void on_ButtonOutputBrowse_clicked();
	
	/** Compute button callback, starts audio processing.
	 Start processing the input data on the maximum number of threads available.
	 On each opened thread an instance of AudioProcess runs.
	 */
	void on_ButtonCompute_clicked();
	
	/** Handle the processEnded signal emitted by AudioProcess.
	 This function updates the completion label every time an instance of AudioProcess finishes running.
	 */
	void on_OneProcessEnded(QSharedPointer<QDir>);
	
	/** Draw the given image in the window and save it in the output folder. */
	void on_imageGenerated(QSharedPointer<QDir> inputDir, QSharedPointer<QImage> image);
	
	/** Change image description. */
	void on_changedImage(QString);
	
	/** Show next image in list. */
	void on_ButtonNextImage_clicked();
	
	/** Show previous image in list. */
	void on_ButtonPrevImage_clicked();
	
	/** Save the feature vector to file.
	 Activated when a new feature vector is computed.
	 */
	void on_newFeatures(QSharedPointer<QDir> dir,
						QSharedPointer<QVector<QVector<double>>> features);
	
	// File Inspectors tab callbacks *********************************
	
	/** Combo box for file choice callback.
	 This callback is executed whenever the user click on a choice in the combo box.
	 This function executes a thread to load the selected audio file, and then make the first record show.
	 */
	void on_ComboChooseFile_activated(QString);
	
	/** Send to charts the command to display the next record */
	void on_ButtonNextRecord_clicked();
	
	/** Send to charts the command to display the previous record */
	void on_ButtonPreviousRecord_clicked();
	
	// Parameters tab callbacks *********************************
	
	/** Save the parameter with maximum frequency specification. */
	void on_ButtonGroupMaxFreq_buttonClicked(QAbstractButton*);
	
	/** Save the parameter with minimum frequency specification. */
	void on_ButtonGroupMinFreq_buttonClicked(QAbstractButton*);
	
	/** Save the parameter with minimum frequency specification. */
	void on_ButtonGroupOversampling_buttonClicked(QAbstractButton*);
	
	// DB Creation tab callbacks *********************************
	
	/** Input intra-speaker database browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonIntraBrowseDB_clicked();
	
	/** Input extra-speaker database browse button callback.
	 Opens a file dialog when the input browse button is pressed.
	 */
	void on_ButtonExtraBrowseDB_clicked();
	
	/** Callback that resets the view to its original zoom and scroll. */
	void on_ButtonResetView_clicked();
	
	/** Create database button callback.
	 Computes the distances between every pair of feature vectors.
	 */
	void on_ButtonCreateDatabase_clicked();
	
	/** Save the database to file. */
	void on_DBCreated();
	
	/** Clear the DB graphic view. */
	void on_ButtonCleanPlot_clicked();
	
	// Matching tab callbacks *********************************
	
	/** Matching tab - plot reset callback. */
	void on_ButtonResetMatching_clicked();
	
	/** Matching tab - start matching callback (intra). */
	void on_ButtonTestIntra_clicked();
	
	/** Matching tab - start matching callback (extra). */
	void on_ButtonTestExtra_clicked();
	
	/** Matching tab - handle the newly computed score. */
	void on_newScore(QList<QVariant>);
	
	/** Matching tab - input DB browse button callback. */
	void on_ButtonBrowseDB_clicked();
	
	/** Matching tab - culprit audio browse button callback. */
	void on_ButtonBrowseCulprit_clicked();
	
	/** Matching tab - suspects folder browse button callback. */
	void on_ButtonBrowseSuspects_clicked();
	
	/** Matching tab - matching with bool parameter. */
	void startMatching(bool testIntra);
	
signals:
	/** Signal emitted when a file path has been selected */
	void readyToDisplay(QSharedPointer<QDir> dir);
};

#endif // audioRecMainWin_h
