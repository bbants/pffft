/* Copyright (c) 2020  Dario Mambro ( dario.mambro@gmail.com )
   Copyright (c) 2020  Hayati Ayguen ( h_ayguen@web.de )

   Redistribution and use of the Software in source and binary forms,
   with or without modification, is permitted provided that the
   following conditions are met:

   - Neither the names of PFFFT, nor the names of its
   sponsors or contributors may be used to endorse or promote products
   derived from this Software without specific prior written permission.

   - Redistributions of source code must retain the above copyright
   notices, this list of conditions, and the disclaimer below.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions, and the disclaimer below in the
   documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
   SOFTWARE.
*/

#pragma once

#include <complex>
#include <vector>
#include <limits>

namespace {
#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )
#include "pffft.h"
#endif
#if defined(PFFFT_ENABLE_DOUBLE)
#include "pffft_double.h"
#endif
}

namespace pffft {

// enum { PFFFT_REAL, PFFFT_COMPLEX }
typedef pffft_transform_t TransformType;

// define 'Scalar' and 'Complex' (in namespace pffft) with template Types<>
// and other type specific helper functions
template<typename T> struct Types {};
#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )
template<> struct Types<float>  {
  typedef float  Scalar;
  typedef std::complex<Scalar> Complex;
  static Scalar* alignedAlloc(int N) { return (Scalar*)pffft_aligned_malloc( N * sizeof(Scalar) ); }
  static void    alignedFree(void *ptr) { pffft_aligned_free(ptr); }
  static int minFFtsize() { return pffft_min_fft_size(PFFFT_REAL); }
  static int nextPowerOfTwo(int N) { return pffft_next_power_of_two(N); }
  static bool isPowerOfTwo(int N) { return pffft_is_power_of_two(N); }
  static int simd_size() { return pffft_simd_size(); }
  static const char * simd_arch() { return pffft_simd_arch(); }
};
template<> struct Types< std::complex<float> >  {
  typedef float  Scalar;
  typedef std::complex<float>  Complex;
  static Complex* alignedAlloc(int N) { return (Complex*)pffft_aligned_malloc( N * sizeof(Complex) ); }
  static void     alignedFree(void *ptr) { pffft_aligned_free(ptr); }
  static int minFFtsize() { return pffft_min_fft_size(PFFFT_COMPLEX); }
  static int nextPowerOfTwo(int N) { return pffft_next_power_of_two(N); }
  static bool isPowerOfTwo(int N) { return pffft_is_power_of_two(N); }
  static int simd_size() { return pffft_simd_size(); }
  static const char * simd_arch() { return pffft_simd_arch(); }
};
#endif
#if defined(PFFFT_ENABLE_DOUBLE)
template<> struct Types<double> {
  typedef double Scalar;
  typedef std::complex<Scalar> Complex;
  static Scalar* alignedAlloc(int N) { return (Scalar*)pffftd_aligned_malloc( N * sizeof(Scalar) ); }
  static void    alignedFree(void *ptr) { pffftd_aligned_free(ptr); }
  static int minFFtsize() { return pffftd_min_fft_size(PFFFT_REAL); }
  static int nextPowerOfTwo(int N) { return pffftd_next_power_of_two(N); }
  static bool isPowerOfTwo(int N) { return pffftd_is_power_of_two(N); }
  static int simd_size() { return pffftd_simd_size(); }
  static const char * simd_arch() { return pffftd_simd_arch(); }
};
template<> struct Types< std::complex<double> > {
  typedef double Scalar;
  typedef std::complex<double> Complex;
  static Complex* alignedAlloc(int N) { return (Complex*)pffftd_aligned_malloc( N * sizeof(Complex) ); }
  static void     alignedFree(void *ptr) { pffftd_aligned_free(ptr); }
  static int minFFtsize() { return pffftd_min_fft_size(PFFFT_COMPLEX); }
  static int nextPowerOfTwo(int N) { return pffftd_next_power_of_two(N); }
  static bool isPowerOfTwo(int N) { return pffftd_is_power_of_two(N); }
  static int simd_size() { return pffftd_simd_size(); }
  static const char * simd_arch() { return pffftd_simd_arch(); }
};
#endif

// Allocator
template<typename T> class PFAlloc;

namespace {
  template<typename T> class Setup;
}

#if (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))

// define AlignedVector<T> utilizing 'using' in C++11
template<typename T>
using AlignedVector = typename std::vector< T, PFAlloc<T> >;

#else

// define AlignedVector<T> having to derive std::vector<>
template <typename T>
struct AlignedVector : public std::vector< T, PFAlloc<T> > {
  AlignedVector() : std::vector< T, PFAlloc<T> >() { }
  AlignedVector(int N) : std::vector< T, PFAlloc<T> >(N) { }
};

#endif


// T can be float, double, std::complex<float> or std::complex<double>
//   define PFFFT_ENABLE_DOUBLE before include this file for double and std::complex<double>
template<typename T>
class Fft
{
public:

