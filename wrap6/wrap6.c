
/*---------------------------------------------------------*/
/*--- BEGIN vex_printf                                  ---*/
/*---------------------------------------------------------*/

/* A standalone mini-printf implementation.  The only thing it assumes is the
   presence of the 'write' call so as to be able to send its output to
   stdout. */

// Don't ask ..
typedef  char                    HChar; // "host char"
typedef  signed int              Int;   // 32 bits
typedef  unsigned int            UInt;  // 32 bits
typedef  signed long long int    Long;  // 64 bits
typedef  unsigned long long int  ULong; // 64 bits
typedef  unsigned long           HWord; // "host word"

typedef  unsigned char  Bool;
#define False ((Bool)0)
#define True ((Bool)1)

#include <stdarg.h> // varargs nonsense
#include <unistd.h> // write

static HChar vex_toupper ( HChar c )
{
   if (c >= 'a' && c <= 'z')
      return c + ('A' - 'a');
   else
      return c;
}

static Int vex_strlen ( const HChar* str )
{
   Int i = 0;
   while (str[i] != 0) i++;
   return i;
}

/* Some flags.  */
#define VG_MSG_SIGNED    1 /* The value is signed. */
#define VG_MSG_ZJUSTIFY  2 /* Must justify with '0'. */
#define VG_MSG_LJUSTIFY  4 /* Must justify on the left. */
#define VG_MSG_PAREN     8 /* Parenthesize if present (for %y) */
#define VG_MSG_COMMA    16 /* Add commas to numbers (for %d, %u) */

/* Copy a string into the buffer. */
static UInt
myvprintf_str ( void(*send)(HChar), Int flags, Int width, HChar* str, 
                Bool capitalise )
{
#  define MAYBE_TOUPPER(ch) (capitalise ? vex_toupper(ch) : (ch))
   UInt ret = 0;
   Int i, extra;
   Int len = vex_strlen(str);

   if (width == 0) {
      ret += len;
      for (i = 0; i < len; i++)
         send(MAYBE_TOUPPER(str[i]));
      return ret;
   }

   if (len > width) {
      ret += width;
      for (i = 0; i < width; i++)
         send(MAYBE_TOUPPER(str[i]));
      return ret;
   }

   extra = width - len;
   if (flags & VG_MSG_LJUSTIFY) {
      ret += extra;
      for (i = 0; i < extra; i++)
         send(' ');
   }
   ret += len;
   for (i = 0; i < len; i++)
      send(MAYBE_TOUPPER(str[i]));
   if (!(flags & VG_MSG_LJUSTIFY)) {
      ret += extra;
      for (i = 0; i < extra; i++)
         send(' ');
   }

#  undef MAYBE_TOUPPER

   return ret;
}

/* Write P into the buffer according to these args:
 *  If SIGN is true, p is a signed.
 *  BASE is the base.
 *  If WITH_ZERO is true, '0' must be added.
 *  WIDTH is the width of the field.
 */
static UInt
myvprintf_int64 ( void(*send)(HChar), Int flags, Int base, Int width, ULong pL)
{
   HChar buf[40];
   Int   ind = 0;
   Int   i, nc = 0;
   Bool  neg = False;
   HChar *digits = "0123456789ABCDEF";
   UInt  ret = 0;
   UInt  p = (UInt)pL;

   if (base < 2 || base > 16)
      return ret;
 
   if ((flags & VG_MSG_SIGNED) && (Int)p < 0) {
      p   = - (Int)p;
      neg = True;
   }

   if (p == 0)
      buf[ind++] = '0';
   else {
      while (p > 0) {
         if ((flags & VG_MSG_COMMA) && 10 == base &&
             0 == (ind-nc) % 3 && 0 != ind) 
         {
            buf[ind++] = ',';
            nc++;
         }
         buf[ind++] = digits[p % base];
         p /= base;
      }
   }

   if (neg)
      buf[ind++] = '-';

   if (width > 0 && !(flags & VG_MSG_LJUSTIFY)) {
      for(; ind < width; ind++) {
	//vassert(ind < 39);
         buf[ind] = ((flags & VG_MSG_ZJUSTIFY) ? '0': ' ');
      }
   }

   /* Reverse copy to buffer.  */
   ret += ind;
   for (i = ind -1; i >= 0; i--) {
      send(buf[i]);
   }
   if (width > 0 && (flags & VG_MSG_LJUSTIFY)) {
      for(; ind < width; ind++) {
	 ret++;
         send(' ');  // Never pad with zeroes on RHS -- changes the value!
      }
   }
   return ret;
}


