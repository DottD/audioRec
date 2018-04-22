#ifndef QAlgorithm_hpp
#define QAlgorithm_hpp

#include <QObject>
#include <QPointer>
#include <QRunnable>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaMethod>
#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDataStream>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QTimer>
#include <QProcess>
#include <iomanip>

#ifndef QA_IN
#define QA_IN "algin_"
#endif
#ifndef QA_OUT
#define QA_OUT "algout_"
#endif
#ifndef QA_PAR
#define QA_PAR "par_"
#endif

/** Defines an input property for the algorithm.
 @param[in] type Type of the input property.
 #param[in] name Name of the input property.
 */
#ifndef QA_INPUT
#define QA_INPUT(type, name) \
Q_PROPERTY(type algin_##name MEMBER m_algin_##name READ getIn##name WRITE setIn##name)\
private:\
type m_algin_##name;\
public:\
void setIn##name (type tmp){\
this->m_algin_##name = tmp;\
}\
type getIn##name (){\
return this->m_algin_##name;\
}
#endif

#ifndef QA_INPUT_LIST
#define QA_INPUT_LIST(type, name) \
Q_PROPERTY(type algin_##name MEMBER m_algin_##name WRITE setIn##name)\
private:\
type m_algin_##name;\
QList<type> m_listin_##name;\
public:\
void setIn##name (type tmp){\
this->m_algin_##name = tmp;\
this->m_listin_##name << tmp;\
}\
QList<type> getIn##name (){\
return this->m_listin_##name;\
}
#endif

#ifndef QA_INPUT_VEC
#define QA_INPUT_VEC(type, name) \
Q_PROPERTY(type algin_##name MEMBER m_algin_##name WRITE setIn##name)\
private:\
type m_algin_##name;\
QVector<type> m_vecin_##name;\
public:\
void setIn##name (type tmp){\
this->m_algin_##name = tmp;\
this->m_vecin_##name << tmp;\
}\
QVector<type> getIn##name (){\
return this->m_vecin_##name;\
}
#endif


/** Defines an output property for the algorithm.
 @param[in] type Type of the output property.
 #param[in] name Name of the output property.
 */
#ifndef QA_OUTPUT
#define QA_OUTPUT(type, name) \
Q_PROPERTY(type algout_##name MEMBER m_algout_##name READ getOut##name WRITE setOut##name)\
private:\
type m_algout_##name;\
protected:\
void setOut##name (type tmp){\
this->m_algout_##name = tmp;\
}\
public:\
type getOut##name (){\
return this->m_algout_##name;\
}
#endif

/** Defines a parameter for the algorithm.
 @param[in] type Type of the parameter.
 #param[in] name Name of the parameter.
 #param[in] type_def Default value of the parameter.
 */
#ifndef QA_PARAMETER
#define QA_PARAMETER(type, name, type_def) \
Q_PROPERTY(type par_##name READ get##name WRITE set##name)\
private:\
type par_##name = type_def;\
public:\
void set##name (type tmp){\
this->par_##name = tmp;\
}\
type get##name (){\
return this->par_##name;\
}
#endif

/** Make subclass inherit QAlgorithm's default constructor.
 This macro is to be used in each and every subclass to make it inherit
 the default constructor. This is useful if no further customization is
 needed, and mandatory is no other costructor is defined.
 */
#ifndef QA_CTOR_INHERIT
#define QA_CTOR_INHERIT 		\
public:							\
using QAlgorithm::QAlgorithm;	\
protected:						\
using QAlgorithm::setup;
#endif

/** Define a subclass' create static method.
 This macro is useful for a subclass, but not mandatory, to give it
 a predefined create static method, that creates a new instance of that
 subclass and returns a QSharedPointer to it.
 @param[in] ClassName Name of the subclass which inherit from QAlgorithm.
 */
#ifndef QA_IMPL_CREATE
#define QA_IMPL_CREATE(ClassName)															\
public:																						\
static inline QSharedPointer<ClassName> create(PropertyMap parameters = PropertyMap(),		\
												QObject* parent = Q_NULLPTR){				\
auto ptr = QSharedPointer<ClassName>(new ClassName(parent), &QObject::deleteLater);			\
ptr->setup();																				\
if(!parameters.isEmpty()){																	\
	ptr->setParameters(parameters);															\
}																							\
return ptr;																					\
}
#endif

