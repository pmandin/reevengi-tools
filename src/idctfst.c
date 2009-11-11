/*
 * jidctfst.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a fast, not so accurate integer implementation of the
 * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
 * must also perform dequantization of the input coefficients.
 *
 * A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
 * on each row (or vice versa, but it's more convenient to emit a row at
 * a time).  Direct algorithms are also available, but they are much more
 * complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on Arai, Agui, and Nakajima's algorithm for
 * scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
 * Japanese, but the algorithm is described in the Pennebaker & Mitchell
 * JPEG textbook (see REFERENCES section in file README).  The following code
 * is based directly on figure 4-8 in P&M.
 * While an 8-point DCT cannot be done in less than 11 multiplies, it is
 * possible to arrange the computation so that many of the multiplies are
 * simple scalings of the final outputs.  These multiplies can then be
 * folded into the multiplications or divisions by the JPEG quantization
 * table entries.  The AA&N method leaves only 5 multiplies and 29 adds
 * to be done in the DCT itself.
 * The primary disadvantage of this method is that with fixed-point math,
 * accuracy is lost due to imprecise representation of the scaled
 * quantization values.  The smaller the quantization table entry, the less
 * precise the scaled value, so this implementation does worse with high-
 * quality-setting files than with low-quality ones.
 */

/*
 * This module is specialized to the case DCTSIZE = 8.
 */

/* Scaling decisions are generally the same as in the LL&M algorithm;
 * see jidctint.c for more details.  However, we choose to descale
 * (right shift) multiplication products as soon as they are formed,
 * rather than carrying additional fractional bits into subsequent additions.
 * This compromises accuracy slightly, but it lets us save a few shifts.
 * More importantly, 16-bit arithmetic is then adequate (for 8-bit samples)
 * everywhere except in the multiplications proper; this saves a good deal
 * of work on 16-bit-int machines.
 *
 * The dequantized coefficients are not integers because the AA&N scaling
 * factors have been incorporated.  We represent them scaled up by PASS1_BITS,
 * so that the first and second IDCT rounds have the same input scaling.
 * For 8-bit JSAMPLEs, we choose IFAST_SCALE_BITS = PASS1_BITS so as to
 * avoid a descaling shift; this compromises accuracy rather drastically
 * for small quantization table entries, but it saves a lot of shifts.
 * For 12-bit JSAMPLEs, there's no hope of using 16x16 multiplies anyway,
 * so we use a much larger scaling factor to preserve accuracy.
 *
 * A final compromise is to represent the multiplicative constants to only
 * 8 fractional bits, rather than 13.  This saves some shifting work on some
 * machines, and may also reduce the cost of multiplication (since there
 * are fewer one-bits in the constants).
 */

#define	BITS_IN_JSAMPLE	8

#if BITS_IN_JSAMPLE == 8
#define CONST_BITS  8
#define PASS1_BITS  2
#else
#define CONST_BITS  8
#define PASS1_BITS  1		/* lose a little precision to avoid overflow */
#endif

/* Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.
 * (With a reasonable C compiler, you can just rely on the FIX() macro...)
 */

#if CONST_BITS == 8
#define FIX_1_082392200  (277)		/* FIX(1.082392200) */
#define FIX_1_414213562  (362)		/* FIX(1.414213562) */
#define FIX_1_847759065  (473)		/* FIX(1.847759065) */
#define FIX_2_613125930  (669)		/* FIX(2.613125930) */
#else
#define FIX_1_082392200  FIX(1.082392200)
#define FIX_1_414213562  FIX(1.414213562)
#define FIX_1_847759065  FIX(1.847759065)
#define FIX_2_613125930  FIX(2.613125930)
#endif


/* We can gain a little more speed, with a further compromise in accuracy,
 * by omitting the addition in a descaling shift.  This yields an incorrectly
 * rounded result half the time...
 */


/* Multiply a DCTELEM variable by an INT32 constant, and immediately
 * descale to yield a DCTELEM result.
 */

