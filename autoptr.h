#ifndef VDR_LIVE_AUTOPTR_H
#define VDR_LIVE_AUTOPTR_H

#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION > 50000
# define AUTO_PTR  std::unique_ptr
#else
# define AUTO_PTR  std::auto_ptr
#endif

#endif // VDR_LIVE_AUTOPTR_H
