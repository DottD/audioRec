#ifndef DBInterpreter_hpp
#define DBInterpreter_hpp

#include <QObject>
#include <QVector>
#include <QFile>
#include <QStringList>
#include <QDataStream>
#include <QSharedPointer>
#include <AA/VoiceFeatures.hpp>
#include <UMF/functions.hpp>
#include <AA/Application.hpp>
#include <armadillo>

namespace Ui {
	class DBInterpreter;
}

#ifndef QVECTOR_DOUBLE_META_DEF
#define QVECTOR_DOUBLE_META_DEF
Q_DECLARE_METATYPE(QVector<double>)
#endif

/** Interpreter for the voices database.
 This class handles the reading and writing from and to files,
 creation of the database starting from voice features list. */
class Ui::DBInterpreter : public QObject {
	Q_OBJECT
	
	Q_PROPERTY(QVector<double> intraCoeffs READ getIntraCoeffs WRITE setIntraCoeffs);
	Q_PROPERTY(QVector<double> extraCoeffs READ getExtraCoeffs WRITE setExtraCoeffs);
	
	Q_PROPERTY(QSharedPointer<QStringList> intraFeats READ getIntraFeats WRITE setIntraFeats);
	Q_PROPERTY(QSharedPointer<QStringList> extraFeats READ getExtraFeats WRITE setExtraFeats);
	
	QVector<double> intraCoeffs, extraCoeffs;
	QSharedPointer<QStringList> intraFeats, extraFeats;
	
public:
	/** Constructor with parent option. */
	DBInterpreter(QObject* parent = Q_NULLPTR);
	
	/** Getter for intra coefficients. */
	QVector<double> getIntraCoeffs();
	
	/** Getter for extra coefficients. */
	QVector<double> getExtraCoeffs();
	
	/** Setter for intra coefficients. */
	void setIntraCoeffs(const QVector<double>& intraCoeffs);
	
	/** Setter for intra coefficients. */
	void setExtraCoeffs(const QVector<double>& extraCoeffs);
	
	/** Getter for intra file names. */
	QSharedPointer<QStringList> getIntraFeats();
	
	/** Getter for extra file names. */
	QSharedPointer<QStringList> getExtraFeats();
	
	/** Setter for intra file names. */
	void setIntraFeats(QSharedPointer<QStringList> intraFeats);
	
	/** Setter for extra file names. */
	void setExtraFeats(QSharedPointer<QStringList> extraFeats);
	
	/** Read database from .adb file. */
	bool readDBFromFile(QFile& file);
	
	/** Write database to .adb file. */
	bool writeDBToFile(QFile& file);
	
	public Q_SLOTS:
	/** Compute database from intra- and extra-speaker voices features lists. */
	Q_SLOT void computeDB();
	
Q_SIGNALS:
	Q_SIGNAL void histogramReady(QVector<double> x,
								 QVector<double> y);
	
	Q_SIGNAL void fittingReady(QVector<double> c,
							   QVector<double> x);
	
	Q_SIGNAL void databaseCreated();
};

#endif /* DBInterpreter_hpp */
