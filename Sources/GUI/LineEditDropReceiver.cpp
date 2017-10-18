#include <GUI/LineEditDropReceiver.hpp>

void Ui::LineEditDropReceiver::dropEvent(QDropEvent* event){
	if (event->mimeData()->hasUrls()){
		QList<QUrl> urls = event->mimeData()->urls();
		if (urls.isEmpty()) throw std::runtime_error("No url found");
		setText(urls.first().toLocalFile());
	}
}

void Ui::LineEditDropReceiver::dragEnterEvent(QDragEnterEvent* event){
	if (event->mimeData()->hasUrls()){
		event->acceptProposedAction();
	}
}