/* A simple vprintf().  */
static 
UInt vprintf_wrk ( void(*send)(HChar), const HChar *format, va_list vargs )
{
   UInt ret = 0;
   int i;
   int flags;
   int width;
   Bool is_long;

   /* We assume that vargs has already been initialised by the 
      caller, using va_start, and that the caller will similarly
      clean up with va_end.
   */

   for (i = 0; format[i] != 0; i++) {
      if (format[i] != '%') {
         send(format[i]);
	 ret++;
         continue;
      }
      i++;
      /* A '%' has been found.  Ignore a trailing %. */
      if (format[i] == 0)
         break;
      if (format[i] == '%') {
         /* `%%' is replaced by `%'. */
         send('%');
	 ret++;
         continue;
      }
      flags = 0;
      is_long = False;
      width = 0; /* length of the field. */
      if (format[i] == '(') {
	 flags |= VG_MSG_PAREN;
	 i++;
      }
      /* If ',' follows '%', commas will be inserted. */
      if (format[i] == ',') {
         flags |= VG_MSG_COMMA;
         i++;
      }
      /* If '-' follows '%', justify on the left. */
      if (format[i] == '-') {
         flags |= VG_MSG_LJUSTIFY;
         i++;
      }
      /* If '0' follows '%', pads will be inserted. */
      if (format[i] == '0') {
         flags |= VG_MSG_ZJUSTIFY;
         i++;
      }
      /* Compute the field length. */
      while (format[i] >= '0' && format[i] <= '9') {
         width *= 10;
         width += format[i++] - '0';
      }
      while (format[i] == 'l') {
         i++;
         is_long = True;
      }

      switch (format[i]) {
         case 'd': /* %d */
            flags |= VG_MSG_SIGNED;
            if (is_long)
               ret += myvprintf_int64(send, flags, 10, width, 
				      (ULong)(va_arg (vargs, Long)));
            else
               ret += myvprintf_int64(send, flags, 10, width, 
				      (ULong)(va_arg (vargs, Int)));
            break;
         case 'u': /* %u */
            if (is_long)
               ret += myvprintf_int64(send, flags, 10, width, 
				      (ULong)(va_arg (vargs, ULong)));
            else
               ret += myvprintf_int64(send, flags, 10, width, 
				      (ULong)(va_arg (vargs, UInt)));
            break;
         case 'p': /* %p */
	    ret += 2;
            send('0');
            send('x');
            ret += myvprintf_int64(send, flags, 16, width, 
				   (ULong)((HWord)va_arg (vargs, void *)));
            break;
         case 'x': /* %x */
            if (is_long)
               ret += myvprintf_int64(send, flags, 16, width, 
				      (ULong)(va_arg (vargs, ULong)));
            else
               ret += myvprintf_int64(send, flags, 16, width, 
				      (ULong)(va_arg (vargs, UInt)));
            break;
         case 'c': /* %c */
	    ret++;
            send((va_arg (vargs, int)));
            break;
         case 's': case 'S': { /* %s */
            char *str = va_arg (vargs, char *);
            if (str == (char*) 0) str = "(null)";
            ret += myvprintf_str(send, flags, width, str, 
                                 (format[i]=='S'));
            break;
	 }
         default:
            break;
      }
   }
   return ret;
}


/* A general replacement for printf().  Note that only low-level 
   debugging info should be sent via here.  The official route is to
   to use vg_message().  This interface is deprecated.
*/
static HChar myprintf_buf[1000];
static Int   n_myprintf_buf;

static void add_to_myprintf_buf ( HChar c )
{
   if (c == '\n' || n_myprintf_buf >= 1000-10 /*paranoia*/ ) {
      write(1/*stdout*/, myprintf_buf, vex_strlen(myprintf_buf) );
      n_myprintf_buf = 0;
      myprintf_buf[n_myprintf_buf] = 0;      
   }
   myprintf_buf[n_myprintf_buf++] = c;
   myprintf_buf[n_myprintf_buf] = 0;
}

