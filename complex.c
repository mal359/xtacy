#include <math.h>
#include "complex.h"

/* from the sci.fractals FAQ
Q9a: How does complex arithmetic work?
A9a: It works mostly like regular algebra with a couple additional formulas:
  (note: a,b are reals, x,y are complex, i is the square root of -1)
  Powers of i: i^2 = -1
  Addition: (a+i*b)+(c+i*d) = (a+c)+i*(b+d)
  Multiplication: (a+i*b)*(c+i*d) = a*c-b*d + i*(a*d+b*c)
  Division: (a+i*b)/(c+i*d) = (a+i*b)*(c-i*d)/(c^2+d^2)
  Exponentiation: exp(a+i*b) = exp(a)(cos(b)+i*sin(b))
  Sine: sin(x) = (exp(i*x)-exp(-i*x))/(2*i)
  Cosine: cos(x) = (exp(i*x)+exp(-i*x)/2
  Magnitude: |a+i*b| = sqrt(a^2+b^2)
  Log: log(a+i*b) = log(|a+i*b|)+i*arctan(b/a)  (Note: log is multivalued.)
  Complex powers: x^y = exp(y*log(x))
  DeMoivre's theorem: x^a = r^a * [cos(a*theta) + i * sin(a*theta)]
*/

struct COMPLEX
divComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  struct COMPLEX temp;
  double denom= (bar.real*bar.real)+(bar.imag*bar.imag);
  if(denom!=0.0)
  {
    temp.real = ((foo.real*bar.real)+(foo.imag*bar.imag))/(denom);
    temp.imag = ((foo.imag*bar.real)-(foo.real*bar.imag))/(denom);
  }
  else 
  {
    temp.real= 100;
    temp.imag= 100;
  }
  
  return temp;
}

struct COMPLEX
logComplex(struct COMPLEX foo)
{
  struct COMPLEX temp;
  double gaaah=fabs(foo.real+foo.imag);
  if(gaaah<=0.0)
    temp.real=-10;
  else
    temp.real = log10(gaaah);
  if (foo.real ==0)
    temp.imag = 0;
  else
    temp.imag = atan2(foo.imag,foo.real);
  return temp;
}

struct COMPLEX
powComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  return (expComplex(multComplex(bar,logComplex(foo))));
}

struct COMPLEX
expComplex(struct COMPLEX foo)
{
  struct COMPLEX temp;
  temp.real = exp(foo.real)*cos(foo.imag);
  temp.imag = exp(foo.real)*sin(foo.imag);
  return temp;
}

struct COMPLEX
addComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  struct COMPLEX temp;
  temp.real = foo.real + bar.real;
  temp.imag = foo.imag + bar.imag;
  return temp;
}

struct COMPLEX
multComplex(struct COMPLEX foo, struct COMPLEX bar)
{
  struct COMPLEX temp;
  temp.real = (foo.real * bar.real) - (foo.imag * bar.imag);
  temp.imag = (foo.imag * bar.real) + (foo.real * bar.imag);
  return temp;
}