  // define types value_type, Scalar and Complex
  typedef T value_type;
  typedef typename Types<T>::Scalar  Scalar;
  typedef typename Types<T>::Complex Complex;

  // static retrospection functions
  static bool isComplexTransform()  { return sizeof(T) == sizeof(Complex); }
  static bool isFloatScalar()  { return sizeof(Scalar) == sizeof(float); }
  static bool isDoubleScalar() { return sizeof(Scalar) == sizeof(double); }

  // simple helper to get minimum possible fft length
  static int minFFtsize() { return Types<T>::minFFtsize(); }
  // simple helper to determine next power of 2 - without inexact/rounding floating point operations
  static int nextPowerOfTwo(int N) { return Types<T>::nextPowerOfTwo(N); }
  static bool isPowerOfTwo(int N) { return Types<T>::isPowerOfTwo(N); }
  static int simd_size() { return Types<T>::simd_size(); }
  static const char * simd_arch() { return Types<T>::simd_arch(); }

  //////////////////

  /*
   * Contructor, with transformation length, preparing transforms.
   *
   * For length <= stackThresholdLen, the stack is used for the internal
   * work memory. for bigger length', the heap is used.
   *
   * Using the stack is probably the best strategy for small
   * FFTs, say for N <= 4096). Threads usually have a small stack, that
   * there's no sufficient amount of memory, usually leading to a crash!
   */
  Fft( int length, int stackThresholdLen = 4096 );

  ~Fft();

  /*
   * prepare for transformation length 'newLength'.
   * length is identical to forward()'s input vector's size,
   * and also equals inverse()'s output vector size.
   * this function is no simple setter. it pre-calculates twiddle factors.
   */
  void prepareLength(int newLength);

  /*
   * retrieve the transformation length.
   */
  int getLength() const { return length; }

  /*
   * retrieve size of complex spectrum vector,
   * the output of forward()
   */
  int getSpectrumSize() const { return isComplexTransform() ? length : ( length / 2 ); }

  /*
   * retrieve size of spectrum vector - in internal layout;
   * the output of forwardToInternalLayout()
   */
  int getInternalLayoutSize() const { return isComplexTransform() ? ( 2 * length ) : length; }


  ////////////////////////////////////////////
  ////
  //// API 1, with std::vector<> based containers,
  ////   which free the allocated memory themselves (RAII).
  ////
  //// uses an Allocator for the alignment of SIMD data.
  ////
  ////////////////////////////////////////////

  // create suitably preallocated aligned vector for one FFT
  AlignedVector<T>       valueVector() const;
  AlignedVector<Complex> spectrumVector() const;
  AlignedVector<Scalar>  internalLayoutVector() const;

  ////////////////////////////////////////////
  // although using Vectors for output ..
  // they need to have resize() applied before!

  // core API, having the spectrum in canonical order

  /*
   * Perform the forward Fourier transform.
   *
   * Transforms are not scaled: inverse(forward(x)) = N*x.
   * Typically you will want to scale the backward transform by 1/N.
   *
   * The output 'spectrum' is canonically ordered - as expected.
   *
   * a) for complex input isComplexTransform() == true,
   *    and transformation length N  the output array is complex:
   *   index k in 0 .. N/2 -1  corresponds to frequency k * Samplerate / N
   *   index k in N/2 .. N -1  corresponds to frequency (k -N) * Samplerate / N,
   *     resulting in negative frequencies
   *
   * b) for real input isComplexTransform() == false,
   *    and transformation length N  the output array is 'mostly' complex:
   *   index k in 1 .. N/2 -1  corresponds to frequency k * Samplerate / N
   *   index k == 0 is a special case:
   *     the real() part contains the result for the DC frequency 0,
   *     the imag() part contains the result for the Nyquist frequency Samplerate/2
   *   both 0-frequency and half frequency components, which are real,
   *   are assembled in the first entry as  F(0)+i*F(N/2).
   *
   * input and output may alias - if you do nasty type conversion.
   * return is just the given output parameter 'spectrum'.
   */
  AlignedVector<Complex> & forward(const AlignedVector<T> & input, AlignedVector<Complex> & spectrum);

