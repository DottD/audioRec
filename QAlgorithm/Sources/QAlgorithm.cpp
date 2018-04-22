#include <Headers/QAlgorithm.hpp>

quint32 QAlgorithm::print_counter = 1;

bool QAlgorithm::hasFinished(){
	return m_finished;
}

bool QAlgorithm::hasStarted(){
	return m_started;
}

void QAlgorithm::setFinished(const bool& value){
	this->m_finished = value;
	Q_EMIT justFinished();
}

void QAlgorithm::setStarted(const bool& value){
	this->m_started = value;
	Q_EMIT justStarted();
}

QAlgorithm::AlgorithmMap QAlgorithm::getAncestors(){
	return ancestors;
}

QAlgorithm::AlgorithmMap QAlgorithm::getDescendants(){
	return descendants;
}

QSharedPointer<QAlgorithm> QAlgorithm::findAncestor(QAlgorithm* ancestor){
	foreach(auto& shr_ancestor, getAncestors().keys()){
		if (shr_ancestor == ancestor){
			return shr_ancestor;
		}
	}
	return QSharedPointer<QAlgorithm>();
}

QSharedPointer<QAlgorithm> QAlgorithm::findAncestor(QSharedPointer<QAlgorithm> ancestor){
	return findAncestor(ancestor.data());
}

QSharedPointer<QAlgorithm> QAlgorithm::findDescendant(QAlgorithm* descendant){
	foreach(auto& shr_descendant, getDescendants().keys()){
		if (shr_descendant == descendant){
			return shr_descendant;
		}
	}
	return QSharedPointer<QAlgorithm>();
}

QSharedPointer<QAlgorithm> QAlgorithm::findDescendant(QSharedPointer<QAlgorithm> descendant){
	return findDescendant(descendant.data());
}

QSharedPointer<QAlgorithm> QAlgorithm::findSharedThis(){
	// Check among the descendants
	foreach(auto descendant, getDescendants().keys()){
		auto shr_this = descendant->findAncestor(this);
		if(!shr_this.isNull()) return shr_this;
	}
	// Otherwise check among the ancestors
	foreach(auto ancestor, getAncestors().keys()){
		auto shr_this = ancestor->findDescendant(this);
		if(!shr_this.isNull()) return shr_this;
	}
	// If nothing was found return null shared pointer
	return QSharedPointer<QAlgorithm>();
}

void QAlgorithm::setup(){
	// Prevent the QThreadPool to delete a parent instance
	setAutoDelete(false);
	// Detect the properties
	_Inputs.clear(); _Outputs.clear(); _Parameters.clear();
	const QMetaObject* MO = metaObject();
	for(int k = 0; k < MO->propertyCount(); ++k){
		QString PropName = MO->property(k).name();
		if(PropName.startsWith(QA_IN)){
			_Inputs << PropName;
		}else if(PropName.startsWith(QA_OUT)){
			_Outputs << PropName;
		}else if(PropName.startsWith(QA_PAR)){
			_Parameters << PropName;
		}
	}
	// Set connection inside the class
	connect(this, SIGNAL(justFinished()), SLOT(on_justFinished()));
}

QAlgorithm::QAlgorithm(QObject* parent) : QObject(parent), QRunnable() {
	qRegisterMetaType<QMapStringVar>();
	qRegisterMetaType<QMapStringString>();
	qRegisterMetaType<QAlgorithm*>();
}

void QAlgorithm::setParameters(const PropertyMap& parameters){
	// Scan the object properties
	for(const QString& PropName: parameters.keys()){
		QString FullName = QA_PAR + PropName;
		// Check whether this property is a parameter
		if(_Parameters.contains(FullName)){
			// Write the desired value in the property
			if (!setProperty(FullName.toStdString().c_str(), parameters.value(PropName))){
				qWarning("%s", (QString("Cannot set parameter ")+PropName).toStdString().c_str());
			}
		}
	}
}

