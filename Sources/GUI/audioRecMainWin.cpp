#include <GUI/audioRecMainWin.hpp>

Ui::AudioRecMainWin::AudioRecMainWin(QWidget *parent) :
QWidget(parent),
ui(new Ui::mainWin),
processed(0),
fileCount(0) {
	qRegisterMetaType<QSharedPointer<QDir>>();
	// Initial setup - mandatory
	ui->setupUi(this);
	// Set the line edit as drag and drop receiver
	ui->LineEditInput->setDragEnabled(true);
	ui->LineEditOutput->setDragEnabled(true);
	ui->LineEditIntraDB->setDragEnabled(true);
	ui->LineEditExtraDB->setDragEnabled(true);
	// Set icons for record scrolling buttons
	ui->ButtonPreviousRecord->setIcon(QIcon(":/48x48/Prev.png"));
	ui->ButtonNextRecord->setIcon(QIcon(":/48x48/Successive.png"));
	// SEt icons for image scrolling buttons
	ui->ButtonPrevImage->setIcon(QIcon(":/48x48/Prev.png"));
	ui->ButtonNextImage->setIcon(QIcon(":/48x48/Successive.png"));
	// Move the record visualization widgets onto other threads
	connect(this, SIGNAL(readyToDisplay(QSharedPointer<QDir>)),
			ui->ChartShowRec, SLOT(display(QSharedPointer<QDir>)));
	connect(this, SIGNAL(readyToDisplay(QSharedPointer<QDir>)),
			ui->ChartShowRecSpectrum, SLOT(display(QSharedPointer<QDir>)));
	connect(ui->ChartShowRec, SIGNAL(raiseError(QString)),
			this, SLOT(popupError(QString)));
	connect(ui->ChartShowRecSpectrum, SIGNAL(raiseError(QString)),
			this, SLOT(popupError(QString)));
	connect(ui->LabelMatrixView, SIGNAL(changedImage(QString)),
			this, SLOT(on_changedImage(QString)));
	// Set initial parameters
	QString maxFreqText = QLocale().toString(Parameters::getParameter(Parameter::ParMaxFreq).toDouble()/1024);
	for(QAbstractButton* button : ui->ButtonGroupMaxFreq->buttons()){
		if (button->text() == maxFreqText) button->setChecked(true);
		else button->setChecked(false);
	}
	QString minFreqText = QLocale().toString(Parameters::getParameter(Parameter::ParMinFreq).toDouble()/64);
	for(QAbstractButton* button : ui->ButtonGroupMinFreq->buttons()){
		if (button->text() == minFreqText) button->setChecked(true);
		else button->setChecked(false);
	}
	QString oversamplingText = QLocale().toString(Parameters::getParameter(Parameter::ParOversampling).toInt()).prepend('x');
	for(QAbstractButton* button : ui->ButtonGroupOversampling->buttons()){
		if (button->text() == oversamplingText) button->setChecked(true);
		else button->setChecked(false);
	}
	// Set chart names
	ui->ChartShowRec->chart()->setTitle("Time domain");
	ui->ChartShowRecSpectrum->chart()->setTitle("Frequency Domain");
	// Set whether to use a logarithmic scale for the frequency plot
	if (ui->CheckLogScale->isChecked()){
		ui->ChartShowRecSpectrum->setLogScale();
	} else {
		ui->ChartShowRecSpectrum->setNaturalScale();
	}
	// Set record description
	ui->LabelRecordDescription->setText(QLocale().toString(ui->ChartShowRec->getRecordIndex()));
}

Ui::AudioRecMainWin::~AudioRecMainWin(){
	delete ui;
}

void Ui::AudioRecMainWin::popupError(QString errorDescription) {
	QMessageBox errorDispatcher;
	errorDispatcher.setWindowTitle("Error");
	errorDispatcher.setStandardButtons(QMessageBox::StandardButton::Ok);
	errorDispatcher.setDefaultButton(QMessageBox::StandardButton::Ok);
	errorDispatcher.setText(errorDescription);
	errorDispatcher.exec();
}