  /*
   * Perform the inverse Fourier transform, see forward().
   * return is just the given output parameter 'output'.
   */
  AlignedVector<T> & inverse(const AlignedVector<Complex> & spectrum, AlignedVector<T> & output);


  // provide additional functions with spectrum in some internal Layout.
  // these are faster, cause the implementation omits the reordering.
  // these are useful in special applications, like fast convolution,
  // where inverse() is following anyway ..

  /*
   * Perform the forward Fourier transform - similar to forward(), BUT:
   *
   * The z-domain data is stored in the most efficient order
   * for transforming it back, or using it for convolution.
   * If you need to have its content sorted in the "usual" canonical order,
   * either use forward(), or call reorderSpectrum() after calling
   * forwardToInternalLayout(), and before the backward fft
   *
   * return is just the given output parameter 'spectrum_internal_layout'.
   */
  AlignedVector<Scalar> & forwardToInternalLayout(
          const AlignedVector<T> & input,
          AlignedVector<Scalar> & spectrum_internal_layout );

  /*
   * Perform the inverse Fourier transform, see forwardToInternalLayout()
   *
   * return is just the given output parameter 'output'.
   */
  AlignedVector<T> & inverseFromInternalLayout(
          const AlignedVector<Scalar> & spectrum_internal_layout,
          AlignedVector<T> & output );

  /*
   * Reorder the spectrum from internal layout to have the
   * frequency components in the correct "canonical" order.
   * see forward() for a description of the canonical order.
   *
   * input and output should not alias.
   */
  void reorderSpectrum(
          const AlignedVector<Scalar> & input,
          AlignedVector<Complex> & output );

  /*
   * Perform a multiplication of the frequency components of
   * spectrum_internal_a and spectrum_internal_b
   * into spectrum_internal_ab.
   * The arrays should have been obtained with forwardToInternalLayout)
   * and should *not* have been reordered with reorderSpectrum().
   *
   * the operation performed is:
   *  spectrum_internal_ab = (spectrum_internal_a * spectrum_internal_b)*scaling
   *
   * The spectrum_internal_[a][b], pointers may alias.
   * return is just the given output parameter 'spectrum_internal_ab'.
   */
  AlignedVector<Scalar> & convolve(
          const AlignedVector<Scalar> & spectrum_internal_a,
          const AlignedVector<Scalar> & spectrum_internal_b,
          AlignedVector<Scalar> & spectrum_internal_ab,
          const Scalar scaling );

  /*
   * Perform a multiplication and accumulation of the frequency components
   * - similar to convolve().
   *
   * the operation performed is:
   *  spectrum_internal_ab += (spectrum_internal_a * spectrum_internal_b)*scaling
   *
   * The spectrum_internal_[a][b], pointers may alias.
   * return is just the given output parameter 'spectrum_internal_ab'.
   */
  AlignedVector<Scalar> & convolveAccumulate(
          const AlignedVector<Scalar> & spectrum_internal_a,
          const AlignedVector<Scalar> & spectrum_internal_b,
          AlignedVector<Scalar> & spectrum_internal_ab,
          const Scalar scaling );


  ////////////////////////////////////////////
  ////
  //// API 2, dealing with raw pointers,
  //// which need to be deallocated using alignedFree()
  ////
  //// the special allocation is required cause SIMD
  //// implementations require aligned memory
  ////
  //// Method descriptions are equal to the methods above,
  //// having  AlignedVector<T> parameters - instead of raw pointers.
  //// That is why following methods have no documentation.
  ////
  ////////////////////////////////////////////

  static void alignedFree(void* ptr);

  static T * alignedAllocType(int length);
  static Scalar* alignedAllocScalar(int length);
  static Complex* alignedAllocComplex(int length);

  // core API, having the spectrum in canonical order

