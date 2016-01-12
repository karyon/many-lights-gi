
#ifndef MFS_PAINTERS_API_H
#define MFS_PAINTERS_API_H

#ifdef MFS_PAINTERS_STATIC_DEFINE
#  define MFS_PAINTERS_API
#  define MFS_PAINTERS_NO_EXPORT
#else
#  ifndef MFS_PAINTERS_API
#    ifdef mfs_painters_EXPORTS
/* We are building this library */
#      ifdef _WIN32
#        define MFS_PAINTERS_API __declspec(dllexport)
#      else
#        define MFS_PAINTERS_API __attribute__ ((visibility ("default")))
#      endif
#    else
/* We are using this library */
#      ifdef _WIN32
#        define MFS_PAINTERS_API __declspec(dllimport)
#      else
#        define MFS_PAINTERS_API
#      endif
#    endif
#  endif

#  ifndef MFS_PAINTERS_NO_EXPORT
#    define MFS_PAINTERS_NO_EXPORT
#  endif
#endif

#ifndef MFS_PAINTERS_DEPRECATED
#  define MFS_PAINTERS_DEPRECATED __declspec(deprecated)
#endif

#ifndef MFS_PAINTERS_DEPRECATED_EXPORT
#  define MFS_PAINTERS_DEPRECATED_EXPORT MFS_PAINTERS_API MFS_PAINTERS_DEPRECATED
#endif

#ifndef MFS_PAINTERS_DEPRECATED_NO_EXPORT
#  define MFS_PAINTERS_DEPRECATED_NO_EXPORT MFS_PAINTERS_NO_EXPORT MFS_PAINTERS_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define MFS_PAINTERS_NO_DEPRECATED
#endif

#endif
