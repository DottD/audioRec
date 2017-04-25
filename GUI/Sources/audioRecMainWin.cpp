#include <Headers/audioRecMainWin.hpp>

Ui::AudioRecMainWin::AudioRecMainWin(QWidget *parent) :
QWidget(parent),
ui(new Ui::mainWin),
processed(0),
fileCount(0) {
	// Initial setup - mandatory
	ui->setupUi(this);
	// Set the line edit as drag and drop receiver
	ui->LineEditInput->setDragEnabled(true);
	ui->LineEditOutput->setDragEnabled(true);
	// Set icons for record scolling buttons
	ui->ButtonPreviousRecord->setIcon(QIcon(":/48x48/Prev.png"));
	ui->ButtonNextRecord->setIcon(QIcon(":/48x48/Successive.png"));
}

Ui::AudioRecMainWin::~AudioRecMainWin(){
	delete ui;
}

void Ui::AudioRecMainWin::on_ButtonInputBrowse_clicked(){
	// Create and execute a file browser process
	QFileDialog fileBrowser(this);
	fileBrowser.setWindowTitle("Select a file or a folder");
	fileBrowser.setFileMode(QFileDialog::FileMode::AnyFile);
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Accept, "Load");
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Reject, "Cancel");
	fileBrowser.setFilter(QDir::Filter::NoDotAndDotDot|QDir::Filter::Files|QDir::Filter::Dirs);
	fileBrowser.setNameFilters({"*.wav"});
	if (fileBrowser.exec()){
		// Check if there is at least a selected file
		if (fileBrowser.selectedFiles().isEmpty()) throw std::runtime_error("No selected files or folders");
		// Get the selected file path
		QString fileName = fileBrowser.selectedFiles().first(); // absolute path
		// Share path with the line edit
		ui->LineEditInput->setText(fileName);
	}
}

void Ui::AudioRecMainWin::on_ButtonOutputBrowse_clicked(){
	/* Create and execute a file browser process */
	QFileDialog fileBrowser(this);
	fileBrowser.setWindowTitle("Select a folder");
	fileBrowser.setFileMode(QFileDialog::FileMode::Directory);
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Accept, "Choose");
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Reject, "Cancel");
	fileBrowser.setFilter(QDir::Filter::NoDotAndDotDot|QDir::Filter::Dirs);
	if (fileBrowser.exec()){
		// Check if there is at least a selected file
		if (fileBrowser.selectedFiles().isEmpty()) throw std::runtime_error("No selected folders");
		// Get the selected file path
		QString fileName = fileBrowser.selectedFiles().first(); // absolute path
		// Share path with the line edit
		ui->LineEditOutput->setText(fileName);
	}
}

QSharedPointer<QStringList> Ui::AudioRecMainWin::listFilesInDir(QSharedPointer<QDir> dir,
																const bool& absolutePath){
	QSharedPointer<QStringList> audioFiles(new QStringList());
	// Check if the path is a directory or a file
	if (dir->exists()) { // The input path is a folder
		// Append every found file to the output list of files
		audioFiles->append(dir->entryList(supportedAudioFormats));
		// Subsitute the absolute path to each file, if requested
		if (absolutePath) for(QString& file: *audioFiles) file = dir->absoluteFilePath(file);
	} else {
		// Get the name of the file
		QString fileName = dir->dirName();
		// Move up if the folder path
		if (!dir->cdUp()) throw std::runtime_error("Unable to cd upwards");
		// Check the existence of the file and append its absolute path
		if (dir->exists(fileName)) {
			if (absolutePath) audioFiles->append(dir->absoluteFilePath(fileName));
			else audioFiles->append(fileName);
		}
	}
	return audioFiles; // possibly empty
}

void Ui::AudioRecMainWin::on_ButtonCompute_clicked(){
	// Reset counters
	fileCount = 0;
	processed = 0;
	// Check whether an input has been provided
	if (ui->LineEditInput->text().isEmpty()) {popupError("Input not provided");return;}
	// Extract the list of files (possibly one) to process
	QSharedPointer<QDir> inputPath(new QDir(ui->LineEditInput->text()));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(inputPath);
	// Check if there are file names in the list
	if (audioFiles->isEmpty()) {popupError("Input is neither a folder nor a file");return;}
	// Store the number of files found
	fileCount = audioFiles->count();
	
	/* Create an instance of AudioProcess for each file name found
	 and associated it with the thread pool */
	for (const QString& fileName: *audioFiles){
		QSharedPointer<QDir> dir(new QDir(fileName));
		AudioProcess* process = new AudioProcess(); // deleted by QThreadPool when finished to work
		process->setFileName(dir);
		connect(process, SIGNAL(processEnded(QSharedPointer<QDir>)),
				this, SLOT(on_OneProcessEnded(QSharedPointer<QDir>)));
		QThreadPool::globalInstance()->start(process);
	}
}

void Ui::AudioRecMainWin::on_OneProcessEnded(QSharedPointer<QDir> dir){
	// Increase the processed counter
	processed++;
	// Extract file name without path and extension
	QString completion = "Done: " + dir->dirName() + " " + QString::number(processed) + "/" + QString::number(fileCount);
	// Show completion
	ui->LabelCompletion->setText(completion);
}

void Ui::AudioRecMainWin::popupError(const QString& errorDescription) {
	QMessageBox errorDispatcher;
	errorDispatcher.setWindowTitle("Error");
	errorDispatcher.setStandardButtons(QMessageBox::StandardButton::Ok);
	errorDispatcher.setDefaultButton(QMessageBox::StandardButton::Ok);
	errorDispatcher.setText(errorDescription);
	errorDispatcher.exec();
}
