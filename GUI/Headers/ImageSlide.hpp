#ifndef ImageSlide_hpp
#define ImageSlide_hpp

#include <QLabel>
#include <QList>
#include <QDir>
#include <QLocale>

namespace Ui {
	class ImageSlide;
}

class Ui::ImageSlide : public QLabel {
	
	Q_OBJECT
	
private:
	QList<QDir> dirList;
	QList<QDir>::iterator current;
	
protected:
	/** Shows the content of the current file. */
	void display();
	
public:
	/** Empty constructor. */
	ImageSlide(QWidget* parent = Q_NULLPTR) : QLabel(parent) {};
	
	public slots:
	/** Appends a path to the widget and shows it. */
	void append(QDir);
	
	/** Go to next image in list, if any, and shows it. */
	void next();
	
	/** Go to previous image in list, if any, and shows it. */
	void prev();
	
	/** Remove every stored path. */
	void reset();
	
signals:
	/** Signal emitted when an image is displayed onto screen.
	 Allows to handle an image description.
	 */
	void changedImage(QString);
};

#endif /* ImageSlide_hpp */
