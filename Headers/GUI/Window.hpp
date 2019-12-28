#ifndef CAVA_GUI_h
#define CAVA_GUI_h

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
#include <UMF/ComputeHistogram.hpp>
#include <UMF/Evaluate1D.hpp>
#include <UMF/CurveNormalization.hpp>
#include <AA/ComputeProbability.hpp>
#include <AA/FeaturesDistance.hpp>
#include <GUI/DatabaseChart.hpp>
#include <GUI/ScanDirectory.hpp>
#include <GUI/savewindow.hpp>

#ifndef popupErrorWindow
#define popupErrorWindow(text) {popupError(text);return;}
#endif

#ifndef addEnumSetting
#define addEnumSetting(_settings_, _class_, _key_, _value_)									\
{																							\
	QString Key = _key_;																	\
	auto& SMO = _class_::staticMetaObject;													\
	auto Value = QVariant::fromValue(_class_::_value_);										\
	auto EnumName = QString(Value.typeName()).split("::").takeLast();						\
	auto Enum = SMO.enumerator(SMO.indexOfEnumerator(EnumName.toStdString().c_str()));		\
	_settings_.setValue(Key, Value.toLongLong());											\
	_settings_.beginGroup(Key+"Enum");														\
	for(int k = 0; k < Enum.keyCount(); ++k){												\
		_settings_.setValue(Enum.key(k), Enum.keyToValue(Enum.key(k)));						\
	}																						\
	_settings_.endGroup();																	\
}
#endif

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
	QList<QList<QString>> foundFiles;
	
	QAlgorithm::PropertyMap getPropsInGroup(const QString& group);
	
	void setupSettingsTab();
	
	void resizeEvent(QResizeEvent *event);
	
	private Q_SLOTS:
	
	/** Display an error.
	 Displays an error as a popup, with optional description, without quitting the graphical interface.
	 */
	Q_SLOT void popupError(QString errorDescription);
	
	// DB Creation tab callbacks *********************************
	
	/** Input audio files browse button callback.
	 Opens a file dialog where the user can choose the folder.
	 */
	Q_SLOT void on_DBFilesBrowseButton_clicked();
	
	/** Scan the database directory and store the result. */
	Q_SLOT void on_DBFilesLineEdit_textChanged(const QString &text);
	
	/** Create database button callback.
	 Computes the distances between every pair of feature vectors.
	 */
	Q_SLOT void on_DBCreateButton_clicked();
		
	/** Save the database to file. */
	Q_SLOT void on_DBSaveButton_clicked();
	
	/** Load the database to file. */
	Q_SLOT void on_DBLoadButton_clicked();
	
	/** Clear the DB graphic view. */
	Q_SLOT void on_PlotCtrlCleanButton_clicked();
	
	// Matching tab callbacks *********************************
	
	/** Matching tab - plot reset callback. */
	Q_SLOT void on_MCtrlResetButton_clicked();
	
	/** Matching tab - start matching callback. */
	Q_SLOT void on_MCtrlMatchButton_clicked();
	
	/** Matching tab - suspects folder browse button callback. */
	Q_SLOT void on_MUBrowseButton_clicked();
	
	Q_SLOT void on_TabWidget_currentChanged(int index);
	
	Q_SLOT void on_MLoadDatabaseButton_clicked();
	
	// File Inspectors tab callbacks *********************************
	
	/** Send to charts the command to display the next record */
	Q_SLOT void on_ButtonNextRecord_clicked();
	
	/** Send to charts the command to display the previous record */
	Q_SLOT void on_ButtonPreviousRecord_clicked();
	
	/** Respond to the browse button and open a file dialog */
	Q_SLOT void on_FIBrowseButton_clicked();
	
	/** Scan the database directory and store the result. */
	Q_SLOT void on_FILineEdit_textChanged(const QString& selectedFile);
	
	/** Handle log scale checkbox. */
	Q_SLOT void on_CheckLogScale_stateChanged(int state);
	
	// Settings Tab callbacks *********************************
	
	/** Reset parameters to their default values. */
	Q_SLOT void on_ResetParametersButton_clicked();
};

#endif // CAVA_GUI_h
