#ifndef _GVECTOR_H_
#define _GVECTOR_H_


// An arbitrary dimension

#include <type_traits>
#include <array>
#include <ostream>
#include <cmath>

template<unsigned int N>
struct GVector{
	std::array<double,N> data;

	GVector(std::array<double,N> arr) : data(arr) {}

	template<typename... Args>
	GVector(Args... args) : data{double(args)...} {
		static_assert(sizeof...(args) == N,
									"Arguments passed do not match templated size");
	}

	double Mag2();

	double Mag(){
		return std::sqrt(Mag2());
	}

	// Modification operators
	GVector<N>& operator+=(const GVector<N>& other){
		for(unsigned int i=0; i<N; i++){
			data[i] += other.data[i];
		}
		return *this;
	}

	GVector<N>& operator-=(const GVector<N>& other){
		for(unsigned int i=0; i<N; i++){
			data[i] -= other.data[i];
		}
		return *this;
	}

	GVector<N>& operator*=(const double other){
		for(unsigned int i=0; i<N; i++){
			data[i] *= other;
		}
		return *this;
	}

	GVector<N>& operator/=(const double other){
		for(unsigned int i=0; i<N; i++){
			data[i] /= other;
		}
		return *this;
	}

	// Cross-product
	template<typename = typename std::enable_if< N==3 >::type>
	GVector<N> operator^(const GVector<N>& b) const {
		return GVector<N>(Y()*b.Z() - Z()*b.Y(),
											Z()*b.X() - X()*b.Z(),
											X()*b.Y() - Y()*b.X());
	}

	// Ease of use accessors for dimensions x,y,z
	template<typename = typename std::enable_if< N>=1 >::type>
	double& X(){return data[0];}
	template<typename = typename std::enable_if< N>=1 >::type>
	const double& X() const {return data[0];}

	template<typename = typename std::enable_if< N>=2 >::type>
	double& Y(){return data[1];}
	template<typename = typename std::enable_if< N>=2 >::type>
	const double& Y() const {return data[1];}

	template<typename = typename std::enable_if< N>=3 >::type>
	double& Z(){return data[2];}
	template<typename = typename std::enable_if< N>=3 >::type>
	const double& Z() const {return data[2];}
};

// Vector addition
template<unsigned int N>
GVector<N> operator+(GVector<N> a, const GVector<N>& b){
	return a += b;
}

// Vector subtraction
template<unsigned int N>
GVector<N> operator-(GVector<N> a, const GVector<N>& b){
	return a -= b;
}

// Left scalar multiplication
template<unsigned int N>
GVector<N> operator*(double a, GVector<N> b){
	return b *= a;
}

// Right scalar multiplication
template<unsigned int N>
GVector<N> operator*(GVector<N> a, double b){
	return a *= b;
}

// Dot product
template<unsigned int N>
double operator*(const GVector<N>& a, const GVector<N>& b){
	double output = 0;
	for(unsigned int i=0; i<N; i++){
		output += a.data[i]*b.data[i];
	}
	return output;
}

template<unsigned int N>
std::ostream& operator<<(std::ostream& st, GVector<N> gv){
	st << "(";
	for(unsigned int i=0; i<N; i++){
		st << gv.data[i];
		if(i!=N-1){
			st << ", ";
		}
	}
	st << ")";
}

#endif /* _GVECTOR_H_ */
