#include <GUI/Window.hpp>

GUI::Window::Window(QWidget *parent) :
QWidget(parent),
ui(new Ui::WindowUI),
processed(0),
fileCount(0) {
	// Initial setup - mandatory
	ui->setupUi(this);
	// Set the line edit as drag and drop receiver
	ui->DBFilesLineEdit->setDragEnabled(true);
	ui->MDBLineEdit->setDragEnabled(true);
	ui->MULineEdit->setDragEnabled(true);
	ui->MKLineEdit->setDragEnabled(true);
	// Set icons for record scrolling buttons
	ui->ButtonPreviousRecord->setIcon(QIcon(":/48x48/Prev.png"));
	ui->ButtonNextRecord->setIcon(QIcon(":/48x48/Successive.png"));
	// Move the record visualization widgets onto other threads
	connect(this, SIGNAL(readyToDisplay(QString)),
			ui->ChartShowRec, SLOT(display(QString)));
	connect(this, SIGNAL(readyToDisplay(QString)),
			ui->ChartShowRecSpectrum, SLOT(display(QString)));
	connect(ui->ChartShowRec, SIGNAL(raiseError(QString)),
			this, SLOT(popupError(QString)));
	connect(ui->ChartShowRecSpectrum, SIGNAL(raiseError(QString)),
			this, SLOT(popupError(QString)));
	// Set initial parameters
	QString maxFreqText = QLocale().toString(QSettings().value("FE/MaxFreq").toDouble()/1024);
	for(QAbstractButton* button : ui->ButtonGroupMaxFreq->buttons()){
		if (button->text() == maxFreqText) button->setChecked(true);
		else button->setChecked(false);
	}
	QString minFreqText = QLocale().toString(QSettings().value("FE/MinFreq").toDouble()/64);
	for(QAbstractButton* button : ui->ButtonGroupMinFreq->buttons()){
		if (button->text() == minFreqText) button->setChecked(true);
		else button->setChecked(false);
	}
	QString oversamplingText = QLocale().toString(QSettings().value("FE/Oversampling").toInt()).prepend('x');
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
	// Set default parameters
	QSettings settings;
	settings.setValue("FE/MinFreq", double(4*64));
	settings.setValue("FE/MaxFreq", double(6*1024));
	settings.setValue("FE/Oversampling", int(1));
	settings.setValue("FE/BEMinFilterRad", int(32-1));
	settings.setValue("FE/BEMaxPeakWidth", double(0.1));
	settings.setValue("FE/BEDerivDiam", int(4));
	settings.setValue("FE/BEMaxIterations", int(10));
	settings.setValue("FE/BEMaxInconsistency", double(2));
	settings.setValue("FE/BEMaxDistNodes", int(3));
	settings.setValue("FE/ButtFilterTail", double(0.1));
	settings.setValue("FE/PeakRelevance", double(0.2));
	settings.setValue("FE/PeakMinVarInfluence", double(0.01));
	settings.setValue("FE/BinWidth", int(8));
	settings.setValue("FE/PeakHeightThreshold", double(0.01));
	settings.setValue("Histogram/BarStep", double(0.05));
	// Reset the progress bars
	ui->DBProgressBar->reset();
	ui->MatchingProgressBar->reset();
	qRegisterMetaType<QVector<double>>();
}

GUI::Window::~Window(){
	delete ui;
}

void GUI::Window::popupError(QString errorDescription) {
	QMessageBox::critical(this, "Error", errorDescription);
}

void GUI::Window::on_CheckLogScale_stateChanged(int state){
	if (ui->CheckLogScale->isChecked()){
		ui->ChartShowRecSpectrum->setLogScale();
	} else {
		ui->ChartShowRecSpectrum->setNaturalScale();
	}
}

// File Inspectors tab callbacks

void GUI::Window::on_ComboChooseFile_activated(QString text){
	// Retrieve all possible audio files paths
	QDir dir(text);
	QDir inputDir(ui->DBFilesLineEdit->text());
	auto scanner = ScanDirectory::create({
		{"Folder", ui->DBFilesLineEdit->text()},
		{"Extensions", supportedAudioFormats}
	});
	scanner->run();
	QStringList audioFiles;
	for(const QList<QString>& files: scanner->getOutContent()){
		audioFiles << files;
	}
	// Search the current selected file among them
	QStringList selectedFile(audioFiles.filter(text));
	if (selectedFile.size() != 1) popupErrorWindow("Only one file should be returned");
	// Pass the file name to the widgets
	emit readyToDisplay(selectedFile.first());
}