QSharedPointer<QStringList> Ui::AudioRecMainWin::listFilesInDir(QSharedPointer<QDir> dir,
																const bool& absolutePath,
																const QStringList& exts){
	QSharedPointer<QStringList> audioFiles(new QStringList());
	// Check if the path is a directory or a file
	if (dir->exists()) { // The input path is a folder
		// Append every found file to the output list of files
		audioFiles->append(dir->entryList(exts));
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

void Ui::AudioRecMainWin::on_CheckLogScale_stateChanged(int state){
	if (ui->CheckLogScale->isChecked()){
		ui->ChartShowRecSpectrum->setLogScale();
	} else {
		ui->ChartShowRecSpectrum->setNaturalScale();
	}
}

// Features Creation tab callbacks

void Ui::AudioRecMainWin::on_ButtonInputBrowse_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder with audio files");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditInput->setText(folderName);
}

void Ui::AudioRecMainWin::on_LineEditInput_textChanged(QString text){
	// Scan for audio files in the given path
	QSharedPointer<QDir> dir(new QDir(text));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(dir, false /*return only file names*/, supportedAudioFormats);
	// List options in the combo box
	while(ui->ComboChooseFile->count() > 0) ui->ComboChooseFile->removeItem(0);
	ui->ComboChooseFile->addItems(*audioFiles);
}

void Ui::AudioRecMainWin::on_ButtonOutputBrowse_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder with audio files");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditOutput->setText(folderName);
}

void Ui::AudioRecMainWin::on_ButtonCompute_clicked(){
	// Roughly check input and output
	if (ui->LineEditInput->text().isEmpty()) {ui->LineEditInput->setPlaceholderText("An input file or folder must be provided!");return;}
	if (ui->LineEditOutput->text().isEmpty()) {ui->LineEditOutput->setPlaceholderText("An output file or folder must be provided!");return;}
	// Reset counters
	fileCount = 0;
	processed = 0;
	ui->LabelMatrixView->reset();
	// Extract the list of files (possibly one) to process
	QSharedPointer<QDir> inputPath(new QDir(ui->LineEditInput->text()));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(inputPath, true, supportedAudioFormats);
	// Check if there are file names in the list
	if (audioFiles->isEmpty()) {popupError("Input folder is not suitable");return;}
	// Store the number of files found
	fileCount = audioFiles->count();
	
	/* Create an instance of AudioProcess for each file name found
	 and associated it with the thread pool */
	for (const QString& fileName: *audioFiles){
		QSharedPointer<QDir> dir(new QDir(fileName));
		AudioProcess* process = new AudioProcess(); // deleted by QThreadPool when finished to work
		process->setFileName(dir);
		process->setCaller(this);
		connect(process, SIGNAL(processEnded(QSharedPointer<QDir>)),
				this, SLOT(on_OneProcessEnded(QSharedPointer<QDir>)));
		connect(process, SIGNAL(imageGenerated(QSharedPointer<QDir>, QSharedPointer<QImage>)),
				this, SLOT(on_imageGenerated(QSharedPointer<QDir>, QSharedPointer<QImage>)));
		connect(process, SIGNAL(newFeatures(QSharedPointer<QDir>, QSharedPointer<QVector<QVector<double>>>)),
				this, SLOT(on_newFeatures(QSharedPointer<QDir>, QSharedPointer<QVector<QVector<double>>>)));
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

void Ui::AudioRecMainWin::on_imageGenerated(QSharedPointer<QDir> inputDir, QSharedPointer<QImage> image){
	// Save the binning matrix to image file
	QScopedPointer<QDir> outputDir(new QDir);
	if (!ui->LineEditOutput->text().isEmpty()){
		outputDir->setPath(ui->LineEditOutput->text());
	}
	QScopedPointer<QString> baseName(new QString(inputDir->dirName().split(".").takeFirst().append(".png")));
	// Create the output file path
	QDir outputName(outputDir->absoluteFilePath(*baseName));
	if (!image->save(outputName.absolutePath()))
		throw std::runtime_error((QString("Unable to save QImage as ")+outputName.absolutePath()).toStdString());
	
	ui->LabelMatrixView->append(outputName);
}

void Ui::AudioRecMainWin::on_changedImage(QString description){
	ui->LabelImageName->setText(description);
}

void Ui::AudioRecMainWin::on_ButtonNextImage_clicked(){
	ui->LabelMatrixView->next();
}

void Ui::AudioRecMainWin::on_ButtonPrevImage_clicked(){
	ui->LabelMatrixView->prev();
}

void Ui::AudioRecMainWin::on_newFeatures(QSharedPointer<QDir> dir,
										 QSharedPointer<QVector<QVector<double>>> features){
	// Create a VoiceFeatures instance with the provided feature vector
	VoiceFeatures vf(features);
	// Generate the QFile with output path of the file to be written
	QStringList temp = dir->dirName().split(".");
	temp.removeLast();
	QDir outdir(ui->LineEditOutput->text());
	QString path = outdir.absoluteFilePath(temp.join(".") + ".feat");
	QFile newFile(path);
	// Save the feature vector to file
	vf.writeToFile(newFile);
}

// File Inspectors tab callbacks

void Ui::AudioRecMainWin::on_ComboChooseFile_activated(QString text){
	// Retrieve all possible audio files paths
	QSharedPointer<QDir> dir(new QDir(text));
	QSharedPointer<QDir> inputDir(new QDir(ui->LineEditInput->text()));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(inputDir, true /*return file paths*/, supportedAudioFormats);
	// Search the current selected file among them
	QSharedPointer<QStringList> selectedFile(new QStringList(audioFiles->filter(text)));
	if (selectedFile->size() != 1) {popupError("Only one file should be returned"); return;}
	// Create a dir from that file and pass it to the widgets
	QSharedPointer<QDir> selectedDir(new QDir(selectedFile->first()));
	emit readyToDisplay(selectedDir);
}

void Ui::AudioRecMainWin::on_ButtonNextRecord_clicked(){
	quint64 idx1 = ui->ChartShowRec->stepUp();
	quint64 idx2 = ui->ChartShowRecSpectrum->stepUp();
	if (idx1 == idx2){
		ui->LabelRecordDescription->setText(QLocale().toString(idx1));
	}
}

void Ui::AudioRecMainWin::on_ButtonPreviousRecord_clicked(){
	quint64 idx1 = ui->ChartShowRec->stepDown();
	quint64 idx2 = ui->ChartShowRecSpectrum->stepDown();
	if (idx1 == idx2){
		ui->LabelRecordDescription->setText(QLocale().toString(idx1));
	}
}

// Parameters tab callbacks

void Ui::AudioRecMainWin::on_ButtonGroupMaxFreq_buttonClicked(QAbstractButton* button){
	// Get the maximum frequency value (input is expressed in kHz)
	double maxFreq = QLocale().toDouble(button->text()) * 1024;
	Parameters::setParameter(Parameter::ParMaxFreq, maxFreq);
}

void Ui::AudioRecMainWin::on_ButtonGroupMinFreq_buttonClicked(QAbstractButton* button){
	// Get the minimum frequency value
	double minFreq = QLocale().toDouble(button->text()) * 64;
	Parameters::setParameter(Parameter::ParMinFreq, minFreq);
}

void Ui::AudioRecMainWin::on_ButtonGroupOversampling_buttonClicked(QAbstractButton* button){
	// Get the oversampling value and set the parameter
	int oversampling = QLocale().toInt(button->text().remove('x'));
	Parameters::setParameter(Parameter::ParOversampling, oversampling);
}

// DB Creation tab callbacks

void Ui::AudioRecMainWin::on_ButtonIntraBrowseDB_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder with features");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditIntraDB->setText(folderName);
}

void Ui::AudioRecMainWin::on_ButtonExtraBrowseDB_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder with features");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditExtraDB->setText(folderName);
}

