AC_INIT([src/mednafen.cpp])
AC_GNU_SOURCE
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_MACRO_DIR([m4])

#
# Avoid trailing and leading zeroes in the decimal version components to avoid confusing not-so-learned people.
#
MEDNAFEN_VERSION='1.32.1'
MEDNAFEN_VERSION_NUMERIC=0x00103201
#                        0xJJJnnnRR

AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([mednafen], $MEDNAFEN_VERSION)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AC_CONFIG_HEADERS([include/config.h:include/config.h.in])

AC_PROG_CC
AC_PROG_CPP
AM_PROG_AS
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CXXCPP

AC_LANG_C
CC="$CC -std=gnu99"
CPP="$CPP -std=gnu99"
AX_APPEND_COMPILE_FLAGS([-fsigned-char -fwrapv], CC)

AC_LANG_CPLUSPLUS
CXX="$CXX -std=gnu++11"
CXXCPP="$CXXCPP -std=gnu++11"
AX_APPEND_COMPILE_FLAGS([-fsigned-char -fwrapv -fexceptions], CXX)

AC_PROG_INSTALL

gl_VISIBILITY

AC_SYS_LARGEFILE
AC_DEFINE(_LARGEFILE64_SOURCE, [1], [Define for largefile support through extra functions.])
AC_DEFINE(_LARGEFILE_SOURCE, [1], [Define for fseeko and ftello on some hosts.])

# sizeof tests need to come after largefile support macros

AC_CHECK_TYPE(ptrdiff_t,long)
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(__int64)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(size_t)
AC_CHECK_SIZEOF(ptrdiff_t)
AC_CHECK_SIZEOF(off_t)
AC_C_CONST
AC_CHECK_FUNCS(memcmp memcpy memmove memset)
AC_CHECK_FUNCS(mmap munmap madvise)
AC_CHECK_FUNCS(signal sigaction siglongjmp _longjmp fork getpwuid getuid)
AC_CHECK_FUNCS(getenv putenv setenv)
AC_CHECK_FUNCS(pipe dup2 fcntl lseek64 fstat64 ftruncate64)
AC_CHECK_FUNCS(strerror strerror_r __mingw_get_crt_info)

AC_SEARCH_LIBS(fmodl, [m])

#
# Check for any libraries that need to be linked against for clock_gettime().
#
# (posix4 is apparently used by much older versions of Solaris according to tar, pre-7, so maybe it's not necessary
#  for Mednafen?)
AC_SEARCH_LIBS(clock_gettime, [rt posix4])
AC_CHECK_FUNCS(clock_gettime nanosleep usleep localtime_r gmtime_r gettimeofday)

AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(stdint.h inttypes.h)
AC_FUNC_MKDIR
PKG_PROG_PKG_CONFIG

AC_CHECK_FUNCS(round nearbyint nearbyintf rint rintf)
AC_CHECK_HEADERS(fenv.h)


AC_DEFINE(MINILZO_HAVE_CONFIG_H, [1], [Define if config.h is present])

AM_ICONV
if test "x$am_cv_func_iconv" != "xyes"; then
	AC_MSG_ERROR([*** iconv not found!])
fi
AM_GNU_GETTEXT

AC_SUBST(LIBICONV)
AC_SUBST(LIBINTL)

dnl Check for zlib
AC_CHECK_LIB([z], [zlibVersion], ZLIB_LIBS="-lz", AC_MSG_ERROR([*** zlib not found!]))
AC_SUBST([ZLIB_LIBS])


ENABLE_LIBXXX_MODE=false
AC_ARG_ENABLE(libxxx-mode, AC_HELP_STRING([--enable-libxxx-mode], [dev use only]))
if test x$enable_libxxx_mode = xyes; then
	ENABLE_LIBXXX_MODE=true
fi

AC_ARG_ENABLE(dev-build,
 AC_HELP_STRING([--enable-dev-build], [enable expensive Mednafen developer features [[default=no]]]),
                  , enable_dev_build=no)

if test x$enable_dev_build = xyes; then
                AC_DEFINE([MDFN_ENABLE_DEV_BUILD], [1], [Define if we are compiling with expensive Mednafen developer features enabled.])
fi

dnl -fno-fast-math and -fno-unsafe-math-optimizations to make sure it's disabled, as the fast-math feature on certain older
dnl versions of gcc produces horribly broken code(and even when it's working correctly, it can have somewhat unpredictable effects).
dnl
dnl -fno-aggressive-loop-optimizations because I don't trust gcc's aggressive loop optimizations, and there miiight be old code
dnl in Mednafen that would cause problems.
dnl
dnl -fomit-frame-pointer is required for some x86 inline assembly to compile.
dnl
dnl -fstrict-aliasing for OpenBSD
dnl
OPTIMIZER_FLAGS=""
AX_APPEND_COMPILE_FLAGS([-fno-fast-math], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-unsafe-math-optimizations], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-aggressive-loop-optimizations], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-ipa-icf], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-printf-return-value], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fomit-frame-pointer], OPTIMIZER_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fstrict-aliasing], OPTIMIZER_FLAGS)

dnl
dnl Use -fwrapv instead of -fno-strict-overflow; -fno-strict-overflow is buggy on gcc and does not work as documented/implied.
dnl
dnl -mfunction-return=keep, -mindirect-branch=keep, -mno-indirect-branch-register to disable performance-murdering Spectre mitigations.
dnl
CODEGEN_FLAGS=""
AX_APPEND_COMPILE_FLAGS([-fjump-tables], CODEGEN_FLAGS)
AX_APPEND_COMPILE_FLAGS([-mfunction-return=keep], CODEGEN_FLAGS)
AX_APPEND_COMPILE_FLAGS([-mindirect-branch=keep], CODEGEN_FLAGS)
AX_APPEND_COMPILE_FLAGS([-mno-indirect-branch-register], CODEGEN_FLAGS)
AX_APPEND_COMPILE_FLAGS([-mno-retpoline], CODEGEN_FLAGS)

