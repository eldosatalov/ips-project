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




      __m512d result = _mm512_load_pd(window);
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (6 , 7, 4, 5, 2, 3, 0, 1) ; //a
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
      }
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (4 , 5, 6, 7, 0, 1, 2, 3) ; //b
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xCC, permNeighMax);
      }
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (6 , 7, 4, 5, 2, 3, 0, 1) ; //a
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
      }
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (0 , 1, 2, 3, 4, 5, 6, 7) ; //c
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xF0, permNeighMax);
      }
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (5 , 4, 7, 6, 1, 0, 3, 2) ; //d
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xCC, permNeighMax);
      }
      {
          __m512i idxNoNeigh = _mm512_set_epi64 (6 , 7, 4, 5, 2, 3, 0, 1) ;
          __m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
          __m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
          __m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;

          result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
      }

      _mm512_store_pd(wndw, result);


      // int64_t a[] = {6, 7, 4, 5, 2, 3, 0, 1};
      // int64_t b[] = {4, 5, 6, 7, 0, 1, 2, 3};
      // int64_t c[] = {0, 1, 2, 3, 4, 5, 6, 7};
      // int64_t d[] = {5, 4, 7, 6, 1, 0, 3, 2};

      int64_t a[] = {1, 0, 3, 2, 5, 4, 7, 6};
      int64_t b[] = {3, 2, 1, 0, 7, 6, 5, 4};
      int64_t c[] = {7, 6, 5, 4, 3, 2, 1, 0};
      int64_t d[] = {2, 3, 0, 1, 6, 7, 4, 2};

      __asm__ __volatile__ (
              "vpmovzxbq (%0), %%zmm0\n\t"
              "vcvtqq2pd %%zmm0, %%zmm0\n\t" // result = window
              "vmovdqa64 (%1), %%zmm1\n\t" // a[]
              "vmovdqa64 (%2), %%zmm2\n\t" // b[]
              "vmovdqa64 (%3), %%zmm3\n\t" // c[]
              "vmovdqa64 (%4), %%zmm4\n\t" // d[]
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm1, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xAA, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
              "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
              "vmovapd %%zmm6, %%zmm0\n\t"
            //}
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm2, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xCC, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
            "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
            "vmovapd %%zmm6, %%zmm0\n\t"
            //}
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm1, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xAA, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
            "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
            "vmovapd %%zmm6, %%zmm0\n\t"
            //}
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm3, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xF0, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
            "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
            "vmovapd %%zmm6, %%zmm0\n\t"
            //}
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm4, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xCC, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
            "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
            "vmovapd %%zmm6, %%zmm0\n\t"
            //}
            //{
            //__m512d permNeigh = _mm512_permutexvar_pd(idxNoNeigh , result ) ;
              "vpermpd %%zmm0, %%zmm1, %%zmm5\n\t"
            //__m512d permNeighMin = _mm512_min_pd(permNeigh , result ) ;
              "vminpd  %%zmm5, %%zmm0, %%zmm6\n\t"
            //__m512d permNeighMax = _mm512_max_pd(permNeigh , result ) ;
              "vmaxpd  %%zmm5, %%zmm0, %%zmm7\n\t"
              "mov $0xAA, %%eax\n\t"
              "kmovb %%eax, %%k1\n\t"
            //  result = _mm512_mask_mov_pd(permNeighMin , 0xAA, permNeighMax);
            "vmovapd %%zmm7, %%zmm6%{%%k1%}\n\t"
            "vmovapd %%zmm6, %%zmm0\n\t"
            //}
              "vcvtpd2qq %%zmm0, %%zmm0\n\t"
              "vpmovsqb %%zmm0, (%0)\n\t"


              ::"S"(window), "D"(a),
                "b"(b), "c"(c), "d"(d)
              : "%zmm0", "%zmm1", "%zmm2", "%zmm3", "%zmm4", "%zmm5",
                "%zmm6","%zmm7", "%eax"
      );