void Ui::AudioRecMainWin::on_ButtonResetView_clicked(){
	ui->BarSetDB->chart()->zoomReset();
}

void Ui::AudioRecMainWin::on_ButtonCreateDatabase_clicked(){
	// Check if it input is empty
	if (ui->LineEditIntraDB->text().isEmpty() or ui->LineEditExtraDB->text().isEmpty()) return;
	// Get the list of files in the input directory
	QSharedPointer<QDir> dirIntra(new QDir(ui->LineEditIntraDB->text()));
	QSharedPointer<QDir> dirExtra(new QDir(ui->LineEditExtraDB->text()));
	QSharedPointer<QStringList> fileNamesIntra = listFilesInDir(dirIntra, true, {"*.feat"});
	QSharedPointer<QStringList> fileNamesExtra = listFilesInDir(dirExtra, true, {"*.feat"});
	// Create the database from features file names (separate thread)
	DBInterpreter* db = new DBInterpreter;
	db->setIntraFeats(fileNamesIntra);
	db->setExtraFeats(fileNamesExtra);
	QThread* dbThread = new QThread(this);
	db->moveToThread(dbThread);
	connect(dbThread, SIGNAL(started()),
			db, SLOT(computeDB()));
	connect(dbThread, SIGNAL(finished()),
			db, SLOT(deleteLater()));
	connect(db, SIGNAL(histogramReady(QVector<double>, QVector<double>)),
			ui->BarSetDB, SLOT(plotHistogram(QVector<double>, QVector<double>)));
	connect(db, SIGNAL(fittingReady(QVector<double>, QVector<double>)),
			ui->BarSetDB, SLOT(plotGaussianCurve(QVector<double>, QVector<double>)));
	connect(db, SIGNAL(databaseCreated()),
			this, SLOT(on_DBCreated()));
	ui->BarSetDB->setFitOverlay(false);
	dbThread->start();
}

void Ui::AudioRecMainWin::on_DBCreated(){
	// Create and execute a file browser process
	QString fileName = QFileDialog::getSaveFileName(this,
													"Select the database file name");
	// If the user press cancel, don't save anything
	if (fileName.isEmpty()) return;
	// Insert the proper extension
	QString ext = "adb";
	if(fileName.split(".").last() != ext) fileName.append(QString(".")+ext);
	// Save the database to that file
	DBInterpreter* db = dynamic_cast<DBInterpreter*>(QObject::sender());
	QFile file(fileName);
	db->writeDBToFile(file);
}