void GUI::Window::on_ButtonNextRecord_clicked(){
	quint64 idx1 = ui->ChartShowRec->stepUp();
	quint64 idx2 = ui->ChartShowRecSpectrum->stepUp();
	if (idx1 == idx2){
		ui->LabelRecordDescription->setText(QLocale().toString(idx1));
	}
}

void GUI::Window::on_ButtonPreviousRecord_clicked(){
	quint64 idx1 = ui->ChartShowRec->stepDown();
	quint64 idx2 = ui->ChartShowRecSpectrum->stepDown();
	if (idx1 == idx2){
		ui->LabelRecordDescription->setText(QLocale().toString(idx1));
	}
}

// Parameters tab callbacks

void GUI::Window::on_ButtonGroupMaxFreq_buttonClicked(QAbstractButton* button){
	// Get the maximum frequency value (input is expressed in kHz)
	double maxFreq = QLocale().toDouble(button->text()) * 1024;
	QSettings().setValue("FE/MaxFreq", maxFreq);
}

void GUI::Window::on_ButtonGroupMinFreq_buttonClicked(QAbstractButton* button){
	// Get the minimum frequency value
	double minFreq = QLocale().toDouble(button->text()) * 64;
	QSettings().setValue("FE/MinFreq", minFreq);
}

void GUI::Window::on_ButtonGroupOversampling_buttonClicked(QAbstractButton* button){
	// Get the oversampling value and set the parameter
	int oversampling = QLocale().toInt(button->text().remove('x'));
	QSettings().setValue("FE/Oversampling", oversampling);
}

// DB Creation tab callbacks

void GUI::Window::on_DBFilesBrowseButton_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder", QDir::currentPath());
	// Check if the user pressed cancel
	if (!folderName.isEmpty()) {
		// Share path with the line edit
		ui->DBFilesLineEdit->setText(folderName);
	}
}

void GUI::Window::on_PlotCtrlResetViewButton_clicked(){
	ui->DBChartView->chart()->zoomReset();
}

