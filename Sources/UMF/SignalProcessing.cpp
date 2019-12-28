#include <UMF/SignalProcessing.hpp>

using namespace arma;

void UMF::ReduceChannels::run(){
	auto signalLength = getInSignal().size()/getNumberChannels();
	if (signalLength * getNumberChannels() != getInSignal().size())
		qInfo() << "Some sample will be discarded reducing channels";
	QVector<double> out;
	out.reserve(signalLength);
	double factor = 1.0 / double(getNumberChannels());
	switch (getChannelsArrangement()) {
		case interleaved:
			for(auto c1 = getInSignal().constBegin(); c1 != getInSignal().constEnd(); c1 += getNumberChannels())
				out << std::reduce(c1, c1+getNumberChannels(), 0.0, std::plus<>{}) * factor;
			break;
		case separated:
			for (auto s = getInSignal().constBegin(); s != getInSignal().constBegin()+signalLength; ++s) {
				double sum = 0.0;
				for(auto c = s; c != getInSignal().constEnd(); c += signalLength)
					sum += *c;
				out << sum * factor;
			}
			break;
	}
	out.squeeze();
	setOutSignal(out);
}

void UMF::Windowing::run(){
	if (getLength() != getInSignal().size()){
		abort("Length (" + QLocale().toString(getLength()) + ") and input signal size (" + QLocale().toString(getInSignal().size()) + ") must be equal");
		return;
	}
	auto signal = getInMoveSignal();
	std::transform(signal.begin(), signal.end(), window.constBegin(), signal.begin(), std::multiplies<>());
	setOutSignal(std::move(signal));
	// Clear input
	setInSignal(QVector<double>());
}

void UMF::Windowing::init(){
	QAlgorithm::init();
	window.reserve(getLength());
	int k = 0;
	switch (getType()) {
		case hann:
			double arg = TMath::TwoPi() / (getLength() - 1);
			std::generate_n(std::back_inserter(window), getLength(), [&arg, &k](){
				return 0.5 - 0.5 * TMath::Cos(arg*(k++));
			});
			break;
	}
	window.squeeze();
}

void UMF::ArrayPad::run(){
	// Checks
	if (getBorderType() != constant && getInSignal().size() <= getRadius()){
		qInfo() << "Signal length insufficient for selected border, fall back to constant case";
		setBorderType(constant);
	}
	// Construct an array with 2*m more element than the input one
	// Fill the central part with the input array
	auto inputSize = getInSignal().size();
	auto output = getInMoveSignal();
	output.resize(inputSize + 2 * getRadius());
	std::move_backward(output.begin(), output.begin()+inputSize, output.end()-getRadius());
	// Clear input signal
	setInSignal(QVector<double>());
	// Fill the output array with correct values
	auto left_begin = output.begin();
	auto left_end = output.begin()+getRadius();
	auto right_begin = output.end()-getRadius();
	auto right_end = output.end();
	switch (getBorderType()) {
		case constant:
			std::fill(left_begin, left_end, 0);
			std::fill(right_begin, right_end, 0);
			break;
		case replicate:
			std::fill(left_begin, left_end, *left_end);
			std::fill(right_begin, right_end, *(right_begin-1));
			break;
		case reflect:
			std::copy(left_end, left_end+getRadius(), left_begin);
			std::reverse(left_begin, left_end);
			std::copy(right_begin-getRadius(), right_begin, right_begin);
			std::reverse(right_begin, right_end);
			break;
		case wrap:
			std::copy(right_begin-getRadius(), right_begin, left_begin);
			std::copy(left_end, left_end+getRadius(), right_begin);
			break;
		case reflect_101:
			std::copy(left_end+1, left_end+getRadius()+1, left_begin);
			std::reverse(left_begin, left_end);
			std::copy(right_begin-getRadius()-1, right_begin-1, right_begin);
			std::reverse(right_begin, right_end);
			break;
		default:
			abort("Interpolation mode not recognized");
			break;
	}
	setOutSignal(output);
}

