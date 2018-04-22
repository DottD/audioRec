#ifndef QAlgorithmEnvelope_hpp
#define QAlgorithmEnvelope_hpp

#include "QAlgorithm.hpp"

class QAlgorithmEnvelope : public QAlgorithm {
	
	Q_OBJECT
	
public:
	/** Constructs a new envelope for the given algorithms.
	 This function uses the given algorithms to build an envelope
	 upon them. Every linkage is performed silently.
	 */
	QAlgorithmEnvelope(QList<QSharedPointer<QAlgorithm>> nodes);
	
	/** Improve a tree of algorithms creating envelopes.
	 This function scans the tree, which the given leaf belongs to,
	 and creates envelopes around the removable connections found.
	 @param[in] leaf An algorithm of the tree to be simplified.
	 */
	static void improveTree(QAlgorithm* leaf);
	
	public Q_SLOTS:
	/** Run the given algorithms one by one.
	 At the end of the computation the signal justFinished is emitted
	 for the last algorithm in the list. This function silently makes
	 any enveloped algorithm run and pass the outputs to the next
	 algorithm; in this scheme is included also the last algorithm, that
	 will signal justFinished and the Qt signal-slot mechanism will make
	 the following process start.
	 At the end of this function, this envelope is marked for deletion,
	 thus you don't need to worry about that, and anything is restored
	 as before its creation.
	 */
	Q_SLOT bool perform();
	
private:
	/** Pointers to the enveloped algorithms. */
	QList<QSharedPointer<QAlgorithm>> nodes;
	
	/** Make the first node take input from the previous algorithm.
	 Redefinition of QAlgorithm::getInput.
	 @param[in] parent Algorithm to get the input from.
	 */
	bool getInput(QSharedPointer<QAlgorithm> parent);
};

#endif /* QAlgorithmEnvelope_hpp */