  Complex* forward(const T* input, Complex* spectrum);

  T* inverse(const Complex* spectrum, T* output);


  // provide additional functions with spectrum in some internal Layout.
  // these are faster, cause the implementation omits the reordering.
  // these are useful in special applications, like fast convolution,
  // where inverse() is following anyway ..

  Scalar* forwardToInternalLayout(const T* input,
                                Scalar* spectrum_internal_layout);

  T* inverseFromInternalLayout(const Scalar* spectrum_internal_layout, T* output);

  void reorderSpectrum(const Scalar* input, Complex* output );

  Scalar* convolve(const Scalar* spectrum_internal_a,
                   const Scalar* spectrum_internal_b,
                   Scalar* spectrum_internal_ab,
                   const Scalar scaling);

  Scalar* convolveAccumulate(const Scalar* spectrum_internal_a,
                             const Scalar* spectrum_internal_b,
                             Scalar* spectrum_internal_ab,
                             const Scalar scaling);

private:
  Setup<T> setup;
  Scalar* work;
  int length;
  int stackThresholdLen;
};



////////////////////////////////////////////////////////////////////

// implementation

namespace {

template<typename T>
class Setup
{};

#if defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) )

template<>
class Setup<float>
{
  PFFFT_Setup* self;

public:
  typedef float value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  void prepareLength(int length)
  {
    if (self) {
      pffft_destroy_setup(self);
    }
    self = pffft_new_setup(length, PFFFT_REAL);
  }

  ~Setup() { pffft_destroy_setup(self); }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffft_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffft_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffft_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffft_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffft_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

template<>
class Setup< std::complex<float> >
{
  PFFFT_Setup* self;

public:
  typedef std::complex<float> value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffft_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffft_destroy_setup(self);
    }
    self = pffft_new_setup(length, PFFFT_COMPLEX);
  }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffft_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffft_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffft_zreorder(self, input, output, direction);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffft_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

#endif /* defined(PFFFT_ENABLE_FLOAT) || ( !defined(PFFFT_ENABLE_FLOAT) && !defined(PFFFT_ENABLE_DOUBLE) ) */


#if defined(PFFFT_ENABLE_DOUBLE)

template<>
class Setup<double>
{
  PFFFTD_Setup* self;

public:
  typedef double value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffftd_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffftd_destroy_setup(self);
      self = NULL;
    }
    if (length > 0) {
      self = pffftd_new_setup(length, PFFFT_REAL);
    }
  }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffftd_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffftd_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffftd_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffftd_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffftd_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

template<>
class Setup< std::complex<double> >
{
  PFFFTD_Setup* self;

public:
  typedef std::complex<double> value_type;
  typedef Types< value_type >::Scalar Scalar;

  Setup()
    : self(NULL)
  {}

  ~Setup() { pffftd_destroy_setup(self); }

  void prepareLength(int length)
  {
    if (self) {
      pffftd_destroy_setup(self);
    }
    self = pffftd_new_setup(length, PFFFT_COMPLEX);
  }

  void transform_ordered(const Scalar* input,
                         Scalar* output,
                         Scalar* work,
                         pffft_direction_t direction)
  {
    pffftd_transform_ordered(self, input, output, work, direction);
  }

  void transform(const Scalar* input,
                 Scalar* output,
                 Scalar* work,
                 pffft_direction_t direction)
  {
    pffftd_transform(self, input, output, work, direction);
  }

  void reorder(const Scalar* input, Scalar* output, pffft_direction_t direction)
  {
    pffftd_zreorder(self, input, output, direction);
  }

  void convolveAccumulate(const Scalar* dft_a,
                          const Scalar* dft_b,
                          Scalar* dft_ab,
                          const Scalar scaling)
  {
    pffftd_zconvolve_accumulate(self, dft_a, dft_b, dft_ab, scaling);
  }

  void convolve(const Scalar* dft_a,
                const Scalar* dft_b,
                Scalar* dft_ab,
                const Scalar scaling)
  {
    pffftd_zconvolve_no_accu(self, dft_a, dft_b, dft_ab, scaling);
  }
};

#endif /* defined(PFFFT_ENABLE_DOUBLE) */

} // end of anonymous namespace for Setup<>


