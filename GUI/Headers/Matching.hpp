#ifndef Matching_hpp
#define Matching_hpp

#include <QObject>
#include <QVector>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QDir>
#include <QSharedPointer>
#include <Headers/AudioProcess.hpp>
#include <Headers/VoiceFeatures.hpp>
#include <alglib/integration.h>
#include <Headers/functions.hpp>

#ifndef QVECTOR_DOUBLE_META_DEF
#define QVECTOR_DOUBLE_META_DEF
Q_DECLARE_METATYPE(QList<QVariant>)
#endif

namespace Ui {
	class Matching;
}

class Ui::Matching : public QObject {
	Q_OBJECT
	
	Q_PROPERTY(QVector<double> intraCoeffs READ getIntraCoeffs WRITE setIntraCoeffs);
	Q_PROPERTY(QVector<double> extraCoeffs READ getExtraCoeffs WRITE setExtraCoeffs);
	Q_PROPERTY(QStringList culpritFiles READ getCulpritFiles WRITE setCulpritFiles);
	Q_PROPERTY(QStringList suspectsFiles READ getSuspectsFiles WRITE setSuspectsFiles);
	
	QVector<double> intraCoeffs, extraCoeffs;
	QStringList culpritFiles, suspectsFiles;
	
	const double minDistance = 0;
	const double maxDistance = 100;
	
	static void eval(double x, double xminusa, double xminusb, double& y, void* ptr);
	
public:
	/** Constructor with parent option. */
	Matching(QObject* parent = Q_NULLPTR);
	
	/** Getter for intra coefficients. */
	QVector<double> getIntraCoeffs();
	
	/** Getter for extra coefficients. */
	QVector<double> getExtraCoeffs();
	
	/** Getter for culprit files. */
	QStringList getCulpritFiles();
	
	/** Getter for suspects files. */
	QStringList getSuspectsFiles();
	
	/** Setter for intra coefficients. */
	void setIntraCoeffs(const QVector<double>& intraCoeffs);
	
	/** Setter for intra coefficients. */
	void setExtraCoeffs(const QVector<double>& extraCoeffs);
	
	/** Setter for culprit files. */
	void setCulpritFiles(const QStringList& culpritFiles);
	
	/** Setter for suspects files. */
	void setSuspectsFiles(const QStringList& suspectsFiles);
	
	public Q_SLOTS:
	/** Perform the matching with the requested features. */
	Q_SLOT void match();
	
Q_SIGNALS:
	/** Signal emitted when a new score is available. */
	void newScore(QList<QVariant>);
	
	/** Signal emitted when a new distance is computed. */
	void newDistance(double);
};

#endif /* Matching_hpp */