WARNING_FLAGS=""
ac_save_CXXFLAGS="$CXXFLAGS"
AX_APPEND_COMPILE_FLAGS([-Werror=unknown-warning-option], CXXFLAGS)
AX_APPEND_COMPILE_FLAGS([-Wall], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wshadow], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wempty-body], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wignored-qualifiers], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wvla], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wvariadic-macros], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wdisabled-optimization], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wstack-usage=32768], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wframe-larger-than=32768], WARNING_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Werror=write-strings], WARNING_FLAGS)
CXXFLAGS="$ac_save_CXXFLAGS"
AC_SUBST(WARNING_FLAGS)

dnl
dnl Aggressively try to disable PIC and PIE, as it has a significant performance overhead and may
dnl break some 32-bit x86 inline assembly.
dnl
dnl A quick and dirty check is done to see if compiling and linking with PIC/PIE forcibly disabled works, and if it doesn't,
dnl PIC/PIE-disabling flags aren't used.  Fixes linking failure on Haiku; it may not catch failures in other situations due to the
dnl simplicity of the test program and some other compiler flag usage differences compared with compiling Mednafen itself.
dnl
NOPICPIE_FLAGS=""
NOPICPIE_LDFLAGS=""
AX_APPEND_COMPILE_FLAGS([-fno-pic], NOPICPIE_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-pie], NOPICPIE_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-PIC], NOPICPIE_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-PIE], NOPICPIE_FLAGS)
AX_CHECK_LINK_FLAG([-nopie], [NOPICPIE_LDFLAGS="$NOPICPIE_LDFLAGS -nopie"])
AX_CHECK_LINK_FLAG([-no-pie], [NOPICPIE_LDFLAGS="$NOPICPIE_LDFLAGS -no-pie"])

if test "x$NOPICPIE_FLAGS" != "x" || test "x$NOPICPIE_LDFLAGS" != "x"; then
	AC_MSG_CHECKING([if compiling and linking works with PIC/PIE-disabling flags])
	ac_save_CFLAGS="$CFLAGS"
	ac_save_CXXFLAGS="$CXXFLAGS"
	ac_save_LDFLAGS="$LDFLAGS"
	CFLAGS="$OPTIMIZER_FLAGS $WARNING_FLAGS $CODEGEN_FLAGS $CFLAGS"
	CXXFLAGS="$OPTIMIZER_FLAGS $WARNING_FLAGS $CODEGEN_FLAGS $CXXFLAGS"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>]], [[return puts("blackraspberrycobblerwithvanillaicecream");]])], [], [AC_MSG_ERROR([*** compiler broken?])])
        dnl Put $NOPICPIE_FLAGS after $CFLAGS/$CXXFLAGS here so the check doesn't give a misleading result,
        dnl but don't do it elsewhere.
	CFLAGS="$CFLAGS $NOPICPIE_FLAGS"
	CXXFLAGS="$CXXFLAGS $NOPICPIE_FLAGS"
	LDFLAGS="$LDFLAGS $NOPICPIE_LDFLAGS"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>]], [[return puts("blackraspberrycobblerwithvanillaicecream");]])],
		[
		 AC_MSG_RESULT([yes])
		],
		[
		 AC_MSG_RESULT([no])
		 NOPICPIE_FLAGS=""
		 NOPICPIE_LDFLAGS=""
		])
	CFLAGS="$ac_save_CFLAGS"
	CXXFLAGS="$ac_save_CXXFLAGS"
	LDFLAGS="$ac_save_LDFLAGS"
fi

#################################
#
# Used as needed in Makefile.am:
#
NO_STACK_PROTECTOR_FLAGS=""
AX_APPEND_COMPILE_FLAGS([-fno-stack-protector], NO_STACK_PROTECTOR_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-stack-protector-all], NO_STACK_PROTECTOR_FLAGS)
AX_APPEND_COMPILE_FLAGS([-fno-stack-protector-strong], NO_STACK_PROTECTOR_FLAGS)
AC_SUBST(NO_STACK_PROTECTOR_FLAGS)

# Used for ngp and quicklz
NO_STRICT_ALIASING_FLAGS=""
AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing], NO_STRICT_ALIASING_FLAGS)
AC_SUBST(NO_STRICT_ALIASING_FLAGS)

# -----------------------------------
# Begin snes flags
#
SNES_EXTRA_FLAGS=""
SNES_EXTRA_CXXFLAGS=""
AX_APPEND_COMPILE_FLAGS([-Wno-unused], SNES_EXTRA_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wno-shadow], SNES_EXTRA_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wno-sign-compare], SNES_EXTRA_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wno-uninitialized], SNES_EXTRA_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wno-parentheses], SNES_EXTRA_FLAGS)
AX_APPEND_COMPILE_FLAGS([-Wno-switch], SNES_EXTRA_FLAGS)
AC_SUBST(SNES_EXTRA_FLAGS)
AC_SUBST(SNES_EXTRA_CXXFLAGS)
#
# End snes flags
# -----------------------------------

# -----------------------------------
# Begin ss flags
#
SS_EXTRA_FLAGS=""
AX_APPEND_COMPILE_FLAGS([-mtune=haswell], SS_EXTRA_FLAGS)
AC_SUBST(SS_EXTRA_FLAGS)
#
# End ss flags
# -----------------------------------
#
# End used-as-needed flags
########################################

AM_CONDITIONAL(WANT_DEBUGGER, false)
AM_CONDITIONAL(WANT_FANCY_SCALERS, false)

AC_ARG_ENABLE(debugger,
 AC_HELP_STRING([--enable-debugger], [build with internal debugger [[default=yes]]]),
                  , enable_debugger=yes)

if test x$enable_debugger = xyes; then
                AC_DEFINE([WANT_DEBUGGER], [1], [Define if we are compiling with debugger.])
		AM_CONDITIONAL(WANT_DEBUGGER, true)