#define MULTIPLY(var,const)  (DESCALE((var) * (const), CONST_BITS))


/* Dequantize a coefficient by multiplying it by the multiplier-table
 * entry; produce a DCTELEM result.  For 8-bit data a 16x16->16
 * multiplication will do.  For 12-bit data, the multiplier table is
 * declared INT32, so a 32-bit multiply will be used.
 */

#if BITS_IN_JSAMPLE == 8
#define DEQUANTIZE(coef,quantval)  (coef)
#else
#define DEQUANTIZE(coef,quantval)  \
	DESCALE((coef), IFAST_SCALE_BITS-PASS1_BITS)
#endif


/* Like DESCALE, but applies to a DCTELEM and produces an int.
 * We assume that int right shift is unsigned if INT32 right shift is.
 */

#define DESCALE(x,n)  ((x)>>(n))
#define	RANGE(n)	(n)
#define	BLOCK	int

/*
 * Perform dequantization and inverse DCT on one block of coefficients.
 */
#define	DCTSIZE	8
#define	DCTSIZE2	64

static void IDCT1(BLOCK *block)
{
	int val = RANGE(DESCALE(block[0], PASS1_BITS+3));
	int i;
	for(i=0;i<DCTSIZE2;i++) block[i]=val;
}

void IDCT(BLOCK *block,int k)
{
  int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  int z5, z10, z11, z12, z13;
  BLOCK *ptr;
  int i;

  /* Pass 1: process columns from input, store into work array. */
  switch(k){
  case 1:IDCT1(block); return;
  }

  ptr = block;
  for (i = 0; i< DCTSIZE; i++,ptr++) {
    /* Due to quantization, we will usually find that many of the input
     * coefficients are zero, especially the AC terms.  We can exploit this
     * by short-circuiting the IDCT calculation for any column in which all
     * the AC terms are zero.  In that case each output is equal to the
     * DC coefficient (with scale factor as needed).
     * With typical images and quantization tables, half or more of the
     * column DCT calculations can be simplified this way.
     */
    
    if ((ptr[DCTSIZE*1] | ptr[DCTSIZE*2] | ptr[DCTSIZE*3] |
	 ptr[DCTSIZE*4] | ptr[DCTSIZE*5] | ptr[DCTSIZE*6] |
	 ptr[DCTSIZE*7]) == 0) {
      /* AC terms all zero */
      ptr[DCTSIZE*0] = 
      ptr[DCTSIZE*1] = 
      ptr[DCTSIZE*2] = 
      ptr[DCTSIZE*3] = 
      ptr[DCTSIZE*4] = 
      ptr[DCTSIZE*5] = 
      ptr[DCTSIZE*6] = 
      ptr[DCTSIZE*7] = 
      	ptr[DCTSIZE*0];
      
      continue;
    }
    
    /* Even part */

    z10 = ptr[DCTSIZE*0] + ptr[DCTSIZE*4];	/* phase 3 */
    z11 = ptr[DCTSIZE*0] - ptr[DCTSIZE*4];
    z13 = ptr[DCTSIZE*2] + ptr[DCTSIZE*6];	/* phases 5-3 */
    z12 = MULTIPLY(ptr[DCTSIZE*2] - ptr[DCTSIZE*6], FIX_1_414213562) - z13; /* 2*c4 */

    tmp0 = z10 + z13;	/* phase 2 */
    tmp3 = z10 - z13;
    tmp1 = z11 + z12;
    tmp2 = z11 - z12;
    
    /* Odd part */

    z13 = ptr[DCTSIZE*3] + ptr[DCTSIZE*5];		/* phase 6 */
    z10 = ptr[DCTSIZE*3] - ptr[DCTSIZE*5];
    z11 = ptr[DCTSIZE*1] + ptr[DCTSIZE*7];
    z12 = ptr[DCTSIZE*1] - ptr[DCTSIZE*7];

    z5 = MULTIPLY(z12 - z10, FIX_1_847759065);
    tmp7 = z11 + z13;		/* phase 5 */
    tmp6 = MULTIPLY(z10, FIX_2_613125930) + z5 - tmp7;	/* phase 2 */
    tmp5 = MULTIPLY(z11 - z13, FIX_1_414213562) - tmp6;
    tmp4 = MULTIPLY(z12, FIX_1_082392200) - z5 + tmp5;

    ptr[DCTSIZE*0] = (tmp0 + tmp7);
    ptr[DCTSIZE*7] = (tmp0 - tmp7);
    ptr[DCTSIZE*1] = (tmp1 + tmp6);
    ptr[DCTSIZE*6] = (tmp1 - tmp6);
    ptr[DCTSIZE*2] = (tmp2 + tmp5);
    ptr[DCTSIZE*5] = (tmp2 - tmp5);
    ptr[DCTSIZE*4] = (tmp3 + tmp4);
    ptr[DCTSIZE*3] = (tmp3 - tmp4);

  }
  
  /* Pass 2: process rows from work array, store into output array. */
  /* Note that we must descale the results by a factor of 8 == 2**3, */
  /* and also undo the PASS1_BITS scaling. */

  ptr = block;
  for (i = 0; i < DCTSIZE; i++ ,ptr+=DCTSIZE) {
    /* Rows of zeroes can be exploited in the same way as we did with columns.
     * However, the column calculation has created many nonzero AC terms, so
     * the simplification applies less often (typically 5% to 10% of the time).
     * On machines with very fast multiplication, it's possible that the
     * test takes more time than it's worth.  In that case this section
     * may be commented out.
     */
    
#ifndef NO_ZERO_ROW_TEST
    if ((ptr[1] | ptr[2] | ptr[3] | ptr[4] | ptr[5] | ptr[6] |
	 ptr[7]) == 0) {
      /* AC terms all zero */
      ptr[0] = 
      ptr[1] = 
      ptr[2] = 
      ptr[3] = 
      ptr[4] = 
      ptr[5] = 
      ptr[6] = 
      ptr[7] = 
      	RANGE(DESCALE(ptr[0], PASS1_BITS+3));;

      continue;
    }
#endif
    
    /* Even part */

    z10 = ptr[0] + ptr[4];
    z11 = ptr[0] - ptr[4];
    z13 = ptr[2] + ptr[6];
    z12 = MULTIPLY(ptr[2] - ptr[6], FIX_1_414213562) - z13;

    tmp0 = z10 + z13;
    tmp3 = z10 - z13;
    tmp1 = z11 + z12;
    tmp2 = z11 - z12;

    /* Odd part */

    z13 = ptr[3] + ptr[5];
    z10 = ptr[3] - ptr[5];
    z11 = ptr[1] + ptr[7];
    z12 = ptr[1] - ptr[7];

    z5 = MULTIPLY(z12 - z10, FIX_1_847759065);
    tmp7 = z11 + z13;		/* phase 5 */
    tmp6 = MULTIPLY(z10, FIX_2_613125930) + z5 - tmp7;	/* phase 2 */
    tmp5 = MULTIPLY(z11 - z13, FIX_1_414213562) - tmp6;
    tmp4 = MULTIPLY(z12, FIX_1_082392200) - z5 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    ptr[0] = RANGE(DESCALE(tmp0 + tmp7, PASS1_BITS+3));;
    ptr[7] = RANGE(DESCALE(tmp0 - tmp7, PASS1_BITS+3));;
    ptr[1] = RANGE(DESCALE(tmp1 + tmp6, PASS1_BITS+3));;
    ptr[6] = RANGE(DESCALE(tmp1 - tmp6, PASS1_BITS+3));;
    ptr[2] = RANGE(DESCALE(tmp2 + tmp5, PASS1_BITS+3));;
    ptr[5] = RANGE(DESCALE(tmp2 - tmp5, PASS1_BITS+3));;
    ptr[4] = RANGE(DESCALE(tmp3 + tmp4, PASS1_BITS+3));;
    ptr[3] = RANGE(DESCALE(tmp3 - tmp4, PASS1_BITS+3));;

  }
}