void GUI::Window::on_DBCreateButton_clicked(){
	// Disable button
	ui->DBCreateButton->setEnabled(false);
	// Check if it input is empty
	if (ui->DBFilesLineEdit->text().isEmpty()){
		ui->DBCreateButton->setEnabled(true);
		popupErrorWindow("An input folder must be chosen");
	}
	// Get the list of files in the input directory
	auto dirScanner = ScanDirectory::create({
		{"Extensions", supportedAudioFormats},
		{"Folder", ui->DBFilesLineEdit->text()},
		{"Recursive", true}
	});
	dirScanner->run();
	QList<QList<QString>> dirContent = dirScanner->getOutContent();
	if(dirContent.isEmpty()) {
		ui->DBCreateButton->setEnabled(true);
		popupErrorWindow("Empty directory");
	}
	// Set up features extraction parameters for later use
	QAlgorithm::PropertyMap FEPars = {
		{"KeepInput", false},
		{"SelectRecord", -1},
		{"MinFreq", QSettings().value("FE/MinFreq")},
		{"MaxFreq", QSettings().value("FE/MaxFreq")},
		{"Oversampling", QSettings().value("FE/Oversampling")},
		{"BEMinFilterRad", QSettings().value("FE/BEMinFilterRad")},
		{"BEMaxPeakWidth", QSettings().value("FE/BEMaxPeakWidth")},
		{"BEDerivDiam", QSettings().value("FE/BEDerivDiam")},
		{"BEMaxIterations", QSettings().value("FE/BEMaxIterations")},
		{"BEMaxInconsistency", QSettings().value("FE/BEMaxInconsistency")},
		{"BEMaxDistNodes", QSettings().value("FE/BEMaxDistNodes")},
		{"ButtFilterTail", QSettings().value("FE/ButtFilterTail")},
		{"PeakRelevance", QSettings().value("FE/PeakRelevance")},
		{"PeakMinVarInfluence", QSettings().value("FE/PeakMinVarInfluence")},
		{"BinWidth", QSettings().value("FE/BinWidth")},
		{"PeakHeightThreshold", QSettings().value("FE/PeakHeightThreshold")}
	};
	// Declare a list of extractors (this intermediate step will avoid reloading files and recomputing the features)
	// extractors will have the same structure of dirContent (1-1 correspondance)
	QList<QList<QSharedPointer<AA::FeaturesExtractor>>> extractors;
	for(const auto& dir: dirContent){// foreach subdir
		// Create a new sublist
		extractors << QList<QSharedPointer<AA::FeaturesExtractor>>();
		for(const auto& file: dir){ // foreach file
			// Set current file as input to an audio reader instance
			// Extract features after the reader has completed its task
			auto reader = AA::AudioReader::create({{"File", file}});
			auto extractor = AA::FeaturesExtractor::create(FEPars);
			reader >> extractor;
			// Add to extractors to the last sublist created
			extractors.last() << extractor;
		}
	}
	// Declare and initialize the total number of files to process
	int n_files = 0;
	// Intra-speaker features
	{
		// Compute the histogram of all the intra-speaker distances
		auto histCompute = UMF::ComputeHistogram::create({
			{"KeepInput", false},
			{"OITable", QVariant::fromValue(QMapStringString({{"Distance","Values"}})) },
			{"BarStep", QSettings().value("Histogram/BarStep")}
		}, this);
		// Define a map of already used pairs to avoid to compute the same distance twice
		QSet<QSet<QAlgorithm*>> used_pairs;
		for(auto& dir: extractors){
			for(auto& outerExtractor: dir){
				// Scan the files in the directory again and compute the distance
				// between each couple of features array
				for(auto& innerExtractor: dir){
					// Some checks:
					auto sameFile = (outerExtractor == innerExtractor); // outer-inner are the same file
					auto alreadyUsedPairs = used_pairs.contains({outerExtractor.data(), innerExtractor.data()});
					if(!sameFile && !alreadyUsedPairs){
						// Distance calculator
						auto distanceCalculator = AA::FeaturesDistance::create({ {"KeepInput", false} }, this);
						outerExtractor >> distanceCalculator;
						innerExtractor >> distanceCalculator >> histCompute;
						// Connect to progress bar
						connect(distanceCalculator.data(), SIGNAL(justFinished()), this, SLOT(on_DBProgressBarStepUp()));
						// Increment the files number
						n_files++;
						// Add this pair of extractors to used_pairs
						used_pairs << QSet<QAlgorithm*>({outerExtractor.data(), innerExtractor.data()});
					}
				}
			}
		}
		// When the histogram is computed draw the histogram
		connect(histCompute.data(), SIGNAL(justFinished()), ui->DBChartView, SLOT(plotHistogram()));
		// Change output names and perform fitting
		this->intraFitting = UMF::FittingGaussExp::create({
			{"OITable", QVariant::fromValue(QMapStringString({{"HistX","X"},{"HistY","Y"}})) }
		}, this);
		this->intraFitting->setObjectName("IntraFitting");
		histCompute >> this->intraFitting;
		// When the fitting algorithm finishes call on_DBCreated and plot the fitted curve
		connect(this->intraFitting.data(), SIGNAL(justFinished()), this, SLOT(on_DBCreated()));
		connect(this->intraFitting.data(), SIGNAL(justFinished()), ui->DBChartView, SLOT(plotGaussianCurve()));
	}
	// Extra-speaker features
	{
		// Compute the histogram of all the extra-speaker distances
		auto histCompute = UMF::ComputeHistogram::create({
			{"KeepInput", false},
			{"OITable", QVariant::fromValue(QMapStringString({{"Distance","Values"}})) },
			{"BarStep", QSettings().value("Histogram/BarStep")}
		}, this);
		// Define a map of already used pairs to avoid to compute the same distance twice
		QSet<QSet<QAlgorithm*>> used_pairs;
		for(auto& outerDir: extractors){
			for(auto& outerExtractor: outerDir){
				// Scan the files in the directory again and compute the distance
				// between each couple of features array
				for(auto& innerDir: extractors){
					// The intra-speaker distances are not to be considered here
					// so skip if the dictories are the same
					if(outerDir == innerDir) continue;
					for(auto& innerExtractor: innerDir){
						// Some checks:
						auto sameFile = (outerExtractor == innerExtractor); // outer-inner are the same file
						auto alreadyUsedPairs = used_pairs.contains({outerExtractor.data(), innerExtractor.data()});
						if(!sameFile && !alreadyUsedPairs){
							// Distance calculator
							auto distanceCalculator = AA::FeaturesDistance::create({ {"KeepInput", false} }, this);
							outerExtractor >> distanceCalculator;
							innerExtractor >> distanceCalculator >> histCompute;
							// Connect to progress bar
							connect(distanceCalculator.data(), SIGNAL(justFinished()), this, SLOT(on_DBProgressBarStepUp()));
							// Increment the files number
							n_files++;
							// Add this pair of extractors to used_pairs
							used_pairs << QSet<QAlgorithm*>({outerExtractor.data(), innerExtractor.data()});
						}
					}
				}
			}
		}
		// When the histogram is computed draw the histogram
		connect(histCompute.data(), SIGNAL(justFinished()), ui->DBChartView, SLOT(plotHistogram()));
		// Change output names and perform fitting
		this->extraFitting = UMF::FittingGaussExp::create({
			{"OITable", QVariant::fromValue(QMapStringString({{"HistX","X"},{"HistY","Y"}})) }
		}, this);
		this->extraFitting->setObjectName("ExtraFitting");
		histCompute >> this->extraFitting;
		// When the fitting algorithm finishes call on_DBCreated and plot the fitted curve
		connect(this->extraFitting.data(), SIGNAL(justFinished()), this, SLOT(on_DBCreated()));
		connect(this->extraFitting.data(), SIGNAL(justFinished()), ui->DBChartView, SLOT(plotGaussianCurve()));
	}
	// Finally set up the progress bar
	ui->DBProgressBar->setRange(0, n_files);
	ui->DBProgressBar->reset();
	// Start computation
	QThreadPool::globalInstance()->start(this->intraFitting.data());
	QThreadPool::globalInstance()->start(this->extraFitting.data());
}

