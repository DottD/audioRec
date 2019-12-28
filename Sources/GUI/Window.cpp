#include <GUI/Window.hpp>

GUI::Window::Window(QWidget *parent) :
QWidget(parent),
ui(new Ui::WindowUI),
processed(0),
fileCount(0) {
	// Initial setup - mandatory
	ui->setupUi(this);
	// Set default parameters if never set
	if(!QSettings().value("alreadySet").toBool())
		on_ResetParametersButton_clicked();
	// Set window dimensions
	{
		QSettings settings;
		settings.beginGroup("windowSize");
		if(!settings.value("width").isValid()) settings.setValue("width", 480);
		if(!settings.value("height").isValid()) settings.setValue("height", 640);
		resize(settings.value("width").toInt(), settings.value("height").toInt());
		settings.endGroup();
	}
	// Set the line edit as drag and drop receiver
	ui->DBFilesLineEdit->setDragEnabled(true);
	ui->MULineEdit->setDragEnabled(true);
	ui->FILineEdit->setDragEnabled(true);
	// Set icons for record scrolling buttons
	ui->ButtonPreviousRecord->setIcon(QIcon(":/48x48/Prev.png"));
	ui->ButtonNextRecord->setIcon(QIcon(":/48x48/Successive.png"));
	// Connect the record visualization widgets for error handling
	checked_connect(ui->ChartShowRec, &ChartRecWidget::raise, this, &Window::popupError, Qt::AutoConnection);
	checked_connect(ui->ChartShowRecSpectrum, &ChartRecWidget::raise, this, &Window::popupError, Qt::AutoConnection);
	// Set X axis title
	ui->ChartShowRec->setProperty("xAxisTitle", "Time (s)");
	ui->ChartShowRecSpectrum->setProperty("xAxisTitle", "Frequency (Hz)");
	// Set charts names
	ui->ChartShowRec->chart()->setTitle("Time Domain");
	ui->ChartShowRecSpectrum->chart()->setTitle("Frequency Domain");
	// Create a FeaturesExtractor instance attached to FileInspectorTab
	auto extractor = AA::FeaturesExtractor::create(getPropsInGroup("FeaturesExtraction"));
	extractor->setSelectRecord(-1);
	checked_connect(extractor.data(), &QAlgorithm::raise, this, &Window::popupError, Qt::AutoConnection);
	checked_connect(extractor.data(), &AA::FeaturesExtractor::timeSeries, ui->ChartShowRec, &GUI::ChartRecWidget::addSeries, Qt::AutoConnection);
	checked_connect(extractor.data(), &AA::FeaturesExtractor::frequencySeries, ui->ChartShowRecSpectrum, &GUI::ChartRecWidget::addSeries, Qt::AutoConnection);
	checked_connect(extractor.data(), &AA::FeaturesExtractor::pointSeries, ui->ChartShowRecSpectrum, &GUI::ChartRecWidget::addPoints, Qt::AutoConnection);
	ui->FileInspectorTab->setProperty("extractor", QVariant::fromValue(extractor));
	// Set charts index conversion functions
	ui->ChartShowRec->indexToXAxis = [extractor](int k){
		const auto& SampleRate = extractor->getOutSampleRate();
		const auto& RecLength = extractor->getOutRecordLength();
		return double(RecLength * extractor->getSelectRecord() + k) / double(SampleRate);
	};
	ui->ChartShowRecSpectrum->indexToXAxis = [extractor](int k){
		const auto& SampleRate = extractor->getOutSampleRate();
		const auto& RecLength = extractor->getOutRecordLength();
		return double(k) * double(SampleRate) / double(RecLength);
	};
	// Set whether to use a logarithmic scale for the frequency plot
	if (ui->CheckLogScale->isChecked()){
		ui->ChartShowRecSpectrum->valueToYAxis = [](double y){return log10(y>0?y/1E-7:1E-20)*10.0;};
		ui->ChartShowRecSpectrum->setProperty("yAxisTitle", "Amplitude (dB)");
	} else {
		ui->ChartShowRecSpectrum->valueToYAxis = [](double y){return y;};
		ui->ChartShowRecSpectrum->setProperty("yAxisTitle", "Amplitude");
	}
	// Set record description
	ui->LabelRecordDescription->setText(QLocale().toString(0));
	// Reset the progress bars
	qRegisterMetaType<QVector<double>>();
	qRegisterMetaType<QVector<int>>();
}

