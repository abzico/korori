AC_PREREQ([2.69])
AC_INIT([krr], [0.1], [wasin@abzi.co])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

dnl Variables we will share their result to Makefile(s)
ISUNIX="false"
ISWINDOWS="false"
ISMACOSX="false"

AM_EXTRA_RECURSIVE_TARGETS([samples \
                            clean-samples \
                            sample-doublemulticolorshader \
                            sample-rotatingcube \
                            sample-rotatingplane \
                            sample-readobjfile_manual \
                            sample-readobjfile \
                            sample-advanced_model \
                            sample-headless \
                            sample-terrain \
                            sample-dds])

AM_PROG_AR

AC_ENABLE_SHARED
AC_ENABLE_STATIC

dnl Determine the OS
case "$host" in
  *-*-cygwin* | *-*-mingw32*)
    ISWINDOWS="true"
    ;;
  *-*-darwin*)
    ISMACOSX="true"
    ;;
  *)
    dnl otherwise, just treat it as unix
    ISUNIX="true"
    ;;
esac

AC_SUBST(ISUNIX)
AC_SUBST(ISWINDOWS)
AC_SUBST(ISMACOSX)

dnl Conditionals used in samples/Makefile.am
dnl Note: Conditional must be defined after result is asigned
dnl back to the variable, it will capture such value at that time
AM_CONDITIONAL([CD_IS_MACOSX], [test x$ISMACOSX = xtrue])
AM_CONDITIONAL([CD_IS_WINDOWS], [test x$ISWINDOWS = xtrue])
AM_CONDITIONAL([CD_IS_UNIX], [test x$ISUNIX = xtrue])

LT_INIT

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lGLEW':
AC_CHECK_LIB([GLEW], [glewInit],,
             [AC_MSG_ERROR(Not found GLEW. See http://glew.sourceforge.net/install.html and install it on your system.")])
# FIXME: Replace `main' with a function in `-lSDL2':
AC_CHECK_LIB([SDL2], [SDL_Init],,
             [AC_MSG_ERROR("Not found SDL2. See https://www.libsdl.org/download-2.0.php and install it on your system.")])
# FIXME: Replace `main' with a function in `-lSDL2_image':
AC_CHECK_LIB([SDL2_image], [IMG_Init],,
             [AC_MSG_ERROR("Not found SDL2_image. See https://www.libsdl.org/projects/SDL_image/ and install it on your system.")])
# FIXME: Replace `main' with a function in `-lSDL2_mixer':
AC_CHECK_LIB([SDL2_mixer], [Mix_Init],,
             [AC_MSG_ERROR("Not found SDL2_mixer. See https://www.libsdl.org/projects/SDL_mixer/ and install it on your system.")])
# FIXME: Replace `main' with a function in `-lcglm':
AC_CHECK_LIB([cglm], [glmc_vec4],,
             [AC_MSG_ERROR("Not found cglm. See https://github.com/recp/cglm and install it on your system.")])
# FIXME: Replace `main' with a function in `-lfreetype':
AC_CHECK_LIB([freetype], [FT_Init_FreeType],,
             [AC_MSG_ERROR("Not found freetype. See https://www.freetype.org/download.html and install on your system.")])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h stdbool.h assert.h time.h math.h stdio.h stdarg.h])

# opengl
AX_CHECK_GL(,[AC_MSG_ERROR("Not found any OpenGL on system")])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memset sqrt])

AC_CONFIG_FILES([Makefile
                 samples/Makefile])
AC_OUTPUT
