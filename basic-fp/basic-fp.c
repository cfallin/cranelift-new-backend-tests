#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

#define P(val, fmt) printf(#val " = %" fmt "\n", val)

double f(double a, double b) {
  P(a, "f");
  P(b, "f");
  double c = a + b;
  P(c, "f");
  double d = c / 43.0;
  P(d, "f");
  double e = d * c;
  P(e, "f");
  double f = fmin(c, d);
  P(f, "f");
  double g = fmax(c, d);
  P(g, "f");
  double h = exp(f) + exp(g);
  P(h, "f");
  double i = sqrt(h);
  P(i, "f");
  double j = g*h + i;
  P(j, "f");
  double k = ceil(j);
  P(k, "f");
  double l = floor(j);
  P(l, "f");
  double m = k + l;
  P(m, "f");
  float n = (float)m;
  P(n, "f");
  double o = (double)(1000.0 * n);
  P(o, "f");
  double p = trunc(o);
  P(p, "f");
  double q = ceil(p);
  P(q, "f");
  double r = fabs(q);
  P(r, "f");
  int32_t s = (int32_t)(1000.0 * a / b);
  P(s, PRIi32);
  int64_t t = (int64_t)(2000.0 * a / b);
  P(t, PRIi64);
  uint32_t u = (uint32_t)(fabs(a - b));
  P(u, PRIu32);
  uint64_t v = (uint64_t)(fabs(a - b));
  P(v, PRIu64);
  int32_t w = (int32_t)(3000.0 * a * b);
  P(w, PRIi32);
  int64_t x = (int64_t)(4000.0 * b / a);
  P(x, PRIi64);
  uint32_t y = (uint32_t)(5000.0 * fabsf((float)(a / b)));
  P(y, PRIu32);
  uint64_t z = (uint64_t)(6000.0 * fabsf((float)(b / a)));
  P(z, PRIu64);
  uint64_t aa = s + t + u + v + w + x + y + z;
  P(aa, PRIu64);
  int64_t bb = -aa;
  P(bb, PRIi64);
  uint32_t cc = (uint32_t)aa;
  P(cc, PRIu32);
  int32_t dd = -cc;
  P(dd, PRIi32);
  float ee = (float)aa;
  P(ee, "f");
  float ff = (float)bb;
  P(ff, "f");
  float gg = (float)cc;
  P(gg, "f");
  float hh = (float)dd;
  P(hh, "f");
  double ii = (double)aa;
  P(ii, "f");
  double jj = (double)bb;
  P(jj, "f");
  double kk = (double)cc;
  P(kk, "f");
  double ll = (double)dd;
  P(ll, "f");

  double result = ee + ff + gg + hh + ii + jj + kk + ll;

  int condition = (result == 42.0) || (result != 42.0) || (result > 42.0) | (result >= 42.0) | (result < 42.0) | (result <= 42.0);

  printf("result = %f, condition = %d\n", result, condition);

  return result;
}

int main() {
  double a = f(1.0, 2.0);
  double b = f(3.0, 4.0);
  double c = f(5.0, 6.0);
  double d = f(7.0, 8.0);

  printf("total = %f\n", a + b + c + d);
}