GUI::Window::~Window(){
	delete ui;
}

QAlgorithm::PropertyMap GUI::Window::getPropsInGroup(const QString& group){
	QSettings settings;
	settings.beginGroup(group);
	QAlgorithm::PropertyMap parameters;
	for(auto& key: settings.childKeys())
		parameters.insert(key, settings.value(key));
	settings.endGroup();
	return parameters;
}

void GUI::Window::setupSettingsTab(){
	// Add structured key-value settings (first level only) to the settings tab
	QSettings settings;
	auto parent = new QWidget;
	// Remove children and layout
	if(auto children = parent->findChildren<QWidget*>("", Qt::FindDirectChildrenOnly); !children.isEmpty()){
		qDeleteAll(children);
	}
	delete parent->layout();
	// Scan settings and add widgets
	auto parentLayout = new QVBoxLayout(parent);
	for(auto& group: settings.childGroups()){
		auto groupBox = new QGroupBox;
		parentLayout->addWidget(groupBox);
		groupBox->setTitle(group);
		groupBox->setAlignment(Qt::AlignHCenter);
		auto formLayout = new QFormLayout;
		groupBox->setLayout(formLayout);
		settings.beginGroup(group);
		for(auto& key: settings.childKeys()){
			QWidget* field;
			if(settings.value(key).type() == QVariant::Type::LongLong){
				// Check if this key has an attached enum object
				if(settings.childGroups().contains(key+"Enum")){
					// In this case construct a ComboBox to be filled with the possible options
					field = new QComboBox;
					if(auto comboBox = dynamic_cast<QComboBox*>(field); comboBox){
						settings.beginGroup(key+"Enum");
						comboBox->addItems(settings.childKeys());
						settings.endGroup();
						comboBox->setObjectName(group+"/"+key);
						connect(comboBox, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::activated),
								[comboBox](const QString &text){
									QSettings settings;
									int value = settings.value(comboBox->objectName()+"Enum/"+text).toInt();
									settings.setValue(comboBox->objectName(), value);
								});
					}
				}else{
					// In this case construct a LineEdit with more freedom
					field = new QLineEdit;
					if(auto lineEdit = dynamic_cast<QLineEdit*>(field); lineEdit){
						lineEdit->setValidator(new QIntValidator);
						lineEdit->setText(QLocale().toString(settings.value(key).toLongLong()));
						lineEdit->setObjectName(group+"/"+key);
						connect(lineEdit, &QLineEdit::editingFinished, [lineEdit](){
							QSettings().setValue(lineEdit->objectName(), QLocale().toLongLong(lineEdit->text()));
						});
					}
				}
			}else if(settings.value(key).type() == QVariant::Type::Double){
				field = new QLineEdit;
				if(auto lineEdit = dynamic_cast<QLineEdit*>(field); lineEdit){
					lineEdit->setValidator(new QDoubleValidator);
					lineEdit->setText(QLocale().toString(settings.value(key).toDouble()));
					lineEdit->setObjectName(group+"/"+key);
					connect(lineEdit, &QLineEdit::editingFinished, [lineEdit](){
						QSettings().setValue(lineEdit->objectName(), QLocale().toDouble(lineEdit->text()));
					});
				}
			}else if(settings.value(key).type() == QVariant::Type::Bool){
				field = new QCheckBox;
				if(auto checkBox = dynamic_cast<QCheckBox*>(field); checkBox){
					checkBox->setChecked(settings.value(key).toBool());
					checkBox->setObjectName(group+"/"+key);
					connect(checkBox, &QCheckBox::stateChanged, [checkBox](){
						QSettings().setValue(checkBox->objectName(), checkBox->isChecked());
					});
				}
			}
			else continue;
			formLayout->addRow(key, field);
		}
		settings.endGroup();
		// If no key is compatible, remove the group box
		if(formLayout->rowCount() <= 0){
			parentLayout->removeWidget(groupBox);
			delete groupBox;
		}
	}
	ui->SettingsScroll->setWidget(parent);
}

void GUI::Window::resizeEvent(QResizeEvent *event){
	QSettings settings;
	settings.beginGroup("windowSize");
	settings.setValue("width", width());
	settings.setValue("height", height());
	settings.endGroup();
}

