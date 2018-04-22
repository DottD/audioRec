#include "../Headers/QAlgorithmEnvelope.hpp"

QAlgorithmEnvelope::QAlgorithmEnvelope(QList<QSharedPointer<QAlgorithm>> _nodes) {
	// Prevent the QThreadPool to delete a parent instance
	setAutoDelete(false);
	// Store the given list
	nodes = _nodes;
	// Check if the given nodes are removable, otherwise raise a warning
	for(int k = 0; k < nodes.count()-1; k++){
		if (!QAlgorithm::isRemovableConnection(nodes.at(k), nodes.at(k+1))){
			qWarning("%s", "QAlgorithmEnvelope, not removable connection found");
		}else{
			// If removable, remove the connection
			QAlgorithm::closeConnection(nodes.at(k), nodes.at(k+1));
		}
	}
	// Remove the connection at the beginning, and connect the envelope
	QSharedPointer<QAlgorithm> envelope(this);
	foreach(auto& ancestor, nodes.first()->getAncestors().keys()){
		QAlgorithm::closeConnection(ancestor, nodes.first());
		QAlgorithm::setConnection(ancestor, envelope);
	}
}

bool QAlgorithmEnvelope::perform() {
	for (int k = 0; k < nodes.count()-1; ++k){
		nodes.at(k)->run();
		nodes.at(k+1)->getInput(nodes.at(k));
	}
	nodes.last()->run();
	// We now restore the connections among the enveloped algorithms
	for(int k = 0; k < nodes.count()-1; ++k){
		QAlgorithm::setConnection(nodes.at(k), nodes.at(k+1));
	}
	// The connections of this last algorithm have not been cut,
	// so, after its execution, the computation go past the envelope
	// and go on normally.
	// We only have to remove the envelope: since it is only connected
	// to its ancestors by means of a single shared pointer, we only need
	// to cut references from them. In the mean while we restore the
	// references to the enveloped algorithms.
	foreach(auto& ancestor, this->getAncestors().keys()){
		foreach(auto& envelope, ancestor->getDescendants().keys()){
			if (envelope == this){
				QAlgorithm::closeConnection(ancestor, envelope);
				QAlgorithm::setConnection(ancestor, nodes.first());
			}
		}
	}
	return true;
}

bool QAlgorithmEnvelope::getInput(QSharedPointer<QAlgorithm> parent) {
	qDebug() << "Executing getInput from subclass!!";
	return nodes.first()->getInput(parent);
}

void QAlgorithmEnvelope::improveTree(QAlgorithm* leaf) {
	auto flatMap = leaf->flattenTree();
	QMap<QSharedPointer<QAlgorithm>, QList<QSharedPointer<QAlgorithm>>> replacements;
	foreach (auto ptr, flatMap.uniqueKeys()){
		foreach(auto child, flatMap[ptr]){
			if (QAlgorithm::isRemovableConnection(ptr, child)){
				// Insert a new replacement (assuming uniqueness of flat map keys)
				replacements[ptr] << child;
			}
		}
	}
	// From the set of removable connection, link the pairs that form a removable connection all together
	bool some_changes = true;
	while(some_changes){
		some_changes = false;
		foreach(auto p1, replacements.keys()){
			// Take the last element in the list of removable connections
			auto p2 = replacements[p1].last();
			// If it has its own removable connections, add them to p1's
			if (replacements.contains(p2)){
				replacements[p1] += replacements.take(p2);
				some_changes = true;
			}
		}
	}
	// Create the envelopes for each replacement
	foreach(QList<QSharedPointer<QAlgorithm>> nodes, replacements.values()){
		new QAlgorithmEnvelope(nodes);
		// automatically handles the envelope linkage and destruction
	}
}
