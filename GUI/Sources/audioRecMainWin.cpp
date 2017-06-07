#include <Headers/audioRecMainWin.hpp>

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
	ui->LineEditInputDB->setDragEnabled(true);
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
	// Set initial record length
	{
		int recLength = Parameters::getParameter(Parameter::ParRecLength).toInt();
		QListIterator<QAbstractButton*> button(ui->ButtonGroupRecLength->buttons());
		while (button.hasNext()) {
			if (QLocale().toString(recLength) == button.peekNext()->text())
				button.next()->setChecked(true);
			else button.next()->setChecked(false);
		}
	}
	// Set initial tail suppression slider value
	{
		double tailsupp = Parameters::getParameter(Parameter::ParTailSuppression).toDouble();
		QSlider* slider = ui->SliderParTailSuppression;
		slider->setValue( slider->minimum() + int(tailsupp * double(slider->maximum()-slider->minimum())) );
	}
	// Set initial bin width
	{
		int binWidth = Parameters::getParameter(Parameter::ParBinWidth).toInt();
		QListIterator<QAbstractButton*> button(ui->ButtonGroupBinWidth->buttons());
		while (button.hasNext()) {
			if (QLocale().toString(binWidth) == button.peekNext()->text())
				button.next()->setChecked(true);
			else button.next()->setChecked(false);
		}
	}
	// Set initial tail suppression slider value
	{
		double peaksrel = Parameters::getParameter(Parameter::ParPeaksRelevance).toDouble();
		QSlider* slider = ui->SliderParPeaksRelevance;
		slider->setValue( slider->minimum() + int(peaksrel * double(slider->maximum()-slider->minimum())) );
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

void Ui::AudioRecMainWin::on_ButtonInputBrowseDB_clicked(){
	// Create and execute a file browser process
	QFileDialog fileBrowser(this);
	fileBrowser.setWindowTitle("Select a file or a folder");
	fileBrowser.setFileMode(QFileDialog::FileMode::AnyFile);
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Accept, "Load");
	fileBrowser.setLabelText(QFileDialog::DialogLabel::Reject, "Cancel");
	fileBrowser.setFilter(QDir::Filter::NoDotAndDotDot|QDir::Filter::Files|QDir::Filter::Dirs);
	fileBrowser.setNameFilters({"*.feat"});
	if (fileBrowser.exec()){
		// Check if there is at least a selected file
		if (fileBrowser.selectedFiles().isEmpty()) throw std::runtime_error("No selected files or folders");
		// Get the selected file path
		QString fileName = fileBrowser.selectedFiles().first(); // absolute path
		// Share path with the line edit
		ui->LineEditInputDB->setText(fileName);
	}
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

void Ui::AudioRecMainWin::on_ButtonCompute_clicked(){
	// Roughly check input and output
	if (ui->LineEditInput->text().isEmpty()) {ui->LineEditInput->setPlaceholderText("An input file or folder must be provided!");return;}
	if (ui->LineEditOutput->text().isEmpty()) {ui->LineEditOutput->setPlaceholderText("An output file or folder must be provided!");return;}
	// Reset counters
	fileCount = 0;
	processed = 0;
	// Extract the list of files (possibly one) to process
	QSharedPointer<QDir> inputPath(new QDir(ui->LineEditInput->text()));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(inputPath, true, supportedAudioFormats);
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

void Ui::AudioRecMainWin::on_ButtonCreateDatabase_clicked(){
	// Check if it input is empty
	if (ui->LineEditInputDB->text().isEmpty()) return;
	// Get the list of files in the input directory
	QSharedPointer<QDir> dir(new QDir(ui->LineEditInputDB->text()));
	QSharedPointer<QStringList> fileNames = listFilesInDir(dir, true, {"*.feat"});
	// Load each file and compare with any other (apart from it)
	QFile out_file, inn_file;
	VoiceFeatures out_feat, inn_feat;
	arma::vec distances(fileNames->size() * (fileNames->size()-1));
	unsigned int k = 0;
	for (const QString& out_name: *fileNames){
		// Read current features vector from file
		out_file.setFileName(out_name);
		out_feat.readFromFile(out_file);
		// Loop over the same list of files, looking for string different from "name"
		for (const QString& inn_name: *fileNames){
			// Check if the inner name is the same of the outer
			if (out_name == inn_name) continue;
			// Load the inner feature vector from file
			inn_file.setFileName(inn_name);
			inn_feat.readFromFile(inn_file);
			// Compute the distance between the vectors
			distances(k++) = out_feat.distance(inn_feat);
		}
	}
	// Compute the histogram of the distances
	// Div by 2 due to the repetition of feature vectors pairs
	double mindist = distances.min();
	double maxdist = distances.max();
	arma::vec barpos = arma::regspace(mindist, 0.05, maxdist);
	arma::uvec uhist = arma::hist(distances, barpos) / 2;
	arma::vec hist = arma::normalise(arma::conv_to<arma::vec>::from(uhist), 1);
	// Plot the histogram
	QLineSeries* qhist = new QLineSeries;
	for (unsigned int k = 0; k < hist.size(); k++) qhist->append(qreal(barpos(k)), qreal(hist(k)));
	QChart* chart = ui->BarSetDB->chart();
	chart->addSeries(qhist);
	chart->setTitle("Distribution");
	chart->setTheme(QChart::ChartThemeBlueCerulean);
	chart->setAnimationOptions(QChart::AnimationOption::AllAnimations);
	chart->legend()->setVisible(false);
	chart->createDefaultAxes();
}

void Ui::AudioRecMainWin::on_ButtonCleanPlot_clicked(){
	ui->BarSetDB->chart()->removeAllSeries();
}

void Ui::AudioRecMainWin::on_LineEditInput_textChanged(QString text){
	// Scan for audio files in the given path
	QSharedPointer<QDir> dir(new QDir(text));
	QSharedPointer<QStringList> audioFiles = listFilesInDir(dir, false /*return only file names*/, supportedAudioFormats);
	// List options in the combo box
	while(ui->ComboChooseFile->count() > 0) ui->ComboChooseFile->removeItem(0);
	ui->ComboChooseFile->addItems(*audioFiles);
}

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

void Ui::AudioRecMainWin::on_OneProcessEnded(QSharedPointer<QDir> dir){
	// Increase the processed counter
	processed++;
	// Extract file name without path and extension
	QString completion = "Done: " + dir->dirName() + " " + QString::number(processed) + "/" + QString::number(fileCount);
	// Show completion
	ui->LabelCompletion->setText(completion);
}

void Ui::AudioRecMainWin::popupError(QString errorDescription) {
	QMessageBox errorDispatcher;
	errorDispatcher.setWindowTitle("Error");
	errorDispatcher.setStandardButtons(QMessageBox::StandardButton::Ok);
	errorDispatcher.setDefaultButton(QMessageBox::StandardButton::Ok);
	errorDispatcher.setText(errorDescription);
	errorDispatcher.exec();
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

void Ui::AudioRecMainWin::on_ButtonGroupRecLength_buttonClicked(QAbstractButton* button){
	// Convert the name of the clicked button to the record length value
	Parameters::setParameter(Parameter::ParRecLength, QLocale().toInt(button->text()));
}

void Ui::AudioRecMainWin::on_SliderParTailSuppression_valueChanged(int value){
	// Save the parameter change
	double min = ui->SliderParTailSuppression->minimum();
	double max = ui->SliderParTailSuppression->maximum();
	Parameters::setParameter(Parameter::ParTailSuppression, (double(value)-min)/(max-min));
}

void Ui::AudioRecMainWin::on_ButtonGroupBinWidth_buttonClicked(QAbstractButton* button){
	// Convert the name of the clicked button to the record length value
	Parameters::setParameter(Parameter::ParBinWidth, QLocale().toInt(button->text()));
}

void Ui::AudioRecMainWin::on_SliderParPeaksRelevance_valueChanged(int value){
	// Save the parameter change
	double min = ui->SliderParPeaksRelevance->minimum();
	double max = ui->SliderParPeaksRelevance->maximum();
	Parameters::setParameter(Parameter::ParPeaksRelevance, (double(value)-min)/(max-min));
}