bool QAlgorithm::getInput(QSharedPointer<QAlgorithm> parent){
	const QMetaObject* parentMO = parent->metaObject();
	for(int k = 0; k < parentMO->propertyCount(); ++k){
		QString parentPropName = parentMO->property(k).name();
		// Generate the corresponding child's property name
		QString parentPropBaseName = parentPropName.mid(strlen(QA_OUT));
		QString childPropName;
		if(getOITable().isEmpty() || !getOITable().contains(parentPropBaseName)){
			childPropName = QA_IN + parentPropBaseName;
		}else{
			childPropName = QA_IN + getOITable().value(parentPropBaseName);
		}
		if(parent->_Outputs.contains(parentPropName) && this->_Inputs.contains(childPropName)){
			// Assign to the child the value from the parent
			QVariant parentProp = parent->property(parentPropName.toStdString().c_str());
			if(!parentProp.isValid()){
				qWarning("%s", ("getInput(): " + parentPropName + " failed to read for " + parentMO->className()).toStdString().c_str() );
				return false;
			}else{
				if(!this->setProperty(childPropName.toStdString().c_str(), parentProp)){
					qWarning("%s", ("getInput(): " + childPropName + " failed to set for " + this->metaObject()->className()).toStdString().c_str() );
					return false;
				}
			}
		}
	}
	return true;
}

void QAlgorithm::run(){
	if(hasFinished() || hasStarted()) return; // weird case
	// Check if every ancestor has finished
	if(allInputsReady()){
		// Set this instance as started
		setStarted();
		// Perform the core part of the algorithm
		bool goodResult;
		try {
			goodResult = perform();
		} catch(std::exception& e) {
			qWarning() << "perform() failed with exception" << e.what();
			goodResult = false;
		}
		if(goodResult){
			// Mark this instance as finished
			setFinished();
		}else qWarning() << "perform() returned false";
	}
}

void QAlgorithm::save(QString path) {
	QFile file;
	// Use the home path as default
	if (path.isEmpty()) file.setFileName(QDir::home().filePath("algorithm.dat"));
	else file.setFileName(path);
	// Try to open the file in writing mode
	if (!file.open(QIODevice::WriteOnly)) {
		abort("Could not open file for writing");
	}
	// Assign a data stream to the file
	QDataStream ostream(&file);
	// Write the output properties to file
	const QMetaObject* obj = this->metaObject();
	for(int k = 0; k < obj->propertyCount(); k++){
		QMetaProperty prop = obj->property(k);
		QString propName = prop.name();
		if(propName.startsWith(QA_IN) || propName.startsWith(QA_OUT)){
			ostream << qMakePair(propName, prop.read(this));
		}
	}
	// Close the file
	file.close();
}

void QAlgorithm::load(QString path) {
	QFile file;
	// Use the home path as default
	if (path.isEmpty()) file.setFileName(QDir::home().filePath("algorithm.dat"));
	else file.setFileName(path);
	// Check if the file already exists
	if (!file.exists()) {
		Q_EMIT abort("The file does not exist");
		return;
	}
	// Try to open the file in writing mode
	if (!file.open(QIODevice::ReadOnly)) {
		Q_EMIT abort("Could not open file for writing");
	}
	// Assign a data stream to the file
	QDataStream istream(&file);
	// Take the metaobject related to this instance
	const QMetaObject* obj = this->metaObject();
	// Read a property at a time from the file
	while(!istream.atEnd()) {
		QString propName;
		QVariant propValue;
		istream >> propName >> propValue;
		// Search among the instance properties
		for(int k = 0; k < obj->propertyCount(); k++){
			QMetaProperty prop = obj->property(k);
			// If there is one with the same name as the one just loaded...
			if (QString(prop.name()) == propName) {
				// ...assign to the value to it
				if (!prop.write(this, propValue)){
					abort("Unable to write property value, report to the QAlgorithm developer");
				}
			}
		}
	}
	// Close the file
	file.close();
}

void QAlgorithm::abort(QString message){
	qWarning() << "Message" << message << "from" << QLocale().toString((quint64)QObject::sender()) <<
	"caught in" << QLocale().toString((quint64)this) << "of class" << this->metaObject()->className();
	Q_EMIT raiseError(message);
}

void QAlgorithm::on_justFinished(){
	auto shr_this = findSharedThis();
	if(!shr_this.isNull()){
		// Notify ancestors
		foreach(auto ancestor, getAncestors().keys()) ancestor->descendants[shr_this] = true;
		// Notify descendants, transfer output to and execute them
		foreach(auto descendant, getDescendants().keys()){
			descendant->ancestors[shr_this] = true;
			descendant->getInput(shr_this);
			if(!descendant->getKeepInput()){
				QAlgorithm::closeConnection(shr_this, descendant);
				// Set each input property to null
				// useful if input has been received with implicit sharing
				for(const QString& name: _Inputs) setProperty(name.toStdString().c_str(), QVariant());
			}
			if(!descendant->hasStarted()) QThreadPool::globalInstance()->start(descendant.data());
		}
	}else qInfo() << "This QAlgorithm has no connections.";
}