void Ui::AudioRecMainWin::on_ButtonCleanPlot_clicked(){
	ui->BarSetDB->chart()->removeAllSeries();
}

// Matching tab callbacks

void Ui::AudioRecMainWin::on_ButtonResetMatching_clicked(){
	// Reset chart
	ui->BarPlotSuspects->chart()->removeAllSeries();
	ui->BarPlotSuspects->chart()->zoomReset();
	// Reset results tab
	ui->TableMatchingResults->setText("Distribution Similarity Estimation");
}

void Ui::AudioRecMainWin::on_ButtonTestIntra_clicked(){
	startMatching(true);
}

void Ui::AudioRecMainWin::on_ButtonTestExtra_clicked(){
	startMatching(false);
}

void Ui::AudioRecMainWin::startMatching(bool testIntra){
	// Roughly check input and output
	if (ui->LineEditDB->text().isEmpty()) {ui->LineEditDB->setPlaceholderText("An input file must be provided!");return;}
	if (ui->LineEditCulprit->text().isEmpty()) {ui->LineEditCulprit->setPlaceholderText("An input file must be provided!");return;}
	if (ui->LineEditSuspects->text().isEmpty()) {ui->LineEditSuspects->setPlaceholderText("An input file must be provided!");return;}
	// Read the database
	DBInterpreter db;
	QFile dbFile(ui->LineEditDB->text());
	db.readDBFromFile(dbFile);
	// Show the curves in the chart
	QVector<double> x;
	for(double d = 0.0; d < 5; d += 0.1) x << d;
	ui->BarPlotSuspects->plotGaussianCurve(db.getIntraCoeffs(), x);
	ui->BarPlotSuspects->plotGaussianCurve(db.getExtraCoeffs(), x);
	// Start the matching on another thread
	QThread* matchingThread = new QThread(this);
	Matching* matching = new Matching;
	matching->setTestIntra(testIntra);
	matching->setExtraCoeffs(db.getExtraCoeffs());
	matching->setIntraCoeffs(db.getIntraCoeffs());
	QSharedPointer<QDir> culpritDir(new QDir(ui->LineEditCulprit->text()));
	matching->setCulpritFiles(*listFilesInDir(culpritDir, true, supportedAudioFormats));
	QSharedPointer<QDir> suspectsDir(new QDir(ui->LineEditSuspects->text()));
	matching->setSuspectsFiles(*listFilesInDir(suspectsDir, true, supportedAudioFormats));
	matching->moveToThread(matchingThread);
	connect(matchingThread, SIGNAL(started()),
			matching, SLOT(match()));
	connect(matchingThread, SIGNAL(finished()),
			matching, SLOT(deleteLater()));
	connect(matching, SIGNAL(newScore(QList<QVariant>)),
			this, SLOT(on_newScore(QList<QVariant>)));
	connect(matching, SIGNAL(newDistance(double)),
			ui->BarPlotSuspects, SLOT(addSpot(double)));
	connect(matching, SIGNAL(fittingReady(QVector<double>, QVector<double>)),
			ui->BarPlotSuspects, SLOT(plotGaussianCurve(QVector<double>, QVector<double>)));
	matchingThread->start();
}

void Ui::AudioRecMainWin::on_newScore(QList<QVariant> score){
	QString line = score.takeFirst().toString() + "   " + QLocale().toString(score.takeFirst().toDouble());
	QString all = ui->TableMatchingResults->text() + "<br>" + line;
	ui->TableMatchingResults->setText(all);
}

void Ui::AudioRecMainWin::on_ButtonBrowseDB_clicked(){
	// Create and execute a file browser process
	QString fileName = QFileDialog::getOpenFileName(this, "Select DB file", QString(), "Audio Database file (*.adb)");
	// Check if the user pressed cancel
	if (fileName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditDB->setText(fileName);
}

void Ui::AudioRecMainWin::on_ButtonBrowseCulprit_clicked(){
	// Create string with supported audio files
	QString supportedAudio = "Audio file (" + supportedAudioFormats.join(" ") + ")";
	// Create and execute a file browser process
	QString fileName = QFileDialog::getOpenFileName(this, "Select culprit audio file", QString(), supportedAudio);
	// Check if the user pressed cancel
	if (fileName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditCulprit->setText(fileName);
}

void Ui::AudioRecMainWin::on_ButtonBrowseSuspects_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select suspects folder");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->LineEditSuspects->setText(folderName);
}

