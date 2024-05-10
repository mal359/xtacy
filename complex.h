struct COMPLEX
{
  double real;
  double imag;
};

struct COMPLEX addComplex(struct COMPLEX,struct COMPLEX);
struct COMPLEX multComplex(struct COMPLEX,struct COMPLEX);
struct COMPLEX expComplex(struct COMPLEX);
struct COMPLEX divComplex(struct COMPLEX foo, struct COMPLEX bar);
struct COMPLEX logComplex(struct COMPLEX foo);
struct COMPLEX powComplex(struct COMPLEX foo, struct COMPLEX bar);