bool QAlgorithm::allInputsReady(){
	bool allReady = !getAncestors().values().contains(false);
	if(!allReady){
		// Run those not finished
		for(const auto& ancestor: getAncestors().keys(false)){
			// Only start processes not already started
			if(!ancestor->hasStarted()) future = QtConcurrent::run(ancestor.data()->run());
		}
	}
	return allReady;
}

void QAlgorithm::printGraph(const QString &path) {
	QFile dotFile;
	QString outFileName;
	if (path.isEmpty()) {
		dotFile.setFileName(QDir::home().absoluteFilePath("QAlgorithmTree.gv"));
		outFileName = QDir::home().absoluteFilePath("QAlgorithmTree.svg");
	}
	else {
		dotFile.setFileName(path);
		QStringList svgpath = path.split(".");
		svgpath.removeLast();
		outFileName = svgpath.join(".")+".svg";
	}
	if (!dotFile.open(QFile::WriteOnly)) abort("Cannot write graph to given file");
	else {
		QTextStream dot(&dotFile);
		QLocale nospace;
		nospace.setNumberOptions(QLocale::NumberOption::OmitGroupSeparator);
		dot << "digraph g{\n";
		auto flatMap = flattenTree();
		// Save node labels
		foreach(auto alg, flatMap.keys()){
			QString id = nospace.toString(quint64(alg.data()));
			QString idspace = QLocale().toString(quint64(alg.data()));
			QString name = alg->metaObject()->className();
			QString nickname = alg->objectName();
			QString dotstring = "var"+id.replace(" ", "")+"[label=\""+name+"\\nID "+idspace;
			if(!nickname.isEmpty()) dotstring += "\\nNick: "+nickname;
			dotstring += "\"];\n";
			dot << dotstring;
		}
		// Save connections between nodes
		foreach(auto parent, flatMap.keys()){
			QString parentName = "var" + nospace.toString(quint64(parent.data()));
			foreach(auto child, flatMap[parent]){
				QString childName = "var" + nospace.toString(quint64(child.data()));
				dot << parentName << " -> " << childName << "\n";
			}
		}
		dot << "}\n";
		dotFile.close();
		// Convert dot file to svg
		QStringList cmdArgs = {dotFile.fileName(), "-Tsvg", "-o", outFileName};
		int r = QProcess::execute("/usr/local/bin/circo", cmdArgs);
		if (r == -1){
			qWarning("%s", "The dot process crashed");
		}else if (r == -2){
			qWarning("%s", "Cannot start the dot process");
		}else{
			r = QProcess::execute("/bin/rm", {dotFile.fileName()});
			if (r == -1){
				qWarning("%s", "The rm process crashed");
			}else if (r == -2){
				qWarning("%s", "Cannot start the rm process");
			}
		}
	}
}

QMap<QSharedPointer<QAlgorithm>, QSet<QSharedPointer<QAlgorithm>>> QAlgorithm::flattenTree(QMap<QSharedPointer<QAlgorithm>, QSet<QSharedPointer<QAlgorithm>>> map) {
	// Search for a shared pointer attached to this instance
	auto keysList = map.keys();
	// Check if the function has already been applied to this
	auto shr_this_it = std::find_if(keysList.begin(), keysList.end(), [this](auto& ptr){return ptr == this;});
	if (shr_this_it == keysList.end()){
		// Otherwise link each descendant to this in the map
		auto shr_this = findSharedThis();
		if(shr_this.isNull()) qWarning("%s", "This instance has no properly set connection, flattenTree will not work");
		else {
			// Insert shr_this as new key in the map
			map[shr_this] = QSet<QSharedPointer<QAlgorithm>>();
			// Append each descendant to it
			for (auto descendant: getDescendants().keys()) map[shr_this] << descendant;
			// Recursive step: apply the function to each relative not yet scanned
			for (auto relative: getDescendants().keys()+getAncestors().keys()){
				if (!map.contains(relative)) map = relative->flattenTree(map);
			}
		}
	}else qWarning("%s", "flattenTree: possible loop");
	return map;
}

