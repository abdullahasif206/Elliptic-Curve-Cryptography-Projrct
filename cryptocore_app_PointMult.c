#include <asm-generic/fcntl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#include "../../include/cryptocore_ioctl_header.h"

/* Prototypes for functions used to access physical memory addresses */
int open_physical (int);
void close_physical (int);

int main(void)
{
	int dd = -1;
	int ret_val;

	__u32 trng_val = 0;
	__u32 i = 0;
	
	double seconds;
	struct timespec tstart={0,0}, tend={0,0};

	if ((dd = open_physical (dd)) == -1)
      return (-1);

// Stop TRNG and clear FIFO
	trng_val = 0x00000010;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);

// Configure Feedback Control Polynomial
	trng_val = 0x0003ffff;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CTR, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Stabilisation Time
	trng_val = 0x00000050;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSTAB, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Configure Sample Time
	trng_val = 0x00000006;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_TSAMPLE, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

// Start TRNG
	trng_val = 0x00000001;
	ret_val = ioctl(dd, IOCTL_SET_TRNG_CMD, &trng_val);
	if(ret_val != 0) {
		printf("Error occured\n");
	}

	usleep(10);


	// replace : ModExp_params_t with PointDouble_params_t
	// replace : ModExp_512_test with PointDouble_512_test
	
	// Initiate the curve
	
	// call affine_to_jacobi
	// call point doubling
	
	InitCurve_params_t InitCurve_512_test = { 512,
	1,
	0,
	{ 0xaadd9db8, 0xdbe9c48b, 0x3fd4e6ae, 0x33c9fc07, 
	  0xcb308db3, 0xb3c9d20e, 0xd6639cca, 0x70330871, 
	  0x7d4d9b00, 0x9bc66842, 0xaecda12a, 0xe6a380e6, 
	  0x2881ff2f, 0x2d82c685, 0x28aa6056, 0x583a48f3 }, // p
	{ 0x7830a331, 0x8b603b89, 0xe2327145, 0xac234cc5,
	  0x94cbdd8d, 0x3df91610, 0xa83441ca, 0xea9863bc,
	  0x2ded5d5a, 0xa8253aa1, 0x0a2ef1c9, 0x8b9ac8b5,
	  0x7f1117a7, 0x2bf2c7b9, 0xe7c1ac4d, 0x77fc94ca }, // a
	{ 0x3df91610, 0xa83441ca, 0xea9863bc, 0x2ded5d5a,
	  0xa8253aa1, 0x0a2ef1c9, 0x8b9ac8b5, 0x7f1117a7,
	  0x2bf2c7b9, 0xe7c1ac4d, 0x77fc94ca, 0xdc083e67,
	  0x984050b7, 0x5ebae5dd, 0x2809bd63, 0x8016f723 }, // b
	};
	
	affine_to_jacobi_params_t affine_to_jacobi_512_test = { 512,
	1,
	0,
	1,
	{  }, // p
	{ 0x81aee4bd, 0xd82ed964, 0x5a21322e, 0x9c4c6a93,
	  0x85ed9f70, 0xb5d916c1, 0xb43b62ee, 0xf4d0098e,
	  0xff3b1f78, 0xe2d0d48d, 0x50d1687b, 0x93b97d5f,
	  0x7c6d5047, 0x406a5e68, 0x8b352209, 0xbcb9f822 }, // x
	{ 0x7dde385d, 0x566332ec, 0xc0eabfa9, 0xcf7822fd,
	  0xf209f700, 0x24a57b1a, 0xa000c55b, 0x881f8111,
	  0xb2dcde49, 0x4a5f485e, 0x5bca4bd8, 0x8a2763ae,
	  0xd1ca2b2f, 0xa8f05406, 0x78cd1e0f, 0x3ad80892 }, // y
	{  }, // X_MD
	{  }, // Y_MD
	{  }, // Z_MD
	};
	
	/*
	
	// Q point :
	
	affine_to_jacobi_params_t affine_to_jacobi_512_test_2 = { 512,
	1,
	0,
	0,
	{  }, // p
	{ 0x81b2354c, 0x0bf74652, 0x500882c8, 0x20021b90,
	  0x7d9a46bc, 0x111315f1, 0xb1d2d915, 0xc6dcde9e,
	  0x0d9019a6, 0xa04eeff3, 0xbbba6aea, 0xe51d5968,
	  0xce2a74d2, 0x29b8851f, 0x3d1d15c8, 0x7146414b }, // x
	{ 0x803f72ee, 0x0ca52bcc, 0x020f708c, 0x3da3f996,
	  0x28681c6c, 0x870a1a28, 0x8042cc77, 0xcd2d32ec,
	  0xb129c648, 0xe6d6bf36, 0x7ed3d78c, 0xdd7e2097,
	  0x2987fcb0, 0x748d6cdf, 0xe605819d, 0xb16d1985 }, // y
	{  }, // X_MD
	{  }, // Y_MD
	{  }, // Z_MD
	};
	*/
	
	
	PointAdd_params_t PointAdd_512_test = { 512,
	1,
	0,
	{  }, // p
	{  }, // Xp
	{  }, // Yp
	{  }, // Zp
	{  }, // Xq
	{  }, // Yq
	{  }, // Zq
	{  }, // a output
	{  }, // b output
	{  }, // c output
	};
	
	PointDouble_params_t PointDouble_512_test = { 512,
	1,
	0,
	{  }, // p
	{  }, // X input
	{  }, // Y input
	{  }, // Z input
	{  }, // a output
	{  }, // b output
	{  }, // c output
	};
	
	
	jacobi_to_affine_params_t jacobi_to_affine_512_test = { 512,
	1,
	0,
	{  }, // p
	{  }, // X_MD
	{  }, // Y_MD
	{  }, // Z_MD
	{  }, // a
	{  }, // b
	};

	PointMult_params_t PointMult_512_test = { 512,
	1,
	0,
	{  }, // p
	{  }, // scalar n
	{  }, // input x
	{  }, // input y
	{  }, // input z
	{  }, // output x
	{  }, // output y
	{  }, // output z
	};
	
	
	
	// Generate random scalar n from TRNG FIRO
	i = 0;
	while (i < PointMult_512_test.prec/32) {
		ret_val = ioctl(dd, IOCTL_READ_TRNG_FIFO, &trng_val);
		if(ret_val == 0) {
			PointMult_512_test.n[i] = trng_val;
			i++;
		} else if (ret_val == -EAGAIN) {
			printf("TRNG FIFO empty\n");
		} else {
			printf("Error occured\n");
		}
	}
	
	
	printf("P : 0x");
	for(i=0; i<InitCurve_512_test.prec/32; i++){
		printf("%08x", InitCurve_512_test.p[i]);
	}
	printf("\n\n");	
	
	printf("A : 0x");
	for(i=0; i<InitCurve_512_test.prec/32; i++){
		printf("%08x", InitCurve_512_test.a[i]);
	}
	printf("\n\n");	
	
	printf("B : 0x");
	for(i=0; i<InitCurve_512_test.prec/32; i++){
		printf("%08x", InitCurve_512_test.b[i]);
	}
	printf("\n\n");	
	
	printf("x_affine: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.x[i]);
	}
	printf("\n\n");
	
	printf("y_affine: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.y[i]);
	}
	printf("\n\n");


	printf("scalar n: 0x");
	for(i=0; i<PointMult_512_test.prec/32; i++){
		printf("%08x", PointMult_512_test.n[i]);
	}
	printf("\n\n");
	
	// start timer
	clock_gettime(CLOCK_MONOTONIC, &tstart);
	
	// call InitCurve()
	ret_val = ioctl(dd, IOCTL_MWMAC_INITCURVE, &InitCurve_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	

	// call affine_to_jacobi(make_copy=1)
	ret_val = ioctl(dd, IOCTL_MWMAC_affine2jacobi, &affine_to_jacobi_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	printf("Affine to Jacobi Results : ");
	printf("\n\n");
	
	// read P_MD :
	
	printf("Xp_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.x_md[i]);
	}
	printf("\n\n");
	printf("Yp_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.y_md[i]);
	}
	printf("\n\n");
	printf("Zp_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.z_md[i]);
	}
	printf("\n\n");
	
	/*
	
	// call affine_to_jacobi(make_copy=0)
	ret_val = ioctl(dd, IOCTL_MWMAC_affine2jacobi, &affine_to_jacobi_512_test_2);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	// read Q_MD :
	
	printf("Xq_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test_2.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test_2.x_md[i]);
	}
	printf("\n\n");
	printf("Yq_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test_2.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test_2.y_md[i]);
	}
	printf("\n\n");
	printf("Zq_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test_2.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test_2.z_md[i]);
	}
	printf("\n\n");
	
	*/
	
	

	printf("Scalar n: 0x");
	for(i=0; i<PointMult_512_test.prec/32; i++){
		printf("%08x", PointMult_512_test.n[i]);
	}
	printf("\n\n");
	
	
	printf("Point Multiplication started ... :");
	printf("\n\n");

	__u32 j = 0;
	__u32 msb_found = 0;
	for(i=0; i<PointMult_512_test.prec/32; i++){
		for(j = 1<<31; j>0; j=j>>1){
			if(msb_found) {
				
				// call PointAdd()
				ret_val = ioctl(dd, IOCTL_MWMAC_PODOUBLE, &PointDouble_512_test);
				if(ret_val != 0) {
					printf("Error occured\n");
				}
				
				
				if(PointMult_512_test.n[i] & j) {
					
					// call PointAdd()
					ret_val = ioctl(dd, IOCTL_MWMAC_POADD, &PointAdd_512_test);
					if(ret_val != 0) {
						printf("Error occured\n");
					}
					
				}
			}
			else if (PointMult_512_test.n[i] & j) msb_found = 1;
		}
	}

	
	
	
	
	
	/*
	
	// call PointAdd()
	ret_val = ioctl(dd, IOCTL_MWMAC_POADD, &PointAdd_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	printf("Point Addition Result : ");
	printf("\n\n");
	// read result from a (X)
	printf("X: 0x");
	for(i=0; i<PointAdd_512_test.prec/32; i++){
		printf("%08x", PointAdd_512_test.a[i]);
	}
	printf("\n\n");

	// read result from b (Y)
	printf("Y: 0x");
	for(i=0; i<PointAdd_512_test.prec/32; i++){
		printf("%08x", PointAdd_512_test.b[i]);
	}
	printf("\n\n");
	
	// read result from c (Z)
	printf("Z: 0x");
	for(i=0; i<PointAdd_512_test.prec/32; i++){
		printf("%08x", PointAdd_512_test.c[i]);
	}
	printf("\n\n");
	
	
	*/
	
	// call Jacobi_to_affine
	ret_val = ioctl(dd, IOCTL_MWMAC_jacobi2affine, &jacobi_to_affine_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	
	
	printf("Jacobi to Affine Result : ");
	printf("\n\n");
	
	
	
	// read result from b (Y)
	printf("x_affine: 0x");
	for(i=0; i<jacobi_to_affine_512_test.prec/32; i++){
		printf("%08x", jacobi_to_affine_512_test.a[i]);
	}
	printf("\n\n");
	
	// read result from c (Z)
	printf("y_affine: 0x");
	for(i=0; i<jacobi_to_affine_512_test.prec/32; i++){
		printf("%08x", jacobi_to_affine_512_test.b[i]);
	}
	printf("\n\n");
	
	
	
	
	
	// stop timer
	clock_gettime(CLOCK_MONOTONIC, &tend);
	
	/*
	
	printf("Affine to Jacobi (Copy) Result : ");
	printf("\n\n");
	
	// read result from a (X)
	printf("X_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.x_md[i]);
	}
	printf("\n\n");
	
	// read result from a (X)
	printf("Y_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.y_md[i]);
	}
	printf("\n\n");
	
	// read result from a (X)
	printf("Z_MD: 0x");
	for(i=0; i<affine_to_jacobi_512_test.prec/32; i++){
		printf("%08x", affine_to_jacobi_512_test.z_md[i]);
	}
	printf("\n\n");
*/



	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("PointDouble 512 took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("PointDouble 512 took about %.5f us\n\n", seconds*1000000.0);	
/*
	// Clear a
	for(i=0; i<PointDouble_512_test.prec/32; i++){
		PointDouble_512_test.a[i] = 0;
	}	
	
	PointDouble_512_test.sec_calc = 1;		

	clock_gettime(CLOCK_MONOTONIC, &tstart);
	ret_val = ioctl(dd, IOCTL_MWMAC_PODOUBLE, &PointDouble_512_test);
	if(ret_val != 0) {
		printf("Error occured\n");
	}
	clock_gettime(CLOCK_MONOTONIC, &tend);

	printf("C = ModExp(R,R,E,B,P): 0x");
	for(i=0; i<PointDouble_512_test.prec/32; i++){
		printf("%08x", PointDouble_512_test.a[i]);
	}
	printf("\n\n");

	seconds = ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
	if (seconds*1000000.0 > 1000.0)
		printf("ModExp 512 (sec calc) took about %.5f ms\n\n", seconds*1000.0);
	else 
		printf("ModExp 512 (sec calc) took about %.5f us\n\n", seconds*1000000.0);
	
*/
	close_physical (dd);   // close /dev/cryptocore
	return 0;
}

// Open /dev/cryptocore, if not already done, to give access to physical addresses
int open_physical (int dd)
{
   if (dd == -1)
      if ((dd = open( "/dev/cryptocore", (O_RDWR | O_SYNC))) == -1)
      {
         printf ("ERROR: could not open \"/dev/cryptocore\"...\n");
         return (-1);
      }
   return dd;
}

// Close /dev/cryptocore to give access to physical addresses
void close_physical (int dd)
{
   close (dd);
}