template<typename T>
inline Fft<T>::Fft(int length, int stackThresholdLen)
  : length(0)
  , stackThresholdLen(stackThresholdLen)
  , work(NULL)
{
#if (__cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900))
  static_assert( sizeof(Complex) == 2 * sizeof(Scalar), "pffft requires sizeof(std::complex<>) == 2 * sizeof(Scalar)" );
#elif defined(__GNUC__)
  char static_assert_like[(sizeof(Complex) == 2 * sizeof(Scalar)) ? 1 : -1]; // pffft requires sizeof(std::complex<>) == 2 * sizeof(Scalar)
#endif
  prepareLength(length);
}

template<typename T>
inline Fft<T>::~Fft()
{
  Types<T>::alignedFree(work);
}

template<typename T>
inline void
Fft<T>::prepareLength(int newLength)
{
  const bool wasOnHeap = ( work != NULL );

  const bool useHeap = newLength > stackThresholdLen;

  if (useHeap == wasOnHeap && newLength == length) {
    return;
  }

  length = newLength;

  setup.prepareLength(length);

  if (work) {
    Types<T>::alignedFree(work);
    work = NULL;
  }

  if (useHeap) {
    work = reinterpret_cast<Scalar*>( Types<T>::alignedAlloc(length) );
  }
}


template<typename T>
inline AlignedVector<T>
Fft<T>::valueVector() const
{
  return AlignedVector<T>(length);
}