void GUI::Window::on_DBProgressBarStepUp(){
	ui->DBProgressBar->setValue( ui->DBProgressBar->value()+1 );
	ui->DBProgressBar->update();
}

void GUI::Window::on_DBCreated(){
	ui->DBCreateButton->setEnabled(true);
	// Every to calls to this function (i.e. when both extra and intra features are computed)
	// get the outputs and save the database
	static QList<UMF::Fitting1D*> finishedAlgo;
	UMF::Fitting1D* sender = dynamic_cast<UMF::Fitting1D*>(QObject::sender());
	finishedAlgo << sender;
	if(finishedAlgo.size() == 2){
		// Create and execute a file browser process
		QString fileName = QFileDialog::getSaveFileName(this, "Select the database file name");
		// If the user press cancel, don't save anything
		if (!fileName.isEmpty()){
			// Insert the proper extension
			QString ext = "cdb";
			if(fileName.split(".").last() != ext) fileName.append(QString(".")+ext);
			// Save the database to that file
			QFile file(fileName);
			if(file.open(QFile::WriteOnly)){
				QDataStream ostream(&file);
				ostream << finishedAlgo.at(0)->objectName() << finishedAlgo.at(0)->getOutCoefficients();
				ostream << finishedAlgo.at(1)->objectName() << finishedAlgo.at(1)->getOutCoefficients();
				file.close();
			}
		}
		// Clear the algorithm list
		finishedAlgo.clear();
	}
}

void GUI::Window::on_PlotCtrlCleanButton_clicked(){
	ui->DBChartView->chart()->removeAllSeries();
}

// Matching tab callbacks

void GUI::Window::on_MCtrlResetButton_clicked(){
	// Reset chart
	ui->MatchingChartView->chart()->removeAllSeries();
	ui->MatchingChartView->chart()->zoomReset();
	// Reset results tab
	ui->MatchingResultsLabel->setText("Distribution Similarity Estimation");
}

