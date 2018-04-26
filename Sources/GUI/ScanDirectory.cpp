#include <GUI/ScanDirectory.hpp>

void GUI::ScanDirectory::run(){
	// Define the top directory
	QDir dir(getFolder());
	// Set filters to retrieve the correct files
	dir.setFilter(QDir::Filter::NoDotAndDotDot|QDir::Filter::NoSymLinks|QDir::Filter::Readable|QDir::Filter::Dirs);
	// Recursively extract a list of every processable file in the given directory
	QDirIterator dirIt(dir, QDirIterator::Subdirectories);
	// Change dir to find files
	dir.setFilter(QDir::Filter::Files|QDir::Filter::Readable);
	// Declare the output list
	QList<QList<QString>> out;
	// Choose between recursive and non-recursive scan
	if(getRecursive()){
		// Skip files directly included in the top directory
		// Scan found directories
		while (dirIt.hasNext()) {
			// Declare the output sub-list
			QList<QString> subOut;
			// Reassign dir pointing to the current directory
			dir.setPath(dirIt.next());
			// Assign the files contained in the current directory to a temporary list
			subOut << dir.entryList(getExtensions());
			// If the current list is empty add nothing
			if(!subOut.isEmpty()){
				// Change every path contained there to an absolute one
				for(QString& file: subOut) file = dir.absoluteFilePath(file);
				// Append the current list of strings to the general entries list
				out << subOut;
			}
		}
	} else {
		// Declare the output sub-list
		QList<QString> subOut;
		// Reassign dir pointing to the current directory
		dir.setPath(getFolder());
		// Assign the files contained in the current directory to a temporary list
		subOut << dir.entryList(getExtensions());
		// If the current list is empty add nothing
		if(!subOut.isEmpty()){
			// Change every path contained there to an absolute one
			for(QString& file: subOut) file = dir.absoluteFilePath(file);
			// Append the current list of strings to the general entries list
			out << subOut;
		}
	}
	// Set output
	if(!out.isEmpty()){
		setOutContent(out);
	}
}
