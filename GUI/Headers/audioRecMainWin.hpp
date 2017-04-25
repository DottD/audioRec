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
	
	/** Display an error.
	 Displays an error as a popup, with optional description, without quitting the graphical interface.
	 */
	void popupError(const QString& errorDescription = "");
	
	private slots:
	/** Handle the processEnded signal emitted by AudioProcess.
	 This function updates the completion label every time an instance of AudioProcess finishes running.
	 */
	void on_OneProcessEnded(QSharedPointer<QDir>);
};

#endif // audioRecMainWin_h
