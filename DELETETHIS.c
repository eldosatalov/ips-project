float zmm_coeff[]=
    {
        0.272f, 0.349f, 0.393f,  0.272f, 0.349f, 0.393f,  0.272f, 0.349f, 0.393f,  0.272f, 0.349f, 0.393f,  0.272f, 0.349f, 0.393f, 1.0f,
        0.534f, 0.686f, 0.769f,  0.534f, 0.686f, 0.769f,  0.534f, 0.686f, 0.769f,  0.534f, 0.686f, 0.769f,  0.534f, 0.686f, 0.769f, 1.0f,
        0.131f, 0.168f, 0.189f,  0.131f, 0.168f, 0.189f,  0.131f, 0.168f, 0.189f,  0.131f, 0.168f, 0.189f,  0.131f, 0.168f, 0.189f, 1.0f,
    };

    // Bs = 0.272 * R + 0.534 * G + 0.131 * B
float B[15];
float G[15];
float R[15];

for(int i = 0; i < 45; i+=3){
  B[i] = (float)pixels[position + i];
}

for(int i = 1; i < 45; i+=3){
  G[i] = (float)pixels[position + i];
}

for(int i = 2; i < 45; i+=3){
  R[i] = (float)pixels[position + i];
}

__asm__ __volatile__ (
    // loading coefficients to zmm 1 2 3
    "vmovups (%2), %%zmm1\n\t"      // R
    "vmovups 0x40(%2), %%zmm2\n\t"  // G
    "vmovups 0x80(%2), %%zmm3\n\t"  // B

    // storing
    //"movb 0x40(%1, %2), %%al\n\t"

    //loading B G R pixels to zmm 5 6 7 respectevly
    "vmovups (%3), %%zmm5\n\t" // B
    "vmovups (%4), %%zmm6\n\t" // G
    "vmovups (%5), %%zmm7\n\t" // R

    /*
      *R = R_coeff * B
      *G = G_coeff * G
      *B = B_coeff * B
    */
    "vmulps %%zmm1, %%zmm5, %%zmm7\n\t"
    "vmulps %%zmm2, %%zmm6, %%zmm6\n\t"
    "vmulps %%zmm3, %%zmm5, %%zmm5\n\t"

    /*
      zmm0  = *R + *G + *B
    */
    "vaddps %%zmm7, %%zmm6, %%zmm0\n\t"
    "vaddps %%zmm5, %%zmm0, %%zmm0\n\t"



    //"vcvtps2dq %%zmm0, %%zmm0\n\t"


    ::
      "D"(pixels), "S"(position), // 0 1
      "d"(zmm_coeff), "a"(B),     // 2 3
      "b"(G), "c"(R)              // 4 5
    :
    "%zmm1", "%zmm2","%zmm3","%zmm0"
);