static UInt vex_printf ( const char *format, ... )
{
   UInt ret;
   va_list vargs;
   va_start(vargs,format);
   
   n_myprintf_buf = 0;
   myprintf_buf[n_myprintf_buf] = 0;      
   ret = vprintf_wrk ( add_to_myprintf_buf, format, vargs );

   if (n_myprintf_buf > 0) {
      write(1/*stdout*/, myprintf_buf, n_myprintf_buf );
   }

   va_end(vargs);

   return ret;
}

/*---------------------------------------------------------*/
/*--- END vex_printf                                    ---*/
/*---------------------------------------------------------*/

#include <stdlib.h> // malloc etc

/* Program that checks all numbers of args (0 through 12) work for
   wrapping.  Also calls originals which trash all the iregs in an
   attempt to shake out any problems caused by insufficient saving of
   caller-save registers around the hidden call instruction. */

#define ROL(_x,n) (((_x) << n) | ((UInt)(_x)) >> ((8*sizeof(UInt)-n)))

#define TRASH_IREGS(_rlval, _vec) \
   do { \
      register UInt* vec = (_vec);   \
      /* x86 spills for v > 4, amd64 for v > 12.  Getting ppc */ \
      /* to spill is quite difficult, requiring v > 28 or so. */ \
      register UInt i, sum = 0;   \
      register UInt v1 = vec[1-1];   \
      register UInt v2 = vec[2-1];   \
      register UInt v3 = vec[3-1];   \
      register UInt v4 = vec[4-1];   \
      register UInt v5 = vec[5-1];   \
      register UInt v6 = vec[6-1];   \
      register UInt v7 = vec[7-1];   \
      register UInt v8 = vec[8-1];   \
      register UInt v9 = vec[9-1];   \
      register UInt v10 = vec[10-1];   \
      register UInt v11 = vec[11-1];   \
      register UInt v12 = vec[12-1];   \
      register UInt v13 = vec[13-1];   \
      register UInt v14 = vec[14-1];   \
      register UInt v15 = vec[15-1];   \
      register UInt v16 = vec[16-1];   \
      register UInt v17 = vec[17-1];   \
      register UInt v18 = vec[18-1];   \
      register UInt v19 = vec[19-1];   \
      register UInt v20 = vec[20-1];   \
      register UInt v21 = vec[21-1];   \
      register UInt v22 = vec[22-1];   \
      register UInt v23 = vec[23-1];   \
      register UInt v24 = vec[24-1];   \
      register UInt v25 = vec[25-1];   \
      register UInt v26 = vec[26-1];   \
      register UInt v27 = vec[27-1];   \
      register UInt v28 = vec[28-1];   \
      register UInt v29 = vec[29-1];   \
      for (i = 0; i < 50; i++) {   \
         v1  = ROL(v1,1);   \
         v2  = ROL(v2,2);   \
         v3  = ROL(v3,3);   \
         v4  = ROL(v4,4);   \
         v5  = ROL(v5,5);   \
         v6  = ROL(v6,6);   \
         v7  = ROL(v7,7);   \
         v8  = ROL(v8,8);   \
         v9  = ROL(v9,9);   \
         v10 = ROL(v10,10);   \
         v11 = ROL(v11,11);   \
         v12 = ROL(v12,12);   \
         v13 = ROL(v13,13);   \
         v14 = ROL(v14,14);   \
         v15 = ROL(v15,15);   \
         v16 = ROL(v16,16);   \
         v17 = ROL(v17,17);   \
         v18 = ROL(v18,18);   \
         v19 = ROL(v19,19);   \
         v20 = ROL(v20,20);   \
         v21 = ROL(v21,21);   \
         v22 = ROL(v22,22);   \
         v23 = ROL(v23,23);   \
         v24 = ROL(v24,24);   \
         v25 = ROL(v25,25);   \
         v26 = ROL(v26,26);   \
         v27 = ROL(v27,27);   \
         v28 = ROL(v28,28);   \
         v29 = ROL(v29,29);   \
         sum ^= ((0xFFF & v1) * i);   \
         sum ^= (v1-v2);   \
         sum ^= (v1-v3);   \
         sum ^= (v1-v4);   \
         sum ^= (v1-v5);   \
         sum ^= (v1-v6);   \
         sum ^= (v1-v7);   \
         sum ^= (v1-v8);   \
         sum ^= (v1-v9);   \
         sum ^= (v1-v10);   \
         sum ^= (v1-v11);   \
         sum ^= (v1-v12);   \
         sum ^= (v1-v13);   \
         sum ^= (v1-v14);   \
         sum ^= (v1-v15);   \
         sum ^= (v1-v16);   \
         sum ^= (v1-v17);   \
         sum ^= (v1-v18);   \
         sum ^= (v1-v19);   \
         sum ^= (v1-v20);   \
         sum ^= (v1-v21);   \
         sum ^= (v1-v22);   \
         sum ^= (v1-v23);   \
         sum ^= (v1-v24);   \
         sum ^= (v1-v25);   \
         sum ^= (v1-v26);   \
         sum ^= (v1-v27);   \
         sum ^= (v1-v28);   \
         sum ^= (v1-v29);   \
      }   \
      _rlval = sum;   \
   } while (0)


