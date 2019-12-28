#include <GUI/savewindow.hpp>
#include "ui_savewindow.h"

SaveWindow::SaveWindow(QHash<QString, GUI::DatabaseLine*> lines, QWidget *parent) :
QWidget(parent),
ui(new Ui::SaveWindow)
{
	ui->setupUi(this);
	setLines(lines);
	// Check if lines is empty
	if(lines.isEmpty()) {
		close();
		return;
	}
	// Assign lines names to combo boxes
	auto names = lines.keys();
	names.sort();
	ui->intraComboBox->addItems(names);
	ui->extraComboBox->addItems(names);
	// Enable drag and drop
	ui->browserLineEdit->setDragEnabled(true);
}

SaveWindow::~SaveWindow()
{
	delete ui;
}

void SaveWindow::on_browserPushButton_clicked(){
	QString fileName = QFileDialog::getSaveFileName(this, "Choose destination file name", QDir::currentPath(), "Text files (*.txt)");
	if (!fileName.isEmpty()) // Check if the user pressed cancel
		ui->browserLineEdit->setText(fileName); // Share path with the line edit
}

void SaveWindow::on_intraComboBox_activated(const QString& text){
	QString name("intra");
	auto line = lines[text];
	setLineWithName(line, name);
}

void SaveWindow::on_extraComboBox_activated(const QString& text){
	QString name("extra");
	auto line = lines[text];
	setLineWithName(line, name);
}

void SaveWindow::setLineWithName(GUI::DatabaseLine* line, QString name){
	for(auto series: ui->chart->chart()->series()){
		if(series->name() == name){
			ui->chart->chart()->removeSeries(series);
			break;
		}
	}
	auto newLine = new QLineSeries;
	newLine->setName(name);
	newLine->setColor(line->color());
	*newLine << line->points();
	ui->chart->updateViewWith(newLine);
}

void SaveWindow::on_saveButton_clicked(){
	// Save the database to that file
	QFile file(ui->browserLineEdit->text());
	if(file.open(QFile::WriteOnly)){
		QDataStream ostream(&file);
		auto intraLine = lines[ui->intraComboBox->currentText()];
		auto extraLine = lines[ui->extraComboBox->currentText()];
		ostream << *intraLine << *extraLine << QSettings();
		file.close();
	}
	// Close the window
	close();
}

QDataStream& operator<<(QDataStream& stream, const QSettings& settings){
	QHash<QString, QVariant> settingsMap;
	for(auto key: settings.allKeys()){
		settingsMap.insert(key, settings.value(key));
	}
	stream << settingsMap;
	return stream;
}

QDataStream& operator>>(QDataStream& stream, QSettings& settings){
	QHash<QString, QVariant> settingsMap;
	stream >> settingsMap;
	for(auto i = settingsMap.constBegin(); i != settingsMap.constEnd(); ++i){
		settings.setValue(i.key(), i.value());
	}
	return stream;
}
