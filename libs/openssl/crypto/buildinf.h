#ifndef MK1MF_BUILD
  /* auto-generated by Configure for crypto/cversion.c:
   * for Unix builds, crypto/Makefile.ssl generates functional definitions;
   * Windows builds (and other mk1mf builds) compile cversion.c with
   * -DMK1MF_BUILD and use definitions added to this file by util/mk1mf.pl. */
  #error "Windows builds (PLATFORM=VC-WIN32) use mk1mf.pl-created Makefiles"
#endif
#ifdef MK1MF_PLATFORM_VC_WIN32
  /* auto-generated/updated by util/mk1mf.pl for crypto/cversion.c */
  #define CFLAGS "compiler: cl -Ox -Ob1 -Oi -Os -Oy -GF -GS- -Gy -DNDEBUG;OPENSSL_NO_CAPIENG;NO_CHMOD;OPENSSL_NO_DGRAM;OPENSSL_NO_RIJNDAEL;DSO_WIN32  /MD /Ox /O2 /Ob2 -DOPENSSL_THREADS  -DDSO_WIN32 -Ox -Ob1 -Oi -Os -Oy -GF -GS- -Gy -DNDEBUG;OPENSSL_NO_CAPIENG;NO_CHMOD;OPENSSL_NO_DGRAM;OPENSSL_NO_RIJNDAEL;DSO_WIN32 -DOPENSSL_NO_ERR -W3 -Gs0 -GF -Gy -nologo -DOPENSSL_SYSNAME_WIN32 -DWIN32_LEAN_AND_MEAN -DL_ENDIAN -D_CRT_SECURE_NO_DEPRECATE -DOPENSSL_USE_APPLINK -I. -DOPENSSL_NO_IDEA -DOPENSSL_NO_CAMELLIA -DOPENSSL_NO_SEED -DOPENSSL_NO_RC4 -DOPENSSL_NO_RC5 -DOPENSSL_NO_MD2 -DOPENSSL_NO_RIPEMD -DOPENSSL_NO_BF -DOPENSSL_NO_CAST -DOPENSSL_NO_WHIRLPOOL -DOPENSSL_NO_SRP -DOPENSSL_NO_ERR -DOPENSSL_NO_KRB5 -DOPENSSL_NO_GOST -DOPENSSL_NO_HW -DOPENSSL_NO_JPAKE -DOPENSSL_NO_DYNAMIC_ENGINE    "
  #define PLATFORM "VC-WIN32"
  #define DATE "Thu Jan 28 19:19:34 2016"
#endif