template<typename T>
inline AlignedVector< typename Fft<T>::Complex >
Fft<T>::spectrumVector() const
{
  return AlignedVector<Complex>( getSpectrumSize() );
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar >
Fft<T>::internalLayoutVector() const
{
  return AlignedVector<Scalar>( getInternalLayoutSize() );
}


template<typename T>
inline AlignedVector< typename Fft<T>::Complex > &
Fft<T>::forward(const AlignedVector<T> & input, AlignedVector<Complex> & spectrum)
{
  forward( input.data(), spectrum.data() );
  return spectrum;
}

template<typename T>
inline AlignedVector<T> &
Fft<T>::inverse(const AlignedVector<Complex> & spectrum, AlignedVector<T> & output)
{
  inverse( spectrum.data(), output.data() );
  return output;
}


template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::forwardToInternalLayout(
    const AlignedVector<T> & input,
    AlignedVector<Scalar> & spectrum_internal_layout )
{
  forwardToInternalLayout( input.data(), spectrum_internal_layout.data() );
  return spectrum_internal_layout;
}

template<typename T>
inline AlignedVector<T> &
Fft<T>::inverseFromInternalLayout(
    const AlignedVector<Scalar> & spectrum_internal_layout,
    AlignedVector<T> & output )
{
  inverseFromInternalLayout( spectrum_internal_layout.data(), output.data() );
  return output;
}

template<typename T>
inline void
Fft<T>::reorderSpectrum(
    const AlignedVector<Scalar> & input,
    AlignedVector<Complex> & output )
{
  reorderSpectrum( input.data(), output.data() );
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::convolveAccumulate(
    const AlignedVector<Scalar> & spectrum_internal_a,
    const AlignedVector<Scalar> & spectrum_internal_b,
    AlignedVector<Scalar> & spectrum_internal_ab,
    const Scalar scaling )
{
  convolveAccumulate( spectrum_internal_a.data(), spectrum_internal_b.data(),
                      spectrum_internal_ab.data(), scaling );
  return spectrum_internal_ab;
}

template<typename T>
inline AlignedVector< typename Fft<T>::Scalar > &
Fft<T>::convolve(
    const AlignedVector<Scalar> & spectrum_internal_a,
    const AlignedVector<Scalar> & spectrum_internal_b,
    AlignedVector<Scalar> & spectrum_internal_ab,
    const Scalar scaling )
{
  convolve( spectrum_internal_a.data(), spectrum_internal_b.data(),
            spectrum_internal_ab.data(), scaling );
  return spectrum_internal_ab;
}


template<typename T>
inline typename Fft<T>::Complex *
Fft<T>::forward(const T* input, Complex * spectrum)
{
  setup.transform_ordered(reinterpret_cast<const Scalar*>(input),
                          reinterpret_cast<Scalar*>(spectrum),
                          work,
                          PFFFT_FORWARD);
  return spectrum;
}

template<typename T>
inline T*
Fft<T>::inverse(Complex const* spectrum, T* output)
{
  setup.transform_ordered(reinterpret_cast<const Scalar*>(spectrum),
                          reinterpret_cast<Scalar*>(output),
                          work,
                          PFFFT_BACKWARD);
  return output;
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::forwardToInternalLayout(const T* input, Scalar* spectrum_internal_layout)
{
  setup.transform(reinterpret_cast<const Scalar*>(input),
                  spectrum_internal_layout,
                  work,
                  PFFFT_FORWARD);
  return spectrum_internal_layout;
}

template<typename T>
inline T*
Fft<T>::inverseFromInternalLayout(const Scalar* spectrum_internal_layout, T* output)
{
  setup.transform(spectrum_internal_layout,
                  reinterpret_cast<Scalar*>(output),
                  work,
                  PFFFT_BACKWARD);
  return output;
}

template<typename T>
inline void
Fft<T>::reorderSpectrum( const Scalar* input, Complex* output )
{
  setup.reorder(input, reinterpret_cast<Scalar*>(output), PFFFT_FORWARD);
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::convolveAccumulate(const Scalar* dft_a,
                           const Scalar* dft_b,
                           Scalar* dft_ab,
                           const Scalar scaling)
{
  setup.convolveAccumulate(dft_a, dft_b, dft_ab, scaling);
  return dft_ab;
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
Fft<T>::convolve(const Scalar* dft_a,
                 const Scalar* dft_b,
                 Scalar* dft_ab,
                 const Scalar scaling)
{
  setup.convolve(dft_a, dft_b, dft_ab, scaling);
  return dft_ab;
}

template<typename T>
inline void
Fft<T>::alignedFree(void* ptr)
{
  Types<T>::alignedFree(ptr);
}


template<typename T>
inline T*
pffft::Fft<T>::alignedAllocType(int length)
{
  return Types<T>::alignedAlloc(length);
}

template<typename T>
inline typename pffft::Fft<T>::Scalar*
pffft::Fft<T>::alignedAllocScalar(int length)
{
  return Types<Scalar>::alignedAlloc(length);
}

template<typename T>
inline typename Fft<T>::Complex *
Fft<T>::alignedAllocComplex(int length)
{
  return Types<Complex>::alignedAlloc(length);
}



////////////////////////////////////////////////////////////////////

// Allocator - for std::vector<>:
// origin: http://www.josuttis.com/cppcode/allocator.html
// http://www.josuttis.com/cppcode/myalloc.hpp
//
// minor renaming and utilizing of pffft (de)allocation functions
// are applied to Jossutis' allocator

/* The following code example is taken from the book
 * "The C++ Standard Library - A Tutorial and Reference"
 * by Nicolai M. Josuttis, Addison-Wesley, 1999
 *
 * (C) Copyright Nicolai M. Josuttis 1999.
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */

template <class T>
class PFAlloc {
  public:
    // type definitions
    typedef T        value_type;
    typedef T*       pointer;
    typedef const T* const_pointer;
    typedef T&       reference;
    typedef const T& const_reference;
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind {
        typedef PFAlloc<U> other;
    };

    // return address of values
    pointer address (reference value) const {
        return &value;
    }
    const_pointer address (const_reference value) const {
        return &value;
    }

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    PFAlloc() throw() {
    }
    PFAlloc(const PFAlloc&) throw() {
    }
    template <class U>
      PFAlloc (const PFAlloc<U>&) throw() {
    }
    ~PFAlloc() throw() {
    }

    // return maximum number of elements that can be allocated
    size_type max_size () const throw() {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate (size_type num, const void* = 0) {
        pointer ret = (pointer)( Types<T>::alignedAlloc(num) );
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct (pointer p, const T& value) {
        // initialize memory with placement new
        new((void*)p)T(value);
    }

    // destroy elements of initialized storage p
    void destroy (pointer p) {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate (pointer p, size_type num) {
        // deallocate memory with pffft
        Types<T>::alignedFree( (void*)p );
    }
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator== (const PFAlloc<T1>&,
                 const PFAlloc<T2>&) throw() {
    return true;
}
template <class T1, class T2>
bool operator!= (const PFAlloc<T1>&,
                 const PFAlloc<T2>&) throw() {
    return false;
}


} // namespace pffft

