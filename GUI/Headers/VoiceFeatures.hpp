#ifndef VoiceFeatures_hpp
#define VoiceFeatures_hpp

#include <QSharedPointer>
#include <QVector>
#include <QFile>
#include <QDataStream>
#include <QtCharts>
#include <armadillo>

namespace Ui {
	class VoiceFeatures;
}

/** Handles I/O and statistical operations on features.
 This class reads and writes features to file, computes covariance
 and the distance between two feature vectors.
 */
class Ui::VoiceFeatures : public QObject {
	
	Q_OBJECT
	
private:
	/** Array of array of features. */
	QSharedPointer<QVector<QVector<double>>> features;
	
	/** Convert the features to an arma::mat.
	 This function returns the arma::mat corresponding to the features stored in
	 the instance. The number of rows equals the size of features, and the number
	 of columns corresponds to the size of each inner vector.
	 @return Matrix corresponding to the stored features.
	 */
	arma::mat toArmaMat() const;
	
public:
	/** Empty constructor. */
	VoiceFeatures(QObject* parent = Q_NULLPTR) : QObject(parent) {};
	
	/** Constructor with initial features vector.
	 The data contained in the given vector is not copied.
	 @sa setFeatures 
	 */
	VoiceFeatures(QSharedPointer<QVector<QVector<double>>> features,
				  QObject* parent = Q_NULLPTR) : QObject(parent) {setFeatures(features);};
	
	/** Constructor with initial features vector.
	 The data contained in the given vector is copied.
	 @sa setFeatures
	 */
	VoiceFeatures(QVector<QVector<double>> features,
				  QObject* parent = Q_NULLPTR) : QObject(parent) {setFeatures(features);};
	
	/** Reads from file.
	 This function reads a binary file, regardless of its extension.
	 @param[in] dir Full path to the file to be read.
	 @return Whether the operation has been successfull.
	 @sa writeToFile
	 */
	bool readFromFile(QFile& file);
	
	/** Writes to file. 
	 This function writes a binary file with the feature vector associated
	 with the class instance. Regardless of the given extension, the new file
	 will have a ".feat" extension.
	 @param[in] dir Full path to the file to be created.
	 @return Whether the operation has been successfull.
	 @sa readFromFile
	 */
	bool writeToFile(QFile& file);
	
	/** Compute the distance with another feature vector.
	 This function comptues the Mahalanobis distance between the stored feature vector
	 and the given one. 
	 @param[in] features Feature vector to be compared with the stored one.
	 @return Mahalnobis distance between feature vectors.
	 */
	double distance(const VoiceFeatures& features);
	
	/** Setter for the features attribute.
	 This setter does not copy the values.
	 */
	void setFeatures(QSharedPointer<QVector<QVector<double>>> features);
	
	/** Setter for the features attribute.
	 This setter copys the values.
	 */
	void setFeatures(const QVector<QVector<double>>& features);
};

#endif /* VoiceFeatures_hpp */