/* Returns one, in a way that gcc probably can't constant fold out */

volatile int one_actual_return_value = 0; /* the value one() returns */

__attribute__((noinline))
int one ( void )
{
   int i, sum, a[7];
   for (i = 0; i < 7; i++)
      a[i] = i;
   a[3] = 3+one_actual_return_value;
   sum = 0;
   for (i = 7-1; i >= 0; i--)
      sum += a[i] - i;
   return sum;
}

#define LOOPS_START                                                \
   { register int len = one();                                     \
     register int x0; for (x0 = 0x1000; x0 < 0x1000+len; x0++) {   \
     register int x1; for (x1 = 0x1100; x1 < 0x1100+len; x1++) {   \
     register int x2; for (x2 = 0x1200; x2 < 0x1200+len; x2++) {   \
     register int x3; for (x3 = 0x1300; x3 < 0x1300+len; x3++) {   \
     register int x4; for (x4 = 0x1400; x4 < 0x1400+len; x4++) {   \
     register int x5; for (x5 = 0x1500; x5 < 0x1500+len; x5++) {   \
     register int x6; for (x6 = 0x1600; x6 < 0x1600+len; x6++) {   \
     register int x7; for (x7 = 0x1700; x7 < 0x1700+len; x7++) {   \
     register int x8; for (x8 = 0x1800; x8 < 0x1800+len; x8++) {   \
     register int x9; for (x9 = 0x1900; x9 < 0x1900+len; x9++) {   \
     register int xA; for (xA = 0x1A00; xA < 0x1A00+len; xA++) {   \
     register int xB; for (xB = 0x1B00; xB < 0x1B00+len; xB++) {   \
     register int xC; for (xC = 0x1C00; xC < 0x1C00+len; xC++) {   \
     register int xD; for (xD = 0x1D00; xD < 0x1D00+len; xD++) {   \
     register int xE; for (xE = 0x1E00; xE < 0x1E00+len; xE++) {   \
     register int xF; for (xF = 0x1F00; xF < 0x1F00+len; xF++) {   \
     /* */

#define LOOPS_END \
     assert(xF >= 0x1F00 && xF <= 0x1F00+len); }                   \
     assert(xE >= 0x1E00 && xE <= 0x1E00+len); }                   \
     assert(xD >= 0x1D00 && xD <= 0x1D00+len); }                   \
     assert(xC >= 0x1C00 && xC <= 0x1C00+len); }                   \
     assert(xB >= 0x1B00 && xB <= 0x1B00+len); }                   \
     assert(xA >= 0x1A00 && xA <= 0x1A00+len); }                   \
     assert(x9 >= 0x1900 && x9 <= 0x1900+len); }                   \
     assert(x8 >= 0x1800 && x8 <= 0x1800+len); }                   \
     assert(x7 >= 0x1700 && x7 <= 0x1700+len); }                   \
     assert(x6 >= 0x1600 && x6 <= 0x1600+len); }                   \
     assert(x5 >= 0x1500 && x5 <= 0x1500+len); }                   \
     assert(x4 >= 0x1400 && x4 <= 0x1400+len); }                   \
     assert(x3 >= 0x1300 && x3 <= 0x1300+len); }                   \
     assert(x2 >= 0x1200 && x2 <= 0x1200+len); }                   \
     assert(x1 >= 0x1100 && x1 <= 0x1100+len); }                   \
     assert(x0 >= 0x1000 && x0 <= 0x1000+len); }                   \
   }

