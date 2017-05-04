#ifndef LineEditDropReceiver_hpp
#define LineEditDropReceiver_hpp

#include <QLineEdit>
#include <QDropEvent>
#include <QString>
#include <QMimeData>

namespace Ui {
	class LineEditDropReceiver;
}

/** QLineEdit subclass that handles file drag and drop. */
class Ui::LineEditDropReceiver : public QLineEdit {
	Q_OBJECT
	
	/** Drop event callback.
	 This function is executed every time something is dropped on this class' instances.
	 Sets the text of this QLineEdit to dropped file path, if any.
	 */
	void dropEvent(QDropEvent* event);
	
	/** Drag event callback.
	 Allows only URLs dragging and dropping.
	 */
	void dragEnterEvent(QDragEnterEvent* event);
	
public:
	LineEditDropReceiver(QWidget *parent = Q_NULLPTR) : QLineEdit(parent) {}; /**< Call the QLineEdit constructor. */
};

#endif /* LineEditDropReceiver_hpp */