fi

AC_ARG_ENABLE(cjk-fonts,
 AC_HELP_STRING([--enable-cjk-fonts], [build with internal CJK(Chinese, Japanese, Korean) fonts [[default=yes]]]),
                  , enable_cjk_fonts=yes)

if test x$enable_cjk_fonts = xyes; then
                AC_DEFINE([WANT_INTERNAL_CJK], [1], [Define if we are compiling with internal CJK fonts.])
fi

AC_ARG_ENABLE(fancy-scalers,
 AC_HELP_STRING([--enable-fancy-scalers], [build with fancy(2xsai, hq2x, etc.) CPU-intensive software video scalers [[default=yes]]]),
                  , enable_fancy_scalers=yes)

if test x$enable_fancy_scalers = xyes; then
                AC_DEFINE([WANT_FANCY_SCALERS], [1], [Define if we are compiling with with fancy CPU-intensive software video scalers.])
		AM_CONDITIONAL(WANT_FANCY_SCALERS, true)
fi

dnl
dnl The code that uses $enable_altivec is lower, in the CPU architecture section.
dnl
AC_ARG_ENABLE(altivec,
 AC_HELP_STRING([--enable-altivec], [use AltiVec extensions on PowerPC/POWER ISA processors [[default=yes]]]),
                  , enable_altivec=yes)


AM_CONDITIONAL(WANT_APPLE2_EMU, false)
AM_CONDITIONAL(WANT_GB_EMU, false)
AM_CONDITIONAL(WANT_GBA_EMU, false)
AM_CONDITIONAL(WANT_LYNX_EMU, false)
AM_CONDITIONAL(WANT_MD_EMU, false)
AM_CONDITIONAL(WANT_NES_EMU, false)
AM_CONDITIONAL(WANT_NES_NEW_EMU, false)
AM_CONDITIONAL(WANT_NGP_EMU, false)
AM_CONDITIONAL(WANT_PCE_EMU, false)
AM_CONDITIONAL(WANT_PCE_FAST_EMU, false)
AM_CONDITIONAL(WANT_PCFX_EMU, false)
AM_CONDITIONAL(WANT_PSX_EMU, false)
AM_CONDITIONAL(WANT_SASPLAY_EMU, false)
AM_CONDITIONAL(WANT_SMS_EMU, false)
AM_CONDITIONAL(WANT_SNES_EMU, false)
AM_CONDITIONAL(WANT_SNES_FAUST_EMU, false)
AM_CONDITIONAL(WANT_SS_EMU, false)
AM_CONDITIONAL(WANT_SSFPLAY_EMU, false)
AM_CONDITIONAL(WANT_VB_EMU, false)
AM_CONDITIONAL(WANT_WSWAN_EMU, false)

AM_CONDITIONAL(NEED_BLIP_BUFFER, false)

AM_CONDITIONAL(NEED_GB_APU_EMU, false)
AM_CONDITIONAL(NEED_YM2413_EMU, false)
AM_CONDITIONAL(NEED_YM2612_EMU, false)
AM_CONDITIONAL(NEED_PCE_PSG_EMU, false)
AM_CONDITIONAL(NEED_SMS_APU_EMU, false)

AM_CONDITIONAL(NEED_68K_EMU, false)
AM_CONDITIONAL(NEED_Z80_EMU, false)
AM_CONDITIONAL(NEED_V810_EMU, false)

AM_CONDITIONAL(NEED_HUC6270_EMU, false)
AM_CONDITIONAL(NEED_ARCADE_CARD_EMU, false)

AC_ARG_ENABLE(apple2,
 AC_HELP_STRING([--enable-apple2], [build with Apple II+ emulation [[default=no]]]),
                  , enable_apple2=no)

if test x$enable_apple2 = xyes; then
                AC_DEFINE([WANT_APPLE2_EMU], [1], [Define if we are compiling with Apple II+ emulation.])
		AM_CONDITIONAL(WANT_APPLE2_EMU, false)
fi

AC_ARG_ENABLE(gb,
 AC_HELP_STRING([--enable-gb], [build with GameBoy emulation [[default=no]]]),
                  , enable_gb=no)