/* General idea is for the wrappers to use LOOPS_START / LOOPS_END to
   soak up lots of int registers.  And the orig fn uses TRASH_IREGS to
   do the same.  If there is insufficient saving of caller-saves regs
   by the CALL_FN_* macros, then hopefully the assertions in LOOPS_END
   will fail. */

/* --------------- 0 --------------- */  

__attribute__((noinline))
UInt fn_0 ( void )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 1 --------------- */  

__attribute__((noinline))
UInt fn_1 ( UInt a1 )
{
   UInt  r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 2 --------------- */  

__attribute__((noinline))
UInt fn_2 ( UInt a1, UInt a2 )
{
   UInt r = 0;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 3 --------------- */  

__attribute__((noinline))
UInt fn_3 ( UInt a1, UInt a2, UInt a3 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 4 --------------- */  

__attribute__((noinline))
UInt fn_4 ( UInt a1, UInt a2, UInt a3, UInt a4 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 5 --------------- */  

__attribute__((noinline))
UInt fn_5 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 6 --------------- */  

__attribute__((noinline))
UInt fn_6 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 7 --------------- */  

__attribute__((noinline))
UInt fn_7 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
            UInt a7 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 8 --------------- */  

__attribute__((noinline))
UInt fn_8 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
            UInt a7, UInt a8 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   words[8-1] = a8;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 9 --------------- */  

__attribute__((noinline))
UInt fn_9 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
            UInt a7, UInt a8, UInt a9 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   words[8-1] = a8;
   words[9-1] = a9;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 10 --------------- */  

__attribute__((noinline))
UInt fn_10 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
             UInt a7, UInt a8, UInt a9, UInt a10 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   words[8-1] = a8;
   words[9-1] = a9;
   words[10-1] = a10;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 11 --------------- */  

__attribute__((noinline))
UInt fn_11 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
             UInt a7, UInt a8, UInt a9, UInt a10, UInt a11 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   words[8-1] = a8;
   words[9-1] = a9;
   words[10-1] = a10;
   words[11-1] = a11;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- 12 --------------- */  

__attribute__((noinline))
UInt fn_12 ( UInt a1, UInt a2, UInt a3, UInt a4, UInt a5, UInt a6,
             UInt a7, UInt a8, UInt a9, UInt a10, UInt a11, UInt a12 )
{
   UInt r;
   UInt* words = calloc(200, sizeof(UInt));
   words[1-1] = a1;
   words[2-1] = a2;
   words[3-1] = a3;
   words[4-1] = a4;
   words[5-1] = a5;
   words[6-1] = a6;
   words[7-1] = a7;
   words[8-1] = a8;
   words[9-1] = a9;
   words[10-1] = a10;
   words[11-1] = a11;
   words[12-1] = a12;
   TRASH_IREGS(r, words);
   free(words);
   return r;
}

/* --------------- main --------------- */  

int main ( void )
{
   UInt w;

   one_actual_return_value = 1;

   vex_printf("fn_0  ...\n");
   w = fn_0();
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_1  ...\n");
   w = fn_1(42);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_2  ...\n");
   w = fn_2(42,43);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_3  ...\n");
   w = fn_3(42,43,44);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_4  ...\n");
   w = fn_4(42,43,44,45);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_5  ...\n");
   w = fn_5(42,43,44,45,46);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_6  ...\n");
   w = fn_6(42,43,44,45,46,47);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_7  ...\n");
   w = fn_7(42,43,44,45,46,47,48);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_8  ...\n");
   w = fn_8(42,43,44,45,46,47,48,49);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_9  ...\n");
   w = fn_9(42,43,44,45,46,47,48,49,50);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_10 ...\n");
   w = fn_10(42,43,44,45,46,47,48,49,50,51);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_11 ...\n");
   w = fn_11(42,43,44,45,46,47,48,49,50,51,52);
   vex_printf("      ...  %d\n\n", (int)w);

   vex_printf("fn_12 ...\n");
   w = fn_12(42,43,44,45,46,47,48,49,50,51,52,53);
   vex_printf("      ...  %d\n\n", (int)w);

   return 0;
}