void GUI::Window::popupError(QString errorDescription) {
	QMessageBox::critical(this, "Error", errorDescription);
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

void GUI::Window::on_DBFilesLineEdit_textChanged(const QString &text){
	// Recursively scan the input database folder for supported audio files
	auto dirScanner = ScanDirectory::create({
		{"Extensions", supportedAudioFormats},
		{"Folder", text},
		{"Recursive", true}
	});
	dirScanner->run();
	// Store the results in the window instance
	foundFiles = dirScanner->getOutContent();
}

void GUI::Window::on_DBCreateButton_clicked(){
	// Check if it input is empty
	if (ui->DBFilesLineEdit->text().isEmpty()){
		popupErrorWindow("An input folder must be chosen");
	}
	// Get the list of files in the input directory
	if(foundFiles.isEmpty()) {
		popupErrorWindow("Empty directory");
	}
	// Create activity indicator
	auto progressDialog = new QProgressDialog("Processing files...", QString(), 0, 0, this);
	progressDialog->setValue(0);
	auto pbStepUp = [progressDialog](){
		progressDialog->setValue(progressDialog->value()+1);
	};
	// Set up features extraction parameters for later use
	QAlgorithm::PropertyMap FEPars = {
		{"File", QString()},
		{"SelectRecord", -1}
	};
	FEPars.unite(getPropsInGroup("FeaturesExtraction"));
	// Collect histogram paramters for later use
	auto HistPars = getPropsInGroup("Histogram");
	HistPars.insert("OITable", QVariant::fromValue(QMapStringString({{"Distance","Values"}})));
	// Collect fitting parameters for later use
	auto fittingPars = getPropsInGroup("Fitting");
	fittingPars.insert("KeepInput", true);
	fittingPars.insert("OITable", QVariant::fromValue(QMapStringString({{"HistX","X"},{"HistY","Y"}})));
	// Declare a list of extractors (this intermediate step will avoid reloading files and recomputing the features)
	// extractors will have the same structure of dirContent (1-1 correspondance)
	int n_extractor = 0;
	QList<QList<QSharedPointer<AA::FeaturesExtractor>>> extractors;
	for(const auto& dir: foundFiles){// foreach subdir
		// Create a new sublist
		extractors << QList<QSharedPointer<AA::FeaturesExtractor>>();
		for(const auto& file: dir){ // foreach file
			// Set current file as input to an audio reader instance
			// Extract features after the reader has completed its task
			FEPars["File"] = file;
			auto extractor = AA::FeaturesExtractor::create(FEPars);
			++n_extractor;
			extractor->setObjectName(QFileInfo(file).baseName());
			// Add to extractors to the last sublist created
			extractors.last() << extractor;
			// Connect the extractor with the progress dialog
			connect(extractor.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
		}
	}
	// Update progress dialog's maximum
	progressDialog->setMaximum(progressDialog->maximum()+n_extractor);
	// Intra-speaker features
	{
		// Create a new database line to store the features computed
		auto line = new DatabaseLine;
		line->setColor(DatabaseLine::genNewColor());
		line->setName("Intra-Speaker Fitting "+ui->DBPlotName->text());
		line->setType(GUI::DatabaseLine::GaussianExp);
		line->setNumPoints(QSettings().value("Plot/Points").toInt());
		// Create an histogram series
		auto histogram = new QHistogramSeries;
		histogram->setColor(line->color().darker());
		histogram->setBorderColor(QColor(0,0,0,0)/*transparent*/);
		histogram->setName("Intra-Speaker Histogram "+ui->DBPlotName->text());
		// Compute the histogram of all the intra-speaker distances
		auto histCompute = UMF::ComputeHistogram::create(HistPars);
		histCompute->setObjectName("Intra histogram");
		// Compute the number of distances used for extra-speaker distribution
		int n_int_dist = 0;
		// Define a map of already used pairs to avoid to compute the same distance twice
		QSet<QSet<QAlgorithm*>> used_pairs;
		for(auto& dir: extractors){
			for(auto& outerExtractor: dir){
				// Store the features into the database line
				connect(outerExtractor.data(), &QAlgorithm::justFinished, this, [line,outerExtractor](){
					line->addFeatures(outerExtractor->getOutFeatures());
				});
				// Scan the files in the directory again and compute the distance
				// between each couple of features array
				for(auto& innerExtractor: dir){
					// Some checks:
					auto sameFile = (outerExtractor == innerExtractor); // outer-inner are the same file
					auto alreadyUsedPairs = used_pairs.contains({outerExtractor.data(), innerExtractor.data()});
					if(!sameFile && !alreadyUsedPairs){
						// Distance calculator
						auto distanceCalculator = AA::FeaturesDistance::create();
						outerExtractor >> distanceCalculator;
						innerExtractor >> distanceCalculator >> histCompute;
						distanceCalculator->setObjectName(outerExtractor->objectName().replace(" extract", "")+"-"+innerExtractor->objectName().replace(" extract", ""));
						// Increment the files number
						n_int_dist++;
						// Add this pair of extractors to used_pairs
						used_pairs << QSet<QAlgorithm*>({outerExtractor.data(), innerExtractor.data()});
					}
				}
			}
		}
		// Proceed only if there is at least one distance to process
		if (n_int_dist > 0){
			// When the histogram is computed draw the histogram
			connect(histCompute.data(), &UMF::ComputeHistogram::histogramReady, this/*as context*/,
					[this, histogram](QVector<double> Bin, QVector<double> Count){
						histogram->setX(Bin);
						histogram->setY(Count);
						ui->DBChartView->updateViewWith(histogram);
					}, Qt::QueuedConnection);
			// Change output names and perform fitting
			auto intraFitting = UMF::FittingGaussExp::create(fittingPars);
			intraFitting->setObjectName("IntraFitting");
			histCompute >> intraFitting;
			// Extend the maximum value in the progress dialog
			progressDialog->setMaximum(progressDialog->maximum()+1);
			// Connect the fitting instance to the progress dialog
			connect(intraFitting.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
			// When the fitting algorithm finishes call on_DBCreated and plot the fitted curve
			connect(intraFitting.data(), &UMF::Fitting1D::fittingReady, this/*as context*/,
					[this, line](QVector<double> C, double min, double max){
						line->setMinimum(min);
						line->setMaximum(max);
						line->setCoefficients(C);
						ui->DBChartView->updateViewWith(line);
					}, Qt::QueuedConnection);
			QAlgorithm::improveTree(intraFitting.data());
			intraFitting->parallelExecution();
		}
	}
	// Extra-speaker features
	{
		// Create a new database line to store the features computed
		auto line = new DatabaseLine;
		line->setColor(DatabaseLine::genNewColor());
		line->setName("Extra-Speaker Fitting "+ui->DBPlotName->text());
		line->setType(GUI::DatabaseLine::GaussianExp);
		line->setNumPoints(QSettings().value("Plot/Points").toInt());
		// Create an histogram series
		auto histogram = new QHistogramSeries;
		histogram->setColor(line->color().darker());
		histogram->setBorderColor(QColor(0,0,0,0)/*transparent*/);
		histogram->setName("Extra-Speaker Histogram "+ui->DBPlotName->text());
		// Compute the histogram of all the extra-speaker distances
		auto histCompute = UMF::ComputeHistogram::create(HistPars);
		histCompute->setObjectName("Extra histogram");
		// Compute the number of distances used for extra-speaker distribution
		int n_ext_dist = 0;
		// Define a map of already used pairs to avoid to compute the same distance twice
		QSet<QSet<QAlgorithm*>> used_pairs;
		for(auto& outerDir: extractors){
			for(auto& outerExtractor: outerDir){
				// Store the features into the database line
				connect(outerExtractor.data(), &QAlgorithm::justFinished, this, [line,outerExtractor](){
					line->addFeatures(outerExtractor->getOutFeatures());
				});
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
							auto distanceCalculator = AA::FeaturesDistance::create();
							outerExtractor >> distanceCalculator;
							innerExtractor >> distanceCalculator >> histCompute;
							distanceCalculator->setObjectName(outerExtractor->objectName().replace(" extract", "")+"-"+innerExtractor->objectName().replace(" extract", ""));
							// Increment the files number
							n_ext_dist++;
							// Add this pair of extractors to used_pairs
							used_pairs << QSet<QAlgorithm*>({outerExtractor.data(), innerExtractor.data()});
						}
					}
				}
			}
		}
		// Proceed only if there is at least one distance to process
		if (n_ext_dist > 0){
			// When the histogram is computed draw the histogram
			connect(histCompute.data(), &UMF::ComputeHistogram::histogramReady, this/*as context*/,
					[this, histogram](QVector<double> Bin, QVector<double> Count){
						histogram->setX(Bin);
						histogram->setY(Count);
						ui->DBChartView->updateViewWith(histogram);
					}, Qt::QueuedConnection);
			// Change output names and perform fitting
			auto extraFitting = UMF::FittingGaussExp::create(fittingPars);
			extraFitting->setObjectName("ExtraFitting");
			histCompute >> extraFitting;
			// Extend the maximum value in the progress dialog
			progressDialog->setMaximum(progressDialog->maximum()+1);
			// Connect the fitting instance to the progress dialog
			connect(extraFitting.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
			// When the fitting algorithm finishes call on_DBCreated and plot the fitted curve
			connect(extraFitting.data(), &UMF::Fitting1D::fittingReady, this/*as context*/,
					[this, line](QVector<double> C, double min, double max){
						line->setMinimum(min);
						line->setMaximum(max);
						line->setCoefficients(C);
						ui->DBChartView->updateViewWith(line);
					}, Qt::QueuedConnection);
			QAlgorithm::improveTree(extraFitting.data());
			extraFitting->parallelExecution();
		}
	}
	progressDialog->exec();
}

void GUI::Window::on_DBSaveButton_clicked(){
	// Get the list of all the line series
	auto series = ui->DBChartView->chart()->series();
	QHash<QString, GUI::DatabaseLine*> lines;
	for(auto s: series){
		if(auto lineS = dynamic_cast<GUI::DatabaseLine*>(s); lineS)
			lines.insert(lineS->name(), lineS);
	}
	// If it is empty popup an error message
	if(lines.isEmpty()) popupErrorWindow("No data available");
	// Create the window and send names
	auto saveWindow = new SaveWindow(lines);
	saveWindow->show();
}

void GUI::Window::on_DBLoadButton_clicked(){
	QString fileName = QFileDialog::getOpenFileName(this, "Choose origin file name", QDir::currentPath(), "Text files (*.txt)");
	if (!fileName.isEmpty()){ // Check if the user pressed cancel
		// Save the database to that file
		QFile file(fileName);
		if(file.open(QFile::ReadOnly)){
			// Read two lines but no settings
			auto lineIntra = new GUI::DatabaseLine;
			auto lineExtra = new GUI::DatabaseLine;
			QDataStream istream(&file);
			istream >> *lineIntra >> *lineExtra;
			file.close();
			// Show the curves on the chart
			ui->DBChartView->updateViewWith(lineIntra);
			ui->DBChartView->updateViewWith(lineExtra);
		}
	}
}

void GUI::Window::on_PlotCtrlCleanButton_clicked(){
	ui->DBChartView->chart()->removeAllSeries();
}

// Matching tab callbacks

void GUI::Window::on_MCtrlResetButton_clicked(){
	// Reset chart
	ui->MatchingChartView->chart()->removeAllSeries();
	// Reset results tab
	ui->MatchingResultsLabel->setText("Distribution Similarity Estimation");
	// Reset the combo boxes
	ui->MIntraCurveComboBox->clear();
	ui->MExtraCurveComboBox->clear();
}

void GUI::Window::on_MCtrlMatchButton_clicked(){
	// Check input and output
	if (ui->MULineEdit->text().isEmpty()){
		popupErrorWindow("Check the input folder");
	}
	if(ui->MIntraCurveComboBox->currentText().isEmpty() || ui->MExtraCurveComboBox->currentText().isEmpty()){
		popupErrorWindow("Use the drop-down menus to choose two curves as a database");
	}
	// Create activity indicator
	auto progressDialog = new QProgressDialog("Processing files...", QString(), 0, 0, this);
	progressDialog->setValue(0);
	auto pbStepUp = [progressDialog](){
		progressDialog->setValue(progressDialog->value()+1);
	};
	// Reset the result label
	ui->MatchingResultsLabel->setText("");
	// Load parameters
	QAlgorithm::PropertyMap FEPars = {
		{"File", QString()},
		{"SelectRecord", -1}
	};
	FEPars.unite(getPropsInGroup("FeaturesExtraction"));
	// Get the list of unknown voice file paths
	QStringList MUScanFlattened;
	{
		auto MUScan = GUI::ScanDirectory::create({
			{"Folder", ui->MULineEdit->text()},
			{"Recursive", true},
			{"Extensions", supportedAudioFormats}
		});
		MUScan->run();
		auto MUScanContent = MUScan->getOutMoveContent();
		for(auto lst: MUScanContent) MUScanFlattened << lst;
		if(MUScanFlattened.isEmpty()) popupErrorWindow("No audio files in Unknown directory");
	}
	// Define the histogram algorithm
	QAlgorithm::PropertyMap HistogramSettings = {
		QAlgorithm::makeOITable({{"Distance","Values"}}),
		{"BarStep", QSettings().value("Histogram/BarStep")},
		{"SuppressZeroCount", false}
	};
	auto HistogramIntra = UMF::ComputeHistogram::create(HistogramSettings);
	auto HistogramExtra = UMF::ComputeHistogram::create(HistogramSettings);
	HistogramIntra->setObjectName("Intra");
	HistogramExtra->setObjectName("Extra");
	// Declare and initialize the total number of files
	QList<QSharedPointer<AA::FeaturesExtractor>> MUExtractors;
	for(auto MUFile: MUScanFlattened){
		FEPars["File"] = MUFile;
		auto MUExtract = AA::FeaturesExtractor::create(FEPars);
		// Connect the extractor to the progress dialog
		connect(MUExtract.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
		// Add to the list
		MUExtractors << MUExtract;
	}
	// Take information from the lines in the chart
	GUI::DatabaseLine *intraLine = 0, *extraLine = 0;
	for(auto s: ui->MatchingChartView->chart()->series()){
		if(auto line = dynamic_cast<GUI::DatabaseLine*>(s); line){
			if(line->name() == ui->MIntraCurveComboBox->currentText())
				intraLine = line;
			else if(line->name() == ui->MExtraCurveComboBox->currentText())
				extraLine = line;
		}
	}
	if(!intraLine || !extraLine){
		popupErrorWindow("Load some database on the chart first");
	}
	// Update the maximum value for the progress dialog
	progressDialog->setMaximum(MUExtractors.size()/*file readings*/ +
							   MUExtractors.size()*(intraLine->getFeatures().size()+extraLine->getFeatures().size())/*dist. calc.*/ +
							   1/*final algorithms*/);
	// Create the distance calculators and connect them to the extractors
	for(auto MUExtract: MUExtractors){
		for(auto IntraFeatures: intraLine->getFeatures()){
			auto DistCalculator = AA::FeaturesDistance::create();
			// Assign the first features array in the distance calculator
			DistCalculator->setInFeatures(IntraFeatures);
			// Link algorithms
			MUExtract >> DistCalculator >> HistogramIntra;
			// Connect the extractor to the progress dialog
			connect(DistCalculator.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
		}
		for(auto ExtraFeatures: extraLine->getFeatures()){
			auto DistCalculator = AA::FeaturesDistance::create();
			// Assign the first features array in the distance calculator
			DistCalculator->setInFeatures(ExtraFeatures);
			// Link algorithms
			MUExtract >> DistCalculator >> HistogramExtra;
			// Connect the extractor to the progress dialog
			connect(DistCalculator.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
		}
	}
	// Connections - plot the histograms
	connect(HistogramIntra.data(), &UMF::ComputeHistogram::histogramReady, this/*context*/,
			[this](QVector<double> Bin, QVector<double> Count){
				QString Name("Intra-Unknown Distances Histogram");
				ui->MatchingChartView->plotHistogram(Bin, Count, Name);
			}, Qt::QueuedConnection);
	connect(HistogramExtra.data(), &UMF::ComputeHistogram::histogramReady, this/*context*/,
			[this](QVector<double> Bin, QVector<double> Count){
				QString Name("Extra-Unknown Distances Histogram");
				ui->MatchingChartView->plotHistogram(Bin, Count, Name);
			}, Qt::QueuedConnection);
	// Specify the test properties
	auto Test = AA::ComputeProbability::create({
		{"IntraCoefficients", QVariant::fromValue<QVector<double>>(intraLine->getCoefficients())},
		{"ExtraCoefficients", QVariant::fromValue<QVector<double>>(extraLine->getCoefficients())},
		{"LeftExtremum", QSettings().value("Histogram/MinimumValue")},
		{"RightExtremum", QSettings().value("Histogram/MaximumValue")},
		QAlgorithm::makeOITable({
			{"HistX", "IntraX"},
			{"HistY", "IntraY"},
			{"HistX", "ExtraX"},
			{"HistY", "ExtraY"}// the histograms algorithms must be named "Intra" or "Extra"
		})
	});
	Test->setObjectName("FinalTest");
	// Connect the algorithms
	HistogramIntra >> Test;
	HistogramExtra >> Test;
	// Connections - final result
	connect(Test.data(), &QAlgorithm::justFinished, this, [this, Test](){
		// Get previous label
		QString label = ui->MatchingResultsLabel->text();
		// Check if the test has been performed
		if(Test->getOutMatchingScore() >= 0.0){
			label += "The unknown voice belongs to the speaker with " + QLocale().toString(Test->getOutMatchingScore()*100.0) + "% probability<br>";
		}else{
			label += "INCONCLUSIVE<br>";
		}
		// Set the result text
		ui->MatchingResultsLabel->setText(label);
	});
	// Connect the two tests to the progress dialog
	connect(Test.data(), &QAlgorithm::justFinished, this/*context*/, pbStepUp, Qt::QueuedConnection);
	// Make them start
	Test->parallelExecution();
	// Display the progress dialog
	progressDialog->exec();
}

void GUI::Window::on_MUBrowseButton_clicked(){
	QString folderName = QFileDialog::getExistingDirectory(this, "Select a folder", QDir::currentPath());
	// Check if the user pressed cancel
	if (!folderName.isEmpty()){
		// Share path with the line edit
		ui->MULineEdit->setText(folderName);
	}
}

void GUI::Window::on_TabWidget_currentChanged(int index){
	QString text = ui->TabWidget->tabText(index);
	if(text == "Settings"){
		setupSettingsTab();
	}
}

void GUI::Window::on_MLoadDatabaseButton_clicked(){
	QString fileName = QFileDialog::getOpenFileName(this, "Choose origin file name", QDir::currentPath(), "Text files (*.txt)");
	if (!fileName.isEmpty()){ // Check if the user pressed cancel
		// Save the database to that file
		QFile file(fileName);
		if(file.open(QFile::ReadOnly)){
			// Read two lines and settings
			auto lineIntra = new GUI::DatabaseLine;
			auto lineExtra = new GUI::DatabaseLine;
			QSettings settings;
			QDataStream istream(&file);
			istream >> *lineIntra >> *lineExtra >> settings;
			file.close();
			// Show the curves on the chart
			ui->MatchingChartView->updateViewWith(lineIntra);
			ui->MatchingChartView->updateViewWith(lineExtra);
			// Fill in the combo boxes
			ui->MIntraCurveComboBox->addItems({lineIntra->name(), lineExtra->name()});
			ui->MExtraCurveComboBox->addItems({lineIntra->name(), lineExtra->name()});
		}
	}
}

// File Inspectors tab callbacks

void GUI::Window::on_ButtonNextRecord_clicked(){
	auto extractor = ui->FileInspectorTab->property("extractor").value<QSharedPointer<AA::FeaturesExtractor>>();
	extractor->setParameters(getPropsInGroup("FeaturesExtraction"));
	auto recIdx = extractor->getSelectRecord();
	if (recIdx < extractor->getOutTotalRecords()-1) ++recIdx;
	ui->LabelRecordDescription->setText(QLocale().toString(recIdx));
	extractor->setSelectRecord(recIdx);
	// Clear all visualization widgets
	ui->ChartShowRec->chart()->removeAllSeries();
	ui->ChartShowRecSpectrum->chart()->removeAllSeries();
	// Execute the extractor on another thread
	extractor->parallelExecution();
}

void GUI::Window::on_ButtonPreviousRecord_clicked(){
	auto extractor = ui->FileInspectorTab->property("extractor").value<QSharedPointer<AA::FeaturesExtractor>>();
	extractor->setParameters(getPropsInGroup("FeaturesExtraction"));
	auto recIdx = extractor->getSelectRecord();
	if (recIdx > 0) --recIdx;
	ui->LabelRecordDescription->setText(QLocale().toString(recIdx));
	extractor->setSelectRecord(recIdx);
	// Clear all visualization widgets
	ui->ChartShowRec->chart()->removeAllSeries();
	ui->ChartShowRecSpectrum->chart()->removeAllSeries();
	// Execute the extractor on another thread
	extractor->parallelExecution();
}

void GUI::Window::on_FIBrowseButton_clicked(){
	// Create and execute a file browser process
	QString folderName = QFileDialog::getOpenFileName(this, "Select an audio file", QDir::currentPath(),
													  "Audio ("+supportedAudioFormats.join(" ")+")");
	// Check if the user pressed cancel
	if (!folderName.isEmpty()) {
		// Share path with the line edit
		ui->FILineEdit->setText(folderName);
	}
}

void GUI::Window::on_FILineEdit_textChanged(const QString& selectedFile){
	// If the line edit is empty, do nothing
	if (selectedFile.isEmpty()) return;
	// Process audio file and send results to widgets
	auto extractor = ui->FileInspectorTab->property("extractor").value<QSharedPointer<AA::FeaturesExtractor>>();
	extractor->setParameters(getPropsInGroup("FeaturesExtraction"));
	extractor->setFile(selectedFile);
	extractor->setSelectRecord(0);
	ui->LabelRecordDescription->setText(QLocale().toString(0));
	// Clear all visualization widgets
	ui->ChartShowRec->chart()->removeAllSeries();
	ui->ChartShowRecSpectrum->chart()->removeAllSeries();
	// Execute the extractor on another thread
	extractor->parallelExecution();
}

void GUI::Window::on_CheckLogScale_stateChanged(int state){
	if (ui->CheckLogScale->isChecked()){
		// Expressed in dB
		ui->ChartShowRecSpectrum->valueToYAxis = [](double y){return log10(y>0?y/1E-7:1E-20)*10.0;};
		ui->ChartShowRecSpectrum->setProperty("yAxisTitle", "Amplitude (dB)");
	} else {
		ui->ChartShowRecSpectrum->valueToYAxis = [](double y){return y;};
		ui->ChartShowRecSpectrum->setProperty("yAxisTitle", "Amplitude");
	}
}

// Settings Tab

void GUI::Window::on_ResetParametersButton_clicked(){
	QSettings settings;
	settings.clear();
	settings.setValue("alreadySet", true);
	settings.beginGroup("FeaturesExtraction");
	settings.setValue("MinimumFrequency", double(500.));
	settings.setValue("MaximumFrequency", double(3500.));
	settings.setValue("MaximumSpectrumLeakage", double(10.));
	addEnumSetting(settings, UMF::ReduceChannels, "ChannelsOperation", average);
	addEnumSetting(settings, UMF::ReduceChannels, "ChannelsArrangement", interleaved);
	addEnumSetting(settings, UMF::Windowing, "WindowingFunction", hann);
	addEnumSetting(settings, UMF::ArrayPad, "ExtrapolationMethod", constant);
	settings.setValue("GaussianFilterWidth", int(8));
	settings.setValue("BackIterations", int(6));
	addEnumSetting(settings, UMF::SpectrumRemoveBackground, "BackDirection", kBackIncreasingWindow);
	addEnumSetting(settings, UMF::SpectrumRemoveBackground, "BackFilterOrder", kBackOrder2);
	settings.setValue("BackSmoothing", false);
	addEnumSetting(settings, UMF::SpectrumRemoveBackground, "BackSmoothWindow", kBackSmoothing3);
	settings.setValue("BackCompton", false);
	settings.endGroup();
	settings.beginGroup("Histogram");
	settings.setValue("BarStep", double(0.02));
	settings.setValue("MinimumValue", double(0.0));
	settings.setValue("MaximumValue", double(2.0));
	settings.endGroup();
	settings.beginGroup("Plot");
	settings.setValue("Points", int(100));
	settings.endGroup();
	settings.beginGroup("ChiSquareTest");
	settings.setValue("Confidence", double(0.05));
	settings.endGroup();
	setupSettingsTab();
}