int UMF::ArrayPad::borderInterpolate(const int& pos,
									 const int& len,
									 const border_type& bd){
	switch (bd) {
		case border_type::constant:
			if (pos < 0 || pos > len-1) return -1; else return pos;
			break;
		case border_type::replicate:
			if (pos < 0) return 0; else if (pos > len-1) return len-1; else return pos;
			break;
		case border_type::reflect:
			if (pos < 0) return -pos-1; else if (pos > len-1) return 2*len-pos-1; else return pos;
			break;
		case border_type::reflect_101:
			if (pos < 0) return -pos; else if (pos > len-1) return 2*len-pos-2; else return pos;
			break;
		case border_type::wrap:
			if (pos < 0) return len+pos; else if (pos > len-1) return pos-len; else return pos;
			break;
		default:
			abort("Interpolation mode not recognized");
	}
}

void UMF::ArrayPad::init(){
	QAlgorithm::init();
	qRegisterMetaType<UMF::ArrayPad::border_type>();
}

void UMF::GaussianFilter::run(){
	// Checks
	if (getInSignal().isEmpty()){
		abort("Input signal not provided or empty");
		return;
	}
	auto inputSize = getInSignal().size();
	if (kernel.is_empty()) {
		abort("Kernel not initialized");
		return;
	}
	// Padding of input signal
	auto pad = ArrayPad::create({
		{"Radius", QVariant::fromValue(getRadius())},
		{"BorderType", QVariant::fromValue((int)getBorderType())},
		{"Signal", QVariant::fromValue(getInSignal())}
	});
	pad->run();
	auto padded = pad->getOutMoveSignal();
	// Convolution with kernel
	QVector<double> filtered(padded.size());
	vec _filtered(filtered.data(), filtered.size(), false, true);
	_filtered = conv(vec(padded.data(), padded.size(), false, true), kernel, "same");
	// Take the central part only
	auto begin = filtered.begin()+getRadius();
	auto end = begin+inputSize;
	std::move(begin, end, filtered.begin()); // shift to the left
	filtered.resize(inputSize);
	setOutSignal(std::move(filtered));
	setInSignal(QVector<double>());
}

void UMF::GaussianFilter::init(){
	QAlgorithm::init();
	qRegisterMetaType<UMF::ArrayPad::border_type>();
	// Kernel initialization
	const auto& m = getRadius();
	kernel = exp( - square(linspace(0, 2*m, 2*m+1) - m) / double(m*m) * 2.0 );
	kernel = normalise(kernel, 1/*1-norm*/);
}

void UMF::SpectrumMagnitude::run(){
	// Move the input signal to local scope
	auto rvSignal = getInMoveSignal();
	// Link the input signal to an alglib array
	alglib::real_1d_array signal;
	signal.setcontent(rvSignal.size(), rvSignal.data());
	// Compute the FFT
	alglib::complex_1d_array dft;
	alglib::fftr1d(signal, dft);
	// Compute the spectrum
	int halfSize = rvSignal.size()/2+1;
	double normFactor = 1.0/rvSignal.size();
	rvSignal.resize(halfSize);
	for(int k = 0; k < halfSize; ++k){
		rvSignal[k] = (dft[k].x*dft[k].x + dft[k].y*dft[k].y) * normFactor;
	}
	setOutSignal(std::move(rvSignal));
}

void UMF::SpectrumRemoveBackground::run(){
	QVector<double> out;
	out.reserve(getInSignal().size());
	std::copy(getInSignal().constBegin(), getInSignal().constEnd(), std::back_inserter(out));
	auto error = TSpectrum().Background(out.data(), out.size(),
										getNumberIterations(), getDirection(), getFilterOrder(),
										getSmoothing(), getSmoothWindow(), getCompton());
	if (error){
		abort(QString(error));
		return;
	}
	std::transform(getInSignal().constBegin(), getInSignal().constEnd(), out.constBegin(), out.begin(),
				   [](const auto& signal, const auto& background){ return signal-background;});
	setOutSignal(out);
}

void UMF::SpectrumRemoveBackground::init(){
	QAlgorithm::init();
	qRegisterMetaType<UMF::SpectrumRemoveBackground::filterOrder>();
	qRegisterMetaType<UMF::SpectrumRemoveBackground::windowDirection>();
	qRegisterMetaType<UMF::SpectrumRemoveBackground::smoothingWindow>();
}
