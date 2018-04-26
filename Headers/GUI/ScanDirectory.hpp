#ifndef ScanDirectory_hpp
#define ScanDirectory_hpp

#include <QAlgorithm.hpp>
#include <QDir>
#include <QDirIterator>

namespace GUI {
	class ScanDirectory;
}

class GUI::ScanDirectory : public QAlgorithm {
	
	Q_OBJECT
	
	QA_PARAMETER(QString, Folder, QString())
	QA_PARAMETER(bool, Recursive, true)
	QA_PARAMETER(QStringList, Extensions, QStringList())
	QA_OUTPUT(QList<QList<QString>>, Content)
	
	QA_CTOR_INHERIT
	QA_IMPL_CREATE(ScanDirectory)
	
public:
	void run();
};

#endif /* ScanDirectory_hpp */
