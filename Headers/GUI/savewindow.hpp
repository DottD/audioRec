#ifndef SAVEWINDOW_H
#define SAVEWINDOW_H

#include <QWidget>
#include <QtCharts>
#include <GUI/DatabaseChart.hpp>

namespace Ui {
class SaveWindow;
}

class SaveWindow : public QWidget
{
    Q_OBJECT
	
public:
    explicit SaveWindow(QHash<QString, GUI::DatabaseLine*> lines, QWidget *parent = 0);
    ~SaveWindow();
	
	void setLines(QHash<QString, GUI::DatabaseLine*> lines){this->lines = lines;};
	QHash<QString, GUI::DatabaseLine*> getLines(){return lines;};
	
	void setLineWithName(GUI::DatabaseLine* line, QString name);
	
	public Q_SLOTS:
	Q_SLOT void on_browserPushButton_clicked();
	
	Q_SLOT void on_intraComboBox_activated(const QString& text);
	
	Q_SLOT void on_extraComboBox_activated(const QString& text);
	
	Q_SLOT void on_saveButton_clicked();

private:
    Ui::SaveWindow *ui;
	
	QHash<QString, GUI::DatabaseLine*> lines;
};

QDataStream& operator<<(QDataStream&, const QSettings&);

QDataStream& operator>>(QDataStream&, QSettings&);

#endif // SAVEWINDOW_H
