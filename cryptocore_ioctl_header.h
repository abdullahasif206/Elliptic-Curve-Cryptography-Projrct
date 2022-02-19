/* 
* cryptocore_ioctl_header.h - the header file with the ioctl definitions.
* The declarations here have to be in a header file, because
* they need to be known both the kernel module in *_driver.c
* and the application *_app.c 
*/

#include <linux/ioctl.h>

// CryptoCore Struct Declarations:
typedef struct MontMult_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} MontMult_params_t;

typedef struct MontR_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 r[16];
} MontR_params_t;

typedef struct MontR2_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 r2[16];
} MontR2_params_t;

typedef struct MontExp_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 e[16];
	__u32 c[16];
} MontExp_params_t;

typedef struct ModAdd_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} ModAdd_params_t;

typedef struct ModSub_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 b[16];
	__u32 c[16];
} ModSub_params_t;

typedef struct CopyH2V_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyH2V_params_t;

typedef struct CopyV2V_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyV2V_params_t;

typedef struct CopyH2H_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyH2H_params_t;

typedef struct CopyV2H_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 a[16];
	__u32 acopy[16];
} CopyV2H_params_t;

typedef struct MontMult1_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 c[16];
} MontMult1_params_t;

typedef struct ModExp_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 b[16];
	__u32 e[16];
	__u32 c[16];
} ModExp_params_t;


typedef struct PointDouble_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 x[16]; // input x
	__u32 y[16]; // input y
	__u32 z[16]; // input z
	__u32 a[16]; // output x
	__u32 b[16]; // output y
	__u32 c[16]; // output z
} PointDouble_params_t;

typedef struct PointAdd_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 x1[16]; // input P.x
	__u32 y1[16]; // input P.y
	__u32 z1[16]; // input P.z
	__u32 x2[16]; // input Q.x
	__u32 y2[16]; // input Q.y
	__u32 z2[16]; // input Q.z
	__u32 a[16]; // output x
	__u32 b[16]; // output y
	__u32 c[16]; // output z
} PointAdd_params_t;

/*
typedef struct PointAdd_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 xp[16]; // input P.x
	__u32 yp[16]; // input P.y
	__u32 xq[16]; // input Q.x
	__u32 yq[16]; // input Q.y
	__u32 a[16]; // output x
	__u32 b[16]; // output y
	__u32 c[16]; // output z
} PointAdd_params_t;
*/
typedef struct PointMult_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 n[16]; // scalar n
	__u32 x[16]; // input x
	__u32 y[16]; // input y
	__u32 z[16]; // input z
	__u32 a[16]; // output x
	__u32 b[16]; // output y
	__u32 c[16]; // output z
} PointMult_params_t;

typedef struct jacobi_to_affine_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 x[16]; // input x
	__u32 y[16]; // input y
	__u32 z[16]; // input z
	__u32 a[16]; // output a
	__u32 b[16]; // output b
} jacobi_to_affine_params_t;


typedef struct affine_to_jacobi_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 make_copy;
	__u32 p[16];
	__u32 x[16]; // input x
	__u32 y[16]; // input y
	__u32 x_md[16]; // output x_md
	__u32 y_md[16]; // output y_md
	__u32 z_md[16]; // output z_md
} affine_to_jacobi_params_t;

typedef struct ModRed_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 n[16];
	__u32 a[16];
	__u32 c[16];
} ModRed_params_t ;

typedef struct InitCurve_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 a[16];
	__u32 b[16];
} InitCurve_params_t ;

/*

typedef struct InitCurve_params{
	__u32 prec;
	__u32 f_sel;
	__u32 sec_calc;
	__u32 p[16];
	__u32 p_2[16];
	__u32 a[16];
	__u32 b[16];
} InitCurve_params_t ;

*/

// Add CryptoCore Struct Declarations here...



#define IOCTL_BASE 'k' 					// magic number

// NOTE: magic | cmdnumber | size of data to pass
#define 	IOCTL_SET_TRNG_CMD			_IOW(IOCTL_BASE,   1, __u32)
#define 	IOCTL_SET_TRNG_CTR			_IOW(IOCTL_BASE,   2, __u32)
#define 	IOCTL_SET_TRNG_TSTAB		_IOW(IOCTL_BASE,   3, __u32)
#define 	IOCTL_SET_TRNG_TSAMPLE		_IOW(IOCTL_BASE,   4, __u32)
#define 	IOCTL_READ_TRNG_FIFO		_IOR(IOCTL_BASE,   5, __u32)

#define		IOCTL_MWMAC_MONTMULT		_IOWR(IOCTL_BASE,  6, MontMult_params_t)
#define		IOCTL_MWMAC_MONTR			_IOWR(IOCTL_BASE,  7, MontR_params_t)
#define		IOCTL_MWMAC_MONTR2			_IOWR(IOCTL_BASE,  8, MontR2_params_t)
#define		IOCTL_MWMAC_MONTEXP			_IOWR(IOCTL_BASE,  9, MontExp_params_t)
#define		IOCTL_MWMAC_MODADD			_IOWR(IOCTL_BASE, 10, ModAdd_params_t)
#define		IOCTL_MWMAC_MODSUB			_IOWR(IOCTL_BASE, 11, ModSub_params_t)
#define		IOCTL_MWMAC_COPYH2V			_IOWR(IOCTL_BASE, 12, CopyH2V_params_t)
#define		IOCTL_MWMAC_COPYV2V			_IOWR(IOCTL_BASE, 13, CopyV2V_params_t)
#define		IOCTL_MWMAC_COPYH2H			_IOWR(IOCTL_BASE, 14, CopyH2H_params_t)
#define		IOCTL_MWMAC_COPYV2H			_IOWR(IOCTL_BASE, 15, CopyV2H_params_t)
#define		IOCTL_MWMAC_MONTMULT1		_IOWR(IOCTL_BASE, 16, MontMult1_params_t)
#define		IOCTL_MWMAC_MONTEXP_FULL	_IOWR(IOCTL_BASE, 17, MontExp_params_t)
#define		IOCTL_MWMAC_MODEXP			_IOWR(IOCTL_BASE, 18, ModExp_params_t)
#define		IOCTL_MWMAC_MODRED			_IOWR(IOCTL_BASE, 19, ModRed_params_t)
#define		IOCTL_MWMAC_PODOUBLE		_IOWR(IOCTL_BASE, 20, PointDouble_params_t)
#define     IOCTL_MWMAC_affine2jacobi   _IOWR(IOCTL_BASE, 21, affine_to_jacobi_params_t)
#define		IOCTL_MWMAC_POADD			_IOWR(IOCTL_BASE, 22, PointAdd_params_t)
#define		IOCTL_MWMAC_POMULT			_IOWR(IOCTL_BASE, 23, PointMult_params_t)
#define		IOCTL_MWMAC_jacobi2affine	_IOWR(IOCTL_BASE, 24, jacobi_to_affine_params_t)
#define		IOCTL_MWMAC_INITCURVE		_IOWR(IOCTL_BASE, 25, InitCurve_params_t)
// Define further IOCTL commands here...