void QAlgorithm::setConnection(QSharedPointer<QAlgorithm> ancestor, QSharedPointer<QAlgorithm> descendant) {
	ancestor->descendants[descendant] = descendant->hasFinished();
	descendant->ancestors[ancestor] = ancestor->hasFinished();
	connect(ancestor.data(), SIGNAL(raiseError(QString)), descendant.data(), SLOT(abort(QString)), Qt::QueuedConnection);
	connect(descendant.data(), SIGNAL(raiseError(QString)), ancestor.data(), SLOT(abort(QString)), Qt::QueuedConnection);
}

void QAlgorithm::closeConnection(QSharedPointer<QAlgorithm> ancestor, QSharedPointer<QAlgorithm> descendant) {
	ancestor->descendants.remove(descendant);
	descendant->ancestors.remove(ancestor);
	disconnect(ancestor.data(), SIGNAL(raiseError(QString)), descendant.data(), SLOT(abort(QString)));
	disconnect(descendant.data(), SIGNAL(raiseError(QString)), ancestor.data(), SLOT(abort(QString)));
}

bool QAlgorithm::checkConnection(QSharedPointer<QAlgorithm> ancestor, QSharedPointer<QAlgorithm> descendant) {
	return ancestor->getDescendants().contains(descendant) && descendant->getAncestors().contains(ancestor);
}

bool QAlgorithm::isRemovableConnection(QSharedPointer<QAlgorithm> p1, QSharedPointer<QAlgorithm> p2) {
	if (QAlgorithm::checkConnection(p2, p1)){
		return (p2->getDescendants().count() == 1) && (p1->getAncestors().count() == 1);
	}else if (QAlgorithm::checkConnection(p1, p2)){
		return (p1->getDescendants().count() == 1) && (p2->getAncestors().count() == 1);
	}else return false;
}

QSharedPointer<QAlgorithm> operator>>(QSharedPointer<QAlgorithm> ancestor,
									  QSharedPointer<QAlgorithm> descendant){
	QAlgorithm::setConnection(ancestor, descendant);
	return descendant;
}

QSharedPointer<QAlgorithm> operator<<(QSharedPointer<QAlgorithm> descendant,
									  QSharedPointer<QAlgorithm> ancestor){
	ancestor >> descendant;
	return ancestor;
}

QDebug operator<<(QDebug debug, const QAlgorithm& c) {
	QDebugStateSaver saver(debug);
	const QMetaObject* obj = c.metaObject();
	
	// Write the algorithm class name
	debug << endl << "------------------------------" << obj->className() << "subclass of QAlgorithm" << endl;
	
	// Write the input properties of the algorithm
	debug << "Algorithm with input:" << endl;
	for(int k = 0; k < obj->propertyCount(); k++){
		QMetaProperty prop = obj->property(k);
		// Check whether the current property's name starts with "algin_"
		QString propName = prop.name();
		if(propName.startsWith(QA_IN)){
			propName.remove(QA_IN);
			debug << propName.rightJustified(30,' ',true) << "\t" << prop.read(&c) << endl;
		}
	}
	
	// Write the input properties of the algorithm
	debug << "Algorithm with parameters:" << endl;
	for(int k = 0; k < obj->propertyCount(); k++){
		QMetaProperty prop = obj->property(k);
		// Check whether the current property's name starts with "algin_"
		QString propName = prop.name();
		if(propName.startsWith(QA_PAR)){
			propName.remove(QA_PAR);
			debug << propName.rightJustified(30,' ',true) << "\t" << prop.read(&c) << endl;
		}
	}
	
	// Write the output properties of the algorithm
	debug << "Algorithm with output:" << endl;
	for(int k = 0; k < obj->propertyCount(); k++){
		QMetaProperty prop = obj->property(k);
		// Check whether the current property's name starts with "algout_"
		QString propName = prop.name();
		if(propName.startsWith(QA_OUT)){
			propName = propName.remove(QA_OUT);
			debug << propName.rightJustified(30,' ',true) << "\t" << prop.read(&c) << endl;
		}
	}
	
	debug << "------------------------------" << endl;
	
	return debug;
}
