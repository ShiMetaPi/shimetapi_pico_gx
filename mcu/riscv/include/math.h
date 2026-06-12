
#ifndef _MATH_H_
#define _MATH_H_

#define FP_NAN         0
#define FP_INFINITE    1
#define FP_ZERO        2
#define FP_SUBNORMAL   3
#define FP_NORMAL      4

# ifndef HUGE_VALF
#  define HUGE_VALF (__builtin_huge_valf())
# endif

# ifndef INFINITY
#  define INFINITY (__builtin_inff())
# endif

# ifndef NAN
#  define NAN (__builtin_nanf(""))
# endif

#define fpclassify(__x) (__builtin_fpclassify (FP_NAN, FP_INFINITE, \
					 FP_NORMAL, FP_SUBNORMAL, \
					 FP_ZERO, __x))
#ifndef isfinite
#define isfinite(__x)	(__builtin_isfinite (__x))
#endif
#ifndef isinf
#define isinf(__x) (__builtin_isinf_sign (__x))
#endif
#ifndef isnan
#define isnan(__x) (__builtin_isnan (__x))
#endif
#define isnormal(__x) (__builtin_isnormal (__x))

float logf(float x);
float sqrtf(float x);
float powf(float x, float y);
float fabsf(float x);
float sinf(float x);
float cosf(float x);

#undef	log
#define	log(x)		logf(x)
#undef	pow
#define	pow(x, y)	powf(x, y)
#undef	abs
#define	abs(x)		fabsf(x)
#undef	fabs
#define	fabs(x)		fabsf(x)
#undef	sqrt
#define	sqrt(x)		sqrtf(x)
#undef	sin
#define	sin(x)		sinf(x)
#undef	cos
#define	cos(x)		cosf(x)


#endif /* _MATH_H_ */

