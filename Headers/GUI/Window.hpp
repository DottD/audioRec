#ifndef CAVA_GUI_h
#define CAVA_GUI_h

#define QT_DEBUG_PLUGINS 1

// Qt Library
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
#include <QVector>
// Other libraries
#include <ui_Window.h>
#include <QAlgorithm.hpp>
#include <QCommuteName.hpp>
#include <QAReleaseInput.hpp>
#include <UMF/ComputeHistogram.hpp>
#include <UMF/Fitting1D.hpp>
#include <UMF/Evaluate1D.hpp>
#include <UMF/ChiSquareTest.hpp>
#include <AA/FeaturesDistance.hpp>
#include <GUI/AudioReadExtract.hpp>
#include <GUI/DatabaseChart.hpp>
#include <GUI/ScanDirectory.hpp>

namespace GUI {
	class Window;
}

/** Main widget that manages user interaction with the associated GUI. */
class GUI::Window : public QWidget
{
	Q_OBJECT
	
public:
	explicit Window(QWidget *parent = 0); /**< Sets up the main window */
	~Window();
	
private:
	Ui::WindowUI* ui; /**< Pointer to the user interface class */
	const QStringList supportedAudioFormats = {"*.wav"}; /**< List of supported audio file formats */
	int processed, /**< Number of audio files processed */
	fileCount; /**< Total number of audio files to be processed */
	QSharedPointer<UMF::FittingGaussExp> extraFitting, intraFitting;
	
	private Q_SLOTS:
	
	/** Display an error.
	 Displays an error as a popup, with optional description, without quitting the graphical interface.
	 */
	Q_SLOT void popupError(QString errorDescription = "");
	
	/** Handle log scale checkbox. */
	Q_SLOT void on_CheckLogScale_stateChanged(int state);
	
	// File Inspectors tab callbacks *********************************
	
	/** Combo box for file choice callback.
	 This callback is executed whenever the user click on a choice in the combo box.
	 This function executes a thread to load the selected audio file, and then make the first record show.
	 */
	Q_SLOT void on_ComboChooseFile_activated(QString);
	
	/** Send to charts the command to display the next record */
	Q_SLOT void on_ButtonNextRecord_clicked();
	
	/** Send to charts the command to display the previous record */
	Q_SLOT void on_ButtonPreviousRecord_clicked();
	
	// Parameters tab callbacks *********************************
	
	/** Save the parameter with maximum frequency specification. */
	Q_SLOT void on_ButtonGroupMaxFreq_buttonClicked(QAbstractButton*);
	
	/** Save the parameter with minimum frequency specification. */
	Q_SLOT void on_ButtonGroupMinFreq_buttonClicked(QAbstractButton*);
	
	/** Save the parameter with minimum frequency specification. */
	Q_SLOT void on_ButtonGroupOversampling_buttonClicked(QAbstractButton*);
	
	// DB Creation tab callbacks *********************************
	
	/** Input audio files browse button callback.
	 Opens a file dialog where the user can choose the folder.
	 */
	Q_SLOT void on_DBFilesBrowseButton_clicked();
	
	/** Callback that resets the view to its original zoom and scroll. */
	Q_SLOT void on_PlotCtrlResetViewButton_clicked();
	
	/** Create database button callback.
	 Computes the distances between every pair of feature vectors.
	 */
	Q_SLOT void on_DBCreateButton_clicked();
	
	/** Instructs the progress bar to increase its value. */
	Q_SLOT void on_DBProgressBarStepUp();
	
	/** Save the database to file. */
	Q_SLOT void on_DBCreated();
	
	/** Clear the DB graphic view. */
	Q_SLOT void on_PlotCtrlCleanButton_clicked();
	
	// Matching tab callbacks *********************************
	
	/** Matching tab - plot reset callback. */
	Q_SLOT void on_MCtrlResetButton_clicked();
	
	/** Matching tab - start matching callback. */
	Q_SLOT void on_MCtrlMatchButton_clicked();
	
	/** Instructs the progress bar to increase its value. */
	Q_SLOT void on_MatchingProgressBarStepUp();
	
	/** Matching tab - handle the newly computed score. */
	Q_SLOT void on_newResult();
	
	/** Matching tab - input DB browse button callback. */
	Q_SLOT void on_MDBBrowseButton_clicked();
	
	/** Matching tab - culprit audio browse button callback. */
	Q_SLOT void on_MKBrowseButton_clicked();
	
	/** Matching tab - suspects folder browse button callback. */
	Q_SLOT void on_MUBrowseButton_clicked();
	
Q_SIGNALS:
	/** Signal emitted when a file path has been selected */
	Q_SIGNAL void readyToDisplay(QString dir);
};

#endif // CAVA_GUI_h