if test x$enable_gb = xyes; then
                AC_DEFINE([WANT_GB_EMU], [1], [Define if we are compiling with GB emulation.])
		AM_CONDITIONAL(WANT_GB_EMU, true)
		AM_CONDITIONAL(NEED_GB_APU_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(gba,
 AC_HELP_STRING([--enable-gba], [build with GameBoy Advance emulation [[default=no]]]),
                  , enable_gba=no)

if test x$enable_gba = xyes; then
                AC_DEFINE([WANT_GBA_EMU], [1], [Define if we are compiling with GBA emulation.])
		AM_CONDITIONAL(WANT_GBA_EMU, true)
		AM_CONDITIONAL(NEED_GB_APU_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(lynx,
 AC_HELP_STRING([--enable-lynx], [build with Atari Lynx emulation [[default=no]]]),
                  , enable_lynx=no)

if test x$enable_lynx = xyes; then
                AC_DEFINE([WANT_LYNX_EMU], [1], [Define if we are compiling with Lynx emulation.])
		AM_CONDITIONAL(WANT_LYNX_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(md,
 AC_HELP_STRING([--enable-md], [build with Sega Genesis/MegaDrive emulation [[default=no]]]),
                  , enable_md=no)

if test x$enable_md = xyes; then
                AC_DEFINE([WANT_MD_EMU], [1], [Define if we are compiling with Sega Genesis/MegaDrive emulation.])
                AM_CONDITIONAL(WANT_MD_EMU, true)
		AM_CONDITIONAL(NEED_68K_EMU, true)
		AM_CONDITIONAL(NEED_Z80_EMU, true)
		AM_CONDITIONAL(NEED_YM2612_EMU, true)
		AM_CONDITIONAL(NEED_SMS_APU_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(nes,
 AC_HELP_STRING([--enable-nes], [build with Nintendo Entertainment System emulation [[default=no]]]),
                  , enable_nes=no)

if test x$enable_nes = xyes; then
                AC_DEFINE([WANT_NES_EMU], [1], [Define if we are compiling with NES emulation.])
                AM_CONDITIONAL(WANT_NES_EMU, true)
fi

#AC_ARG_ENABLE(nes_new,
# AC_HELP_STRING([--enable-nes-new], [build with experimental, alternate new(ish) NES emulation [[default=no]]]),
#                  , enable_nes_new=no)
#
#if test x$enable_nes_new = xyes; then
#                AC_DEFINE([WANT_NES_NEW_EMU], [1], [Define if we are compiling with experimental, alternate new(ish) NES emulation.])
#                AM_CONDITIONAL(WANT_NES_NEW_EMU, true)
#fi

AC_ARG_ENABLE(ngp,
 AC_HELP_STRING([--enable-ngp], [build with Neo Geo Pocket emulation [[default=no]]]),
                  , enable_ngp=no)

if test x$enable_ngp = xyes; then
                AC_DEFINE([WANT_NGP_EMU], [1], [Define if we are compiling with NGP emulation.])
		AM_CONDITIONAL(WANT_NGP_EMU, true)
		AM_CONDITIONAL(NEED_Z80_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(pce,
 AC_HELP_STRING([--enable-pce], [build with PC Engine(TurboGrafx 16) emulation [[default=no]]]),
                  , enable_pce=no)

if test x$enable_pce = xyes; then
                AC_DEFINE([WANT_PCE_EMU], [1], [Define if we are compiling with PCE emulation.])
		AM_CONDITIONAL(WANT_PCE_EMU, true)
		AM_CONDITIONAL(NEED_PCE_PSG_EMU, true)
		AM_CONDITIONAL(NEED_HUC6270_EMU, true)
		AM_CONDITIONAL(NEED_ARCADE_CARD_EMU, true)
fi

AC_ARG_ENABLE(pce-fast,
 AC_HELP_STRING([--enable-pce-fast], [build the separate, fast PC Engine(TurboGrafx 16) emulation [[default=no]]]),
                  , enable_pce_fast=no)

if test x$enable_pce_fast = xyes; then
                AC_DEFINE([WANT_PCE_FAST_EMU], [1], [Define if we are compiling with separate fast PCE emulation.])
                AM_CONDITIONAL(WANT_PCE_FAST_EMU, true)
		AM_CONDITIONAL(NEED_ARCADE_CARD_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(pcfx,
 AC_HELP_STRING([--enable-pcfx], [build with PC-FX emulation [[default=no]]]),
                  , enable_pcfx=no)

if test x$enable_pcfx = xyes; then
                AC_DEFINE([WANT_PCFX_EMU], [1], [Define if we are compiling with PC-FX emulation.])
		AM_CONDITIONAL(WANT_PCFX_EMU, true)
		AM_CONDITIONAL(NEED_V810_EMU, true)
		AM_CONDITIONAL(NEED_PCE_PSG_EMU, true)
		AM_CONDITIONAL(NEED_HUC6270_EMU, true)
fi

AC_ARG_ENABLE(psx,
 AC_HELP_STRING([--enable-psx], [build with PlayStation emulation [[default=no]]]),
                  , enable_psx=no)

if test x$enable_psx = xyes; then
                AC_DEFINE([WANT_PSX_EMU], [1], [Define if we are compiling with PlayStation emulation.])
                AM_CONDITIONAL(WANT_PSX_EMU, true)
fi

AC_ARG_ENABLE(sasplay,
 AC_HELP_STRING([--enable-sasplay], [build with Sega Arcade SCSP Player [[default=yes]]]),
                  , enable_sasplay=yes)

if test x$enable_sasplay = xyes; then
                AC_DEFINE([WANT_SASPLAY_EMU], [1], [Define if we are compiling with Sega Arcade SCSP Player.])
                AM_CONDITIONAL(WANT_SASPLAY_EMU, true)
                AM_CONDITIONAL(NEED_68K_EMU, true)
fi

AC_ARG_ENABLE(sms,
 AC_HELP_STRING([--enable-sms], [build with SMS+GG emulation [[default=no]]]),
                  , enable_sms=no)

if test x$enable_sms = xyes; then
                AC_DEFINE([WANT_SMS_EMU], [1], [Define if we are compiling with SMS+GG emulation.])
                AM_CONDITIONAL(WANT_SMS_EMU, true)
		AM_CONDITIONAL(NEED_Z80_EMU, true)
		AM_CONDITIONAL(NEED_SMS_APU_EMU, true)
		AM_CONDITIONAL(NEED_YM2413_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

AC_ARG_ENABLE(snes,
 AC_HELP_STRING([--enable-snes], [build with SNES emulation [[default=no]]]),
                  , enable_snes=no)

if test x$enable_snes = xyes; then
                AC_DEFINE([WANT_SNES_EMU], [1], [Define if we are compiling with SNES emulation.])
                AM_CONDITIONAL(WANT_SNES_EMU, true)
fi

AC_ARG_ENABLE(snes-faust,
 AC_HELP_STRING([--enable-snes-faust], [build with experimental fast SNES emulation [[default=yenos]]]),
                  , enable_snes_faust=no)

if test x$enable_snes_faust = xyes; then
                AC_DEFINE([WANT_SNES_FAUST_EMU], [1], [Define if we are compiling with experimental fast SNES emulation.])
                AM_CONDITIONAL(WANT_SNES_FAUST_EMU, true)
fi

AC_ARG_ENABLE(ss,
 AC_HELP_STRING([--enable-ss], [build with Sega Saturn emulation [[default=x86_64 amd64 aarch64* arm64* ppc64* powerpc64*]]]),
                  , [enable_ss="x86_64 amd64 aarch64* arm64* ppc64* powerpc64*"])
for i in $enable_ss; do case $host_cpu in $i) enable_ss=yes ;; esac; done

if test "x$enable_ss" = "xyes"; then
                AC_DEFINE([WANT_SS_EMU], [1], [Define if we are compiling with Sega Saturn emulation.])
                AM_CONDITIONAL(WANT_SS_EMU, true)
		AM_CONDITIONAL(NEED_68K_EMU, true)
fi

AC_ARG_ENABLE(ssfplay,
 AC_HELP_STRING([--enable-ssfplay], [build with SSF playback support [[default=yes]]]),
                  , enable_ssfplay=yes)

if test x$enable_ssfplay = xyes; then
                AC_DEFINE([WANT_SSFPLAY_EMU], [1], [Define if we are compiling with SSF playback support.])
                AM_CONDITIONAL(WANT_SSFPLAY_EMU, true)
                AM_CONDITIONAL(NEED_68K_EMU, true)
fi

AC_ARG_ENABLE(vb,
 AC_HELP_STRING([--enable-vb], [build with Virtual Boy emulation [[default=no]]]),
                  , enable_vb=no)

if test x$enable_vb = xyes; then
                AC_DEFINE([WANT_VB_EMU], [1], [Define if we are compiling with Virtual Boy emulation.])
                AM_CONDITIONAL(WANT_VB_EMU, true)
		AM_CONDITIONAL(NEED_V810_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi


AC_ARG_ENABLE(wswan,
 AC_HELP_STRING([--enable-wswan], [build with WonderSwan emulation [[default=no]]]),
                  , enable_wswan=no)

if test x$enable_wswan = xyes; then
                AC_DEFINE([WANT_WSWAN_EMU], [1], [Define if we are compiling with WonderSwan emulation.])
                AM_CONDITIONAL(WANT_WSWAN_EMU, true)
		AM_CONDITIONAL(NEED_BLIP_BUFFER, true)
fi

##
## ALSA and JACK checking code ripped from SDL's autoconf file, with heavy modifications
##
#
# ALSA
#
AM_CONDITIONAL(HAVE_ALSA, false)
AC_ARG_ENABLE(alsa, AC_HELP_STRING([--enable-alsa], [support the ALSA audio API [[default=yes]]]),
	[],
	[enable_alsa=yes])
if test x$enable_alsa = xyes; then
	AM_PATH_ALSA(1.0.0, have_alsa=yes, have_alsa=no)

	# Restore all flags from before the ALSA detection runs
	CFLAGS="$alsa_save_CFLAGS"
	LDFLAGS="$alsa_save_LDFLAGS"
	LIBS="$alsa_save_LIBS"

	if test x$have_alsa = xyes; then
		AM_CONDITIONAL(HAVE_ALSA, true)
		AC_DEFINE([HAVE_ALSA], [1], [Define if we are compiling with ALSA support.])
	fi
fi
AC_SUBST([ALSA_LIBS])
AC_SUBST([ALSA_CFLAGS])


#
# JACK
#
AM_CONDITIONAL(HAVE_JACK, false)
AC_ARG_ENABLE(jack, AC_HELP_STRING([--enable-jack], [support the JACK audio API [[default=yes]]]),
	[],
	[enable_jack=yes])
if test x$enable_jack = xyes; then
	PKG_CHECK_MODULES(JACK, jack, [have_jack=yes], [have_jack=no])

	if test x$have_jack = xyes; then
		AM_CONDITIONAL(HAVE_JACK, true)
		AC_DEFINE([HAVE_JACK], [1], [Define if we are compiling with JACK support.])
	fi
fi
AC_SUBST([JACK_LIBS])
AC_SUBST([JACK_CFLAGS])


#
# OpenBSD Audio
#
AM_CONDITIONAL(HAVE_OPENBSD_AUDIO, false)
AC_MSG_CHECKING([for OpenBSD audio])
AC_TRY_COMPILE([
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <sys/audioio.h>
	#include <string.h>
	],
	[
	struct audio_swpar par;
	struct audio_pos pos;
	struct audio_status status;
	AUDIO_INITPAR(&par);
	],
	[AC_MSG_RESULT([yes])
	 AM_CONDITIONAL(HAVE_OPENBSD_AUDIO, true)
	 AC_DEFINE([HAVE_OPENBSD_AUDIO], [1], [Define if we are compiling with OpenBSD audio support.])],
	[AC_MSG_RESULT([no])])

#
# mpcdec
#
AM_CONDITIONAL(HAVE_EXTERNAL_MPCDEC, false)
AC_ARG_WITH([external-mpcdec], [AS_HELP_STRING([--with-external-mpcdec], [compile and link with external mpcdec @<:@default=no@:>@])],
	[],
	[with_external_mpcdec=no])
if test x$with_external_mpcdec = xyes; then
	AC_CHECK_LIB([mpcdec], [mpc_demux_init], [], AC_MSG_ERROR([*** mpcdec not found!]))
	AC_CHECK_HEADER([mpc/mpcdec.h], [], AC_MSG_ERROR([*** mpcdec not found!]))

	AC_DEFINE([HAVE_EXTERNAL_MPCDEC], [1], [Define if we are compiling and linking with external mpcdec.])
	AM_CONDITIONAL(HAVE_EXTERNAL_MPCDEC, true)
	AC_SUBST([MPCDEC_LIBS], ["-lmpcdec"])
else
	AC_DEFINE([MPC_FIXED_POINT], [1], [Define to use fixed-point MPC decoder.])
fi

#
# tremor
#
AM_CONDITIONAL(HAVE_EXTERNAL_TREMOR, false)
AC_ARG_WITH([external-tremor], [AS_HELP_STRING([--with-external-tremor], [compile and link with external tremor @<:@default=no@:>@])],
	[],
	[with_external_tremor=no])
if test x$with_external_tremor = xyes; then
	AC_CHECK_LIB([vorbisidec], [ov_read], [], AC_MSG_ERROR([*** libvorbisidec not found!]))
	AC_CHECK_HEADER([tremor/ivorbisfile.h], [], AC_MSG_ERROR([*** libvorbisidec not found!]))

	AC_DEFINE([HAVE_EXTERNAL_TREMOR], [1], [Define if we are compiling and linking with external tremor.])
	AM_CONDITIONAL(HAVE_EXTERNAL_TREMOR, true)
	AC_SUBST([TREMOR_LIBS], ["-lvorbisidec"])
fi

#
# trio
#
AM_CONDITIONAL(HAVE_EXTERNAL_TRIO, false)
AC_ARG_WITH([external-trio], [AS_HELP_STRING([--with-external-trio], [compile and link with external trio @<:@default=no@:>@])],
	[],
	[with_external_trio=no])
if test x$with_external_trio = xyes; then
	PKG_CHECK_MODULES(TRIO, libtrio, [], [
		AC_CHECK_LIB([trio], [trio_printf], [], AC_MSG_ERROR([*** libtrio not found!]))
		AC_CHECK_HEADER([trio.h], [], AC_MSG_ERROR([*** libtrio not found!]))
		TRIO_LIBS="-ltrio"
		TRIO_CFLAGS=""
	])

	AC_SUBST([TRIO_LIBS])
	AC_SUBST([TRIO_CFLAGS])

	AC_DEFINE([HAVE_EXTERNAL_TRIO], [1], [Define if we are compiling and linking with external trio.])
	AM_CONDITIONAL(HAVE_EXTERNAL_TRIO, true)
	trio_include_symsrc="include/trio_external"
else
	trio_include_symsrc="src/trio"
fi
AC_CONFIG_LINKS([include/trio/triodef.h:$trio_include_symsrc/triodef.h])
AC_CONFIG_LINKS([include/trio/trio.h:$trio_include_symsrc/trio.h])
AC_CONFIG_LINKS([include/trio/trionan.h:$trio_include_symsrc/trionan.h])
AC_CONFIG_LINKS([include/trio/triop.h:$trio_include_symsrc/triop.h])
AC_CONFIG_LINKS([include/trio/triostr.h:$trio_include_symsrc/triostr.h])

#
# minilzo/lzo
#
AM_CONDITIONAL(HAVE_EXTERNAL_LZO2, false)
AC_ARG_WITH([external-lzo], [AS_HELP_STRING([--with-external-lzo], [compile and link with external LZO @<:@default=no@:>@])],
	[],
	[with_external_lzo=no])
if test x$with_external_lzo = xyes; then
	PKG_CHECK_MODULES(LZO2, lzo2 >= 2.0.9, [], AC_MSG_ERROR([*** lzo2 >= 2.0.9 not found!]))
	AC_SUBST([LZO2_LIBS])
	AC_SUBST([LZO2_CFLAGS])
	AC_DEFINE([HAVE_EXTERNAL_LZO2], [1], [Define if we are compiling and linking with external LZO.])
	AM_CONDITIONAL(HAVE_EXTERNAL_LZO2, true)
	minilzo_include_symsrc="include/minilzo_external"
else
	minilzo_include_symsrc="include/minilzo_internal"
fi
AC_CONFIG_LINKS([include/minilzo/minilzo.h:$minilzo_include_symsrc/minilzo.h])

#
# Zstd
#
AM_CONDITIONAL(HAVE_EXTERNAL_LIBZSTD, false)
AC_ARG_WITH([external-libzstd], [AS_HELP_STRING([--with-external-libzstd], [compile and link with external libzstd @<:@default=no@:>@])],
	[],
	[with_external_libzstd=no])
if test x$with_external_libzstd = xyes; then
	PKG_CHECK_MODULES(LIBZSTD, libzstd >= 1.4.8, [], AC_MSG_ERROR([*** libzstd >= 1.4.8 not found!]))
	AC_SUBST([LIBZSTD_LIBS])
	AC_SUBST([LIBZSTD_CFLAGS])
	AC_DEFINE([HAVE_EXTERNAL_LIBZSTD], [1], [Define if we are compiling and linking with external libzstd.])
	AM_CONDITIONAL(HAVE_EXTERNAL_LIBZSTD, true)
	libzstd_include_symsrc="include/zstd_external"
else
	libzstd_include_symsrc="src/zstd"
fi
AC_CONFIG_LINKS([include/zstd/zstd.h:$libzstd_include_symsrc/zstd.h])

#
# libFLAC
#
AM_CONDITIONAL(HAVE_LIBFLAC, false)
AC_ARG_WITH([libflac],
            [AS_HELP_STRING([--with-libflac],
              [support FLAC file format in ripped CD images @<:@default=yes@:>@])],
            [],
            [with_libflac=yes])
if test x$with_libflac = xyes; then
	PKG_CHECK_MODULES(FLAC, flac >= 1.3.0, [], AC_MSG_ERROR([*** libflac >= 1.3.0 not found!]))
	AC_DEFINE([HAVE_LIBFLAC], [1], [Define if we are compiling with libFLAC support.])
	AM_CONDITIONAL(HAVE_LIBFLAC, true)
fi
AC_SUBST([FLAC_LIBS])
AC_SUBST([FLAC_CFLAGS])

AM_CONDITIONAL(HAVE_OSSDSP, false)
AM_CONDITIONAL(HAVE_DIRECTSOUND, false)
AM_CONDITIONAL(HAVE_WASAPI, false)
AM_CONDITIONAL(HAVE_SDL, false)
AM_CONDITIONAL(DOS, false)
AM_CONDITIONAL(WIN32, false)
AM_CONDITIONAL(UNIX, false)
AM_CONDITIONAL(HAVE_LINUX_JOYSTICK, false)
AM_CONDITIONAL(ENABLE_LIBXXX_MODE, false)
NEED_SDL=true

if $ENABLE_LIBXXX_MODE; then
	NEED_SDL=false
	AC_DEFINE([ENABLE_LIBXXX_MODE], [1], [Define if we are compiling in libxxx mode.])
	AM_CONDITIONAL(ENABLE_LIBXXX_MODE, true)
fi

if expr x"$host" : 'x.*-mingw*' > /dev/null; then
	AC_CHECK_TOOL([WINDRES], [windres])

	if test x$host_cpu != xx86_64; then
		# Enable larger address space support(for the 32-bit build).
		LDFLAGS="$LDFLAGS -Wl,--large-address-aware"
		AX_APPEND_COMPILE_FLAGS([-march=i686], OPTIMIZER_FLAGS)
		AX_APPEND_COMPILE_FLAGS([-mtune=pentium3], OPTIMIZER_FLAGS)
	else
		AX_APPEND_COMPILE_FLAGS([-mtune=amdfam10], OPTIMIZER_FLAGS)
	fi

	# Always build with DirectSound support.
	AM_CONDITIONAL(HAVE_DIRECTSOUND,true)
	AC_DEFINE([HAVE_DIRECTSOUND], [1], [Define if we are compiling with DirectSound support.])
	LIBS="-ldxguid -lwinmm -ldsound $LIBS"

	# Always build with WASAPI support.
	AM_CONDITIONAL(HAVE_WASAPI,true)
	AC_DEFINE([HAVE_WASAPI], [1], [Define if we are compiling with WASAPI support.])
	LIBS="-lole32 $LIBS"

        AC_DEFINE([MDFN_PSS_STYLE], [2], [Defines the filesystem path-separator type.])
	AC_DEFINE([WIN32], [1], [Define if we are compiling for Win32.])
	AM_CONDITIONAL(WIN32, true)

	LIBS="-mconsole -lws2_32 -ldxguid -lwinmm -ldinput $LIBS -lwinmm"
	CPPFLAGS="-D_LFS64_LARGEFILE=1 $CPPFLAGS"
elif expr x"$host" : 'x.*djgpp' > /dev/null; then
       	AC_DEFINE([MDFN_PSS_STYLE], [2], [Defines the filesystem path-separator type.])
       	AC_DEFINE([DOS], [1], [Define if we are compiling for DOS.])
       	AM_CONDITIONAL(DOS, true)
	NEED_SDL=false
else
	AC_DEFINE([MDFN_PSS_STYLE], [1], [Defines the filesystem path-separator type.])

	#
	# POSIX threads
	#
	AC_SEARCH_LIBS(pthread_create, [pthread])
	AC_CHECK_FUNCS(pthread_create sem_init, [], AC_MSG_ERROR([*** pthreads not found!]))
	AC_CHECK_FUNCS(sem_timedwait sem_timedwait_monotonic sem_clockwait sem_clockwait_np pthread_condattr_setclock pthread_setname_np pthread_set_name_np pthread_getaffinity_np pthread_setaffinity_np pthread_cond_timedwait_relative_np)
	AC_CHECK_HEADERS(pthread.h sched.h)
	AC_CHECK_HEADERS(pthread_np.h, [], [], [
		#include <pthread.h>
		])
	AC_MSG_CHECKING([type for pthread affinity setting])
	AC_TRY_COMPILE(
		[
			#include <pthread.h>

			#if defined HAVE_SCHED_H
 			 #include <sched.h>
			#endif

			#if defined(HAVE_PTHREAD_NP_H)
			 #include <pthread_np.h>
			#endif
		],
		[
			cpu_set_t c;
			int result;

			CPU_ZERO(&c);

			result = pthread_setaffinity_np(pthread_self(), sizeof(c), &c);
			result = pthread_getaffinity_np(pthread_self(), sizeof(c), &c);
		],
		[
			AC_MSG_RESULT([cpu_set_t])
			AC_DEFINE([PTHREAD_AFFINITY_NP], [cpu_set_t], [Define to CPU set type if we are compiling with pthreads affinity setting support.])
		],
			AC_TRY_COMPILE(
				[
					#include <pthread.h>

					#if defined HAVE_SCHED_H
		 			 #include <sched.h>
					#endif

					#if defined(HAVE_PTHREAD_NP_H)
					 #include <pthread_np.h>
					#endif
				],
				[
					cpuset_t c;
					int result;

					CPU_ZERO(&c);

					result = pthread_setaffinity_np(pthread_self(), sizeof(c), &c);
					result = pthread_getaffinity_np(pthread_self(), sizeof(c), &c);
				],
				[
					AC_MSG_RESULT([cpuset_t])
					AC_DEFINE([PTHREAD_AFFINITY_NP], [cpuset_t], [Define to CPU set type if we are compiling with pthreads affinity setting support.])
				],
				[
					AC_MSG_RESULT([unknown])
				]))


	AC_CHECK_HEADER([sys/soundcard.h],[AM_CONDITIONAL(HAVE_OSSDSP,true) AC_DEFINE([HAVE_OSSDSP], [1], [Define if we are compiling with OSS support.])])

	#
	# Linux joystick
	#
	AC_MSG_CHECKING([for Linux joystick APIs])
	AC_TRY_COMPILE([],
		[
			#ifndef __linux__
			 #error "error"
			#endif
		],
		[
			AC_TRY_COMPILE([
				#include <linux/joystick.h>
				#include <linux/input.h>
			],
			[
				struct js_event dummy0;
				struct ff_effect dummy1;
			],
			[
				AC_MSG_RESULT([yes])
				AM_CONDITIONAL(HAVE_LINUX_JOYSTICK, true)
				AC_DEFINE([HAVE_LINUX_JOYSTICK], [1], [Define if we are compiling with Linux joystick support.])
			],
			AC_MSG_ERROR([*** Linux joystick APIs not found!]))
		],
		[AC_MSG_RESULT([no])])
fi

if $NEED_SDL
then
	SDL_VERSION=2.0.5
	AM_PATH_SDL2($SDL_VERSION, HAVE_SDL=true, HAVE_SDL=false)
	if $HAVE_SDL
	then
		AM_CONDITIONAL(HAVE_SDL, true)
		AC_DEFINE([HAVE_SDL], [1], [Define if we are compiling with SDL.])
		AC_SUBST(SDL_LIBS)
		AC_SUBST(SDL_CFLAGS)
	else
		AC_MSG_ERROR([*** SDL >= 2.0.5 not found!])
	fi
fi


dnl
dnl FIXME: Make sure POSIX/BSD sockets API detection doesn't have any false positives(like when targeting Windows).
dnl
AM_CONDITIONAL(HAVE_POSIX_SOCKETS, false)
dnl
dnl
HAVE_POSIX_SOCKETS=yes

dnl
dnl Solaris may require "nsl" and "socket" libraries.
dnl
AC_SEARCH_LIBS([gethostbyname], [nsl])
AC_SEARCH_LIBS([socket], [socket network])

AC_CHECK_FUNCS(socket bind listen connect accept send recv close gethostbyname gethostbyaddr getaddrinfo freeaddrinfo gai_strerror select poll getsockopt setsockopt, [], 
	[HAVE_POSIX_SOCKETS=no])

if test x$HAVE_POSIX_SOCKETS = xyes; then
	AM_CONDITIONAL(HAVE_POSIX_SOCKETS, true)
        AC_DEFINE([HAVE_POSIX_SOCKETS], [1], [Define if we are compiling with POSIX sockets support.])
fi

AM_CONDITIONAL(ARCH_X86, false)
AM_CONDITIONAL(ARCH_X86_32, false)
AM_CONDITIONAL(ARCH_X86_64, false)
AM_CONDITIONAL(ARCH_POWERPC, false)

TRIO_BUILD_CFLAGS=""

dnl OpenBSD...
AC_TRY_LINK([], [asm volatile("vcvtss2si %xmm6,%ecx\n\tvhaddps %ymm7,%ymm7,%ymm7\n\tvzeroupper\n\t");],
	AC_DEFINE([HAVE_INLINEASM_AVX], [1], [Define if GNU-style AVX inline assembly is supported.]),
	[])

case "$host_cpu" in
	x86_64|amd64)
	        AC_DEFINE([ARCH_X86], [1], [Define if we are compiling for 32-bit or 64-bit x86 architectures.])
	        AM_CONDITIONAL(ARCH_X86, true)
                AC_DEFINE([ARCH_X86_64], [1], [Define if we are compiling for 64-bit x86 architectures.])
                AM_CONDITIONAL(ARCH_X86_64, true)
		AX_APPEND_COMPILE_FLAGS([-mcmodel=small], CODEGEN_FLAGS)
		;;

	i*86)
                AC_DEFINE([ARCH_X86], [1], [Define if we are compiling for 32-bit or 64-bit x86 architectures.])
                AM_CONDITIONAL(ARCH_X86, true)
                AC_DEFINE([ARCH_X86_32], [1], [Define if we are compiling for 32-bit x86 architectures.])
                AM_CONDITIONAL(ARCH_X86_32, true)
		;;

	powerpc*)
	        AC_DEFINE([ARCH_POWERPC], [1], [Define if we are compiling for PPC architectures.])
	        AM_CONDITIONAL(ARCH_POWERPC, true)

	        if test x$enable_altivec = xyes; then
        	        ALTIVEC_FLAGS=""
                	AX_APPEND_COMPILE_FLAGS([-faltivec], ALTIVEC_FLAGS)
	                AX_APPEND_COMPILE_FLAGS([-maltivec], ALTIVEC_FLAGS)
	                if test "x$ALTIVEC_FLAGS" != "x"; then
				altivec_save_CPPFLAGS="$CPPFLAGS"
				CPPFLAGS="$CPPFLAGS $ALTIVEC_FLAGS"
				AC_CHECK_HEADER([altivec.h],
					AC_DEFINE([HAVE_ALTIVEC_H], [1], [Define if altivec.h is present and usable.]),
					[] )
				CPPFLAGS="$altivec_save_CPPFLAGS"
	                fi
	        fi
		;;

	alpha*)
		AX_APPEND_COMPILE_FLAGS([-mieee], TRIO_BUILD_CFLAGS)
		;;
esac
AC_SUBST(TRIO_BUILD_CFLAGS)

AC_C_BIGENDIAN([AC_DEFINE([MSB_FIRST],[1],[Define on big-endian platforms.])], 
[
 AC_DEFINE([LSB_FIRST],[1],[Define on little-endian platforms.])
])
AC_DEFINE_UNQUOTED([MEDNAFEN_VERSION],"$MEDNAFEN_VERSION", [Mednafen version definition.])
AC_DEFINE_UNQUOTED([MEDNAFEN_VERSION_NUMERIC],$MEDNAFEN_VERSION_NUMERIC, [Mednafen version numeric.])

AC_SUBST([AM_CFLAGS], "$ALTIVEC_FLAGS $OPTIMIZER_FLAGS $WARNING_FLAGS $CODEGEN_FLAGS $NOPICPIE_FLAGS")
AC_SUBST([AM_CXXFLAGS], "$ALTIVEC_FLAGS $OPTIMIZER_FLAGS $WARNING_FLAGS $CODEGEN_FLAGS $NOPICPIE_FLAGS")
AC_SUBST([AM_LDFLAGS], "$NOPICPIE_LDFLAGS")

dnl Output Makefiles
AC_OUTPUT([Makefile po/Makefile.in intl/Makefile src/Makefile src/drivers/Makefile src/drivers_libxxx/Makefile src/drivers_dos/Makefile src/sexyal/Makefile src/ss/Makefile])
