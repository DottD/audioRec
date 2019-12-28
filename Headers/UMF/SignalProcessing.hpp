#ifndef SignalProcessing_hpp
#define SignalProcessing_hpp

#include <QAlgorithm.hpp>
#include <TSpectrum.h>
#include <TMath.h>
#include <alglib/fasttransforms.h>
#include <armadillo>

namespace UMF {
	class ReduceChannels : public QAlgorithm {
		
		Q_OBJECT
		
	public:
		enum channels{
			interleaved, 	// C1,C2,C3,C1,C2,C3,...
			separated		// C1,...,C1,C2,...,C2,C3,...,C3
		};
		Q_ENUM(channels)
		
		enum operation{
			average
		};
		Q_ENUM(operation)
		
		QA_INPUT(QVector<double>, Signal)
		QA_PARAMETER(int, ChannelsArrangement, interleaved)
		QA_PARAMETER(int, Operation, average)
		QA_PARAMETER(int, NumberChannels, 2)
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(ReduceChannels)
		
	public:
		void run();
	};
	
	class Windowing : public QAlgorithm {
		
		Q_OBJECT
		
	public:
		enum function{
			hann
		};
		Q_ENUM(function)
		
		QA_INPUT(QVector<double>, Signal)
		QA_PARAMETER(int, Type, hann)
		QA_PARAMETER(int, Length, 5)
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(Windowing)
		
	private:
		QVector<double> window;
		
	public:
		void run();
		
		void init();
	};
	
	class ArrayPad : public QAlgorithm {
		
		Q_OBJECT
		
	public:
		enum border_type {
			constant,		// iiiiii|abcdefgh|iiiiiii with some specified i
			replicate,		// aaaaaa|abcdefgh|hhhhhhh
			reflect,		// fedcba|abcdefgh|hgfedcb
			wrap,			// cdefgh|abcdefgh|abcdefg
			reflect_101		// gfedcb|abcdefgh|gfedcba
		};
		Q_ENUM(border_type)
		
		/** Radius of the gaussian filter. */
		QA_PARAMETER(int, Radius, 5)
		/** Out of border interpolation mode. */
		QA_PARAMETER(int, BorderType, constant)
		/** Vector to be filtered. */
		QA_INPUT(QVector<double>, Signal)
		/** Filtered vector. */
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(ArrayPad)
		
	public:
		void run();
		
		void init();
		
		/** Compute interpolation coordinates.
		 The function computes and returns the coordinate of a donor pixel corresponding
		 to the specified extrapolated pixel when using the specified extrapolation border mode.
		 
		 If the border type is constant, then -1 is returned.
		 
		 There is no check whether the final result is out of bounds: it may happen
		 that a pos too much out of bounds leads to negative returning value.
		 */
		int borderInterpolate(const int& pos,
							  const int& len,
							  const border_type& bd);
	};
	
	class GaussianFilter : public QAlgorithm {
		
		Q_OBJECT
		
		/** Radius of the gaussian filter. */
		QA_PARAMETER(int, Radius, 5)
		/** Out of border interpolation mode. */
		QA_PARAMETER(int, BorderType, ArrayPad::constant)
		/** Vector to be filtered. */
		QA_INPUT(QVector<double>, Signal)
		/** Filtered vector. */
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(GaussianFilter)
		
	public:
		void init();
		
		void run();
		
	private:
		arma::vec kernel;
	};
	
	class SpectrumMagnitude : public QAlgorithm {
		
		Q_OBJECT
		
		QA_INPUT(QVector<double>, Signal)
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(SpectrumMagnitude)
		
	public:
		void run();
	};
	
	class SpectrumRemoveBackground : public QAlgorithm {
		
		Q_OBJECT
		
	public:
		enum filterOrder {
			kBackOrder2 =0,
			kBackOrder4 =1,
			kBackOrder6 =2,
			kBackOrder8 =3
		};
		enum windowDirection {
			kBackIncreasingWindow =0,
			kBackDecreasingWindow =1
		};
		enum smoothingWindow {
			kBackSmoothing3 =3,
			kBackSmoothing5 =5,
			kBackSmoothing7 =7,
			kBackSmoothing9 =9,
			kBackSmoothing11 =11,
			kBackSmoothing13 =13,
			kBackSmoothing15 =15
		};
		Q_ENUM(filterOrder)
		Q_ENUM(windowDirection)
		Q_ENUM(smoothingWindow)
		
		QA_INPUT(QVector<double>, Signal)
		QA_PARAMETER(int, NumberIterations, 6)
		QA_PARAMETER(int, Direction, TSpectrum::kBackIncreasingWindow)
		QA_PARAMETER(int, FilterOrder, TSpectrum::kBackOrder2)
		QA_PARAMETER(bool, Smoothing, kFALSE)
		QA_PARAMETER(int, SmoothWindow, TSpectrum::kBackSmoothing3)
		QA_PARAMETER(bool, Compton, kFALSE)
		QA_OUTPUT(QVector<double>, Signal)
		
		QA_CTOR_INHERIT
		QA_IMPL_CREATE(SpectrumRemoveBackground)
		
	public:
		void run();
		
		void init();
	};
}

#endif /* SignalProcessing_hpp */