void GUI::Window::on_MCtrlMatchButton_clicked(){
	// Check input and output
	if (ui->MDBLineEdit->text().isEmpty() ||
		ui->MKLineEdit->text().isEmpty() ||
		ui->MULineEdit->text().isEmpty()){
		popupErrorWindow("Check the input folders");
	}
	// Reset the result label
	ui->MatchingResultsLabel->setText("");
	// Load default parameters
	QAlgorithm::PropertyMap FEPars = {
		{"SelectRecord", -1},
		{"MinFreq", QSettings().value("FE/MinFreq")},
		{"MaxFreq", QSettings().value("FE/MaxFreq")},
		{"Oversampling", QSettings().value("FE/Oversampling")},
		{"BEMinFilterRad", QSettings().value("FE/BEMinFilterRad")},
		{"BEMaxPeakWidth", QSettings().value("FE/BEMaxPeakWidth")},
		{"BEDerivDiam", QSettings().value("FE/BEDerivDiam")},
		{"BEMaxIterations", QSettings().value("FE/BEMaxIterations")},
		{"BEMaxInconsistency", QSettings().value("FE/BEMaxInconsistency")},
		{"BEMaxDistNodes", QSettings().value("FE/BEMaxDistNodes")},
		{"ButtFilterTail", QSettings().value("FE/ButtFilterTail")},
		{"PeakRelevance", QSettings().value("FE/PeakRelevance")},
		{"PeakMinVarInfluence", QSettings().value("FE/PeakMinVarInfluence")},
		{"BinWidth", QSettings().value("FE/BinWidth")},
		{"PeakHeightThreshold", QSettings().value("FE/PeakHeightThreshold")}
	};
	// Compute the distances between files from the known voice and files from unknown ones
	auto MKScan = GUI::ScanDirectory::create({
		{"File", ui->MKLineEdit->text()},
		{"Recursive", false},
		{"Extensions", supportedAudioFormats}
	});
	MKScan->run();
	auto MUScan = GUI::ScanDirectory::create({
		{"File", ui->MULineEdit->text()},
		{"Recursive", false},
		{"Extensions", supportedAudioFormats}
	});
	MUScan->run();
	// Compute the histogram
	auto Histogram = UMF::ComputeHistogram::create({
		{"OITable", QVariant::fromValue(QMapStringString({{"Distance","Values"}})) },
		{"BarStep", QSettings().value("Histogram/BarStep")}
	});
	// Declare and initialize the total number of files
	int n_files = 0;
	for(const QString& MKFile: MKScan->getOutContent().first().first()){
		auto MKRead = AA::AudioReader::create({{"File",{MKFile}}});
		auto MKExtract = AA::FeaturesExtractor::create(FEPars);
		// Increment the files number and connect to the progress bar
		n_files++;
		connect(MKExtract.data(), SIGNAL(justFinished()), this, SLOT(on_MatchingProgressBarStepUp()));
		for(const QString& MUFile: MUScan->getOutContent().first().first()){
			auto MURead = AA::AudioReader::create({{"File",{MUFile}}});
			auto MUExtract = AA::FeaturesExtractor::create(FEPars);
			auto DistCalculator = AA::FeaturesDistance::create({ {"KeepInput", false} });
			// Increment the files number and connect to the progress bar
			n_files++;
			connect(MUExtract.data(), SIGNAL(justFinished()), this, SLOT(on_MatchingProgressBarStepUp()));
			// Link algorithms
			MKRead >> MKExtract >> DistCalculator;
			MURead >> MUExtract >> DistCalculator >> Histogram;
		}
	}
	// Connections - plot the histogram
	connect(Histogram.data(), SIGNAL(justFinished()), ui->MatchingChartView, SLOT(plotHistogram()));
	// Read the database
	QFile file(ui->MDBLineEdit->text());
	if(file.open(QFile::ReadOnly)){
		QVector<double> C1, C2;
		QString S1, S2;
		QDataStream istream(&file);
		istream >> S1 >> C1 >> S2 >> C2;
		file.close();
		// Declare the chi square test algorithms
		auto Evaluate1 = UMF::EvaluateGaussExp::create({
			{"Coefficients", QVariant::fromValue<QVector<double>>(C1)},
			{"OITable", QVariant::fromValue( QMapStringString({{"HistX", "X"}}) ) }
		});
		auto Evaluate2 = UMF::EvaluateGaussExp::create({
			{"Coefficients", QVariant::fromValue<QVector<double>>(C2)},
			{"OITable", QVariant::fromValue( QMapStringString({{"HistX", "X"}}) ) }
		});
		auto CST1 = UMF::ChiSquareTest::create({
			{"Confidence", {0.05}},
			{"OITable", QVariant::fromValue( QMapStringString({{"HistY", "Distributions"}, {"Y", "Distributions"}}) ) }
		});
		auto CST2 = UMF::ChiSquareTest::create({
			{"Confidence", {0.05}},
			{"OITable", QVariant::fromValue( QMapStringString({{"HistY", "Distributions"}, {"Y", "Distributions"}}) ) }
		});
		Histogram >> CST1;
		Histogram >> CST2;
		Histogram >> Evaluate1 >> CST1;
		Histogram >> Evaluate2 >> CST2;
		if(S1.startsWith("Extra") && S2.startsWith("Intra")){
			CST1->setObjectName("Extra");
			CST2->setObjectName("Intra");
		} else if(S1.startsWith("Intra") && S2.startsWith("Extra")){
			CST1->setObjectName("Intra");
			CST2->setObjectName("Extra");
		} else {
			popupErrorWindow("Check input database");
		}
		// Connections - plot curves from the database
		connect(Evaluate1.data(), SIGNAL(justFinished()), ui->MatchingChartView, SLOT(plotGaussianCurve()));
		connect(Evaluate2.data(), SIGNAL(justFinished()), ui->MatchingChartView, SLOT(plotGaussianCurve()));
		// Connections - final results
		connect(CST1.data(), SIGNAL(justFinished()), this, SLOT(on_newResult()));
		connect(CST2.data(), SIGNAL(justFinished()), this, SLOT(on_newResult()));
		// Set up the progress bar
		ui->MatchingProgressBar->setRange(0, n_files);
		ui->MatchingProgressBar->reset();
		// Make them start
		QThreadPool::globalInstance()->start(CST1.data());
		QThreadPool::globalInstance()->start(CST2.data());
	}
}