typedef QMap<QString, QVariant> QMapStringVar;

#ifndef QA_MAP_STR_VAR
#define QA_MAP_STR_VAR
Q_DECLARE_METATYPE(QMapStringVar)
#endif

typedef QMap<QString, QString> QMapStringString;

#ifndef QA_MAP_STR_STR
#define QA_MAP_STR_STR
Q_DECLARE_METATYPE(QMapStringString)
#endif

/*
 Algorithms that accepts more than one ancestors must implement
 */

class QAlgorithm : public QObject, public QRunnable {
	
	Q_OBJECT
	
	/** Establishes when the input must be released.
	 If this property is set to true (default) the input is not freed until this instance
	 is destroyed. Otherwise the input values are set to QVariant() as soon as the
	 computation ends, if no other instances share that input.
	 */
	QA_PARAMETER(bool, KeepInput, true)
	
	/** Table of property names for setting the input.
	 When the table is empty the child input name must be the same as the parent output.
	 Otherwise this table is used as a mapping between the parent's property name and the
	 corresponding child's property name.
	 */
	QA_PARAMETER(QMapStringString, OITable, QMapStringString())
	
public:
	typedef QMap<QSharedPointer<QAlgorithm>, bool> AlgorithmMap;
	typedef QMapStringVar PropertyMap;
	
	static quint32 print_counter;
	
private:
	Q_PROPERTY(bool finished MEMBER m_finished READ hasFinished WRITE setFinished NOTIFY justFinished())
	Q_PROPERTY(bool started MEMBER m_started READ hasStarted WRITE setStarted NOTIFY justStarted())
	Q_PROPERTY(AlgorithmMap ancestors READ getAncestors)
	Q_PROPERTY(AlgorithmMap descendants READ getDescendants)
	
	AlgorithmMap ancestors, descendants;
	bool m_finished = false, m_started = false;
	QSet<QString> _Inputs, _Outputs, _Parameters;
	
	/** Result of this instance's computation. */
	QFuture<void> result;
	
public:
	AlgorithmMap getAncestors();
	AlgorithmMap getDescendants();
	
	virtual bool getInput(QSharedPointer<QAlgorithm> parent);
protected:
	
	/** Set of instructions to set up the algorithm.
	 This is automatically called by the create() function,
	 otherwise, if you want to manually create a subclass
	 instance, you should call setup() by yourself.
	 */
	void setup();
	
	/** Find an ancestor.
	 Scans the ancestors of this algorithm looking for the given ancestor.
	 If nothing is found returns a null shared pointer.
	 @param[in] ancestor Pointer to the ancestor to be found.
	 @return Shared pointer to the requested ancestor.
	 @sa findDescendant, findSharedThis
	 */
	QSharedPointer<QAlgorithm> findAncestor(QAlgorithm* ancestor);
	
	/** Find an ancestor.
	 This is an overloaded method.
	 @param[in] ancestor Shared pointer to the ancestor to be found.
	 @return Shared pointer to the requested ancestor.
	 @sa findDescendant, findSharedThis
	 */
	QSharedPointer<QAlgorithm> findAncestor(QSharedPointer<QAlgorithm> ancestor);
	
	/** Find a descendant.
	 Scans the descendants of this algorithm looking for the given descendant.
	 If nothing is found returns a null shared pointer.
	 @param[in] descendant Pointer to the descendant to be found.
	 @return Shared pointer to the requested descendant.
	 @sa findAncestor, findSharedThis
	 */
	QSharedPointer<QAlgorithm> findDescendant(QAlgorithm* descendant);
	
	/** Find a descendant.
	 This is an overloaded method.
	 @param[in] descendant Shared pointer to the descendant to be found.
	 @return Shared pointer to the requested descendant.
	 @sa findAncestor, findSharedThis
	 */
	QSharedPointer<QAlgorithm> findDescendant(QSharedPointer<QAlgorithm> descendant);
	
