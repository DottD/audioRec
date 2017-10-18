#include <GUI/ImageSlide.hpp>

void Ui::ImageSlide::display(){
	// Check if there is something in the list
	if (dirList.isEmpty()) return;
	// Load the picture from file
	QImage image(current->absolutePath());
	// Test if loading gives errors
	if (image.isNull()) return;
	// Convert image to a pixmap
	QPixmap pixmap;
	pixmap.convertFromImage(image);
	// Display the picture onto the screen
	this->setPixmap(pixmap);
	// Emit signal with image description
	QString description;
	description += QLocale().toString( std::distance(dirList.begin(), current)+1 );
	description += QString("/") + QLocale().toString( dirList.size() );
	description += QString(" ") + current->dirName();
	emit changedImage(description);
}

void Ui::ImageSlide::append(QDir dir){
	// Add a new element to the list
	dirList.append(dir);
	// Make the current iterator point to that element
	current = dirList.end();
	--current;
	// Redisplay
	display();
}

void Ui::ImageSlide::next(){
	// Try to increment the iterator, otherwise leave it as it is
	if (++current == dirList.end()) --current;
	// Redisplay
	display();
}

void Ui::ImageSlide::prev(){
	// Try to decrement the iterator, otherwise leave it as it is
	if (current != dirList.begin()) --current;
	// Redisplay
	display();
}

void Ui::ImageSlide::reset(){
	// Try to decrement the iterator, otherwise leave it as it is
	dirList.clear();
	// Remove pixmap from label
	setPixmap(QPixmap());
}