void GUI::Window::on_MatchingProgressBarStepUp(){
	ui->MatchingProgressBar->setValue(ui->MatchingProgressBar->value()+1);
	ui->DBProgressBar->update();
}

void GUI::Window::on_newResult(){
	auto CST = dynamic_cast<UMF::ChiSquareTest*>(QObject::sender());
	if(CST != Q_NULLPTR){
		// Get previous label
		QString label = ui->MatchingResultsLabel->text();
		label += "Known and unknown voices ";
		// Append a string based on the matching result
		if(CST->getOutDistrAreEqual()){
			// Same distributions, i.e. match true
			label += "ARE ";
		} else {
			// Different distributions, i.e. match false
			label += "ARE NOT ";
		}
		// Append a string based on the kind of test (intra or extra)
		if(CST->objectName().startsWith("Intra")){
			label += "THE SAME ";
		} else {
			label += "DIFFERENT ";
		}
		// Append a string based on the reliability of the test
		// FInish the sentence and add a line break at the end
		label += "voices (Confidence " + QLocale().toString(CST->getConfidence()) + ", similarity " +
		QLocale().toString(CST->getOutSimilarity()) + ")<br>";
		// Set the result text
		ui->MatchingResultsLabel->setText(label);
	}
}

void GUI::Window::on_MDBBrowseButton_clicked(){
	// Create and execute a file browser process
	QString fileName = QFileDialog::getOpenFileName(this, "Select DB file", QString(), "Audio Database file (*.adb)");
	// Check if the user pressed cancel
	if (fileName.isEmpty()) return;
	// Share path with the line edit
	ui->MDBLineEdit->setText(fileName);
}

void GUI::Window::on_MKBrowseButton_clicked(){
	// Create string with supported audio files
	QString supportedAudio = "Audio file (" + supportedAudioFormats.join(" ") + ")";
	// Create and execute a file browser process
	QString fileName = QFileDialog::getOpenFileName(this, "Select culprit audio file", QString(), supportedAudio);
	// Check if the user pressed cancel
	if (fileName.isEmpty()) return;
	// Share path with the line edit
	ui->MKLineEdit->setText(fileName);
}

void GUI::Window::on_MUBrowseButton_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getExistingDirectory(this, "Select suspects folder");
	// Check if the user pressed cancel
	if (folderName.isEmpty()) return;
	// Share path with the line edit
	ui->MULineEdit->setText(folderName);
}