	/** Find a shared pointer to this instance.
	 Scans the descendants and ancestors of this algorithm looking
	 for a shared pointer to this instance. The first pointer found
	 is returned, if any.
	 If nothing is found returns a null shared pointer, no shared
	 pointer is created by this function.
	 @return Shared pointer to this instance.
	 @sa findAncestor, findDescendant
	 */
	QSharedPointer<QAlgorithm> findSharedThis();
	
public:
	QAlgorithm(QObject* parent = Q_NULLPTR);
	
	virtual void setParameters(const PropertyMap& parameters);
	
	virtual bool perform() = 0;
	
	bool hasFinished();
	void setFinished(const bool& value = true);
	bool hasStarted();
	void setStarted(const bool& value = true);
	
	public Q_SLOTS:
	/** Run the algorithm.
	 This pure virtual function must be reimplemented in each subclass;
	 indeed each subclass is a different algorithm, with its own rules.
	 A good process method should take into account the input given by
	 setInput(), and should emit the finished() signal at the end
	 of the computation. This will make possible connecting two algorithms
	 together, without the need to set up each one and instruct them
	 how to run.
	 @sa setInput, finished
	 */
	Q_SLOT void run();
	
	/** Save the results to files. */
	Q_SLOT virtual void save(QString path = QString());
	
	/** Load the results from files. */
	Q_SLOT virtual void load(QString path = QString());
	
	/** Invalidate the process and raise the specified error. */
	Q_SLOT void abort(QString message = "Unknown Error");
	
	Q_SLOT bool allInputsReady();
	
	Q_SLOT void on_justFinished();
		
Q_SIGNALS:
	/** Signal emitted when every children have completed their tasks.
	 Each subclass must call Algorithm::run() at the end of the computation,
	 otherwise this signal is not emitted. The argument of the signal
	 is a pointer to the emitting algorithm instance; it can be used
	 to get the results from other threads.
	 @sa run()
	 */
	Q_SIGNAL void justFinished();
	Q_SIGNAL void justStarted();
	
	/** Signal emitted whenever an error occurs.
	 This class emits the raiseError signal whenever an internal error
	 occurs. This behaviour replaces the traditional exception throwing,
	 allowing the user to easily handle errors in Qt event loop.
	 Indeed each subclass should use the same convention, and every caller
	 should handle and possibly retransmit the raiseError signal.
	 @param[in] message The error description.
	 */
	Q_SIGNAL void raiseError(QString message);
	
public:
	void printGraph(const QString& path = QString());
	
	QMap<QSharedPointer<QAlgorithm>, QSet<QSharedPointer<QAlgorithm>>> flattenTree(QMap<QSharedPointer<QAlgorithm>, QSet<QSharedPointer<QAlgorithm>>> map = QMap<QSharedPointer<QAlgorithm>, QSet<QSharedPointer<QAlgorithm>>>());
	
	static void setConnection(QSharedPointer<QAlgorithm> ancestor,
							  QSharedPointer<QAlgorithm> descendant);
	static void closeConnection(QSharedPointer<QAlgorithm> ancestor,
							  QSharedPointer<QAlgorithm> descendant);
	static bool checkConnection(QSharedPointer<QAlgorithm> ancestor,
							  QSharedPointer<QAlgorithm> descendant);
	
	static bool isRemovableConnection(QSharedPointer<QAlgorithm> p1,
									  QSharedPointer<QAlgorithm> p2);
};

#ifndef QA_QALGORITHM_PTR
#define QA_QALGORITHM_PTR
Q_DECLARE_METATYPE(QAlgorithm*)
#endif

/** Creates a connection in a friendly way.
 */
QSharedPointer<QAlgorithm> operator>>(QSharedPointer<QAlgorithm> ancestor,
									  QSharedPointer<QAlgorithm> descendant);

/** Creates a connection in a friendly way.
 */
QSharedPointer<QAlgorithm> operator<<(QSharedPointer<QAlgorithm> descendant,
									  QSharedPointer<QAlgorithm> ancestor);

/** Send debugging information to a QDebug instance.
 */
QDebug operator<<(QDebug debug, const QAlgorithm& c);

#endif /* QAlgorithm_hpp */
