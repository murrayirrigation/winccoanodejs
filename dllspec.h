#ifndef DLLSPEC_H_
#define DLLSPEC_H_

#ifndef IL_DEPRECATED
  #define IL_DEPRECATED(MSG) __declspec(deprecated(MSG))
#endif

#ifdef MAKEDLL_BCM
  #define DLLEXP_BCM  __declspec(dllexport)
#else
  #define DLLEXP_BCM  __declspec(dllimport)
#endif

#ifdef MAKE_DLL_BASICS
  #define DLLEXP_BASICS  __declspec(dllexport)
  #define EXTERN_BASICS
#else
  #define DLLEXP_BASICS  __declspec(dllimport)
  #define EXTERN_BASICS  extern
#endif

#ifdef MAKE_DLL_BASICS
  #define DLLEXP_OABASICS  __declspec(dllexport)
  #define EXTERN_OABASICS
#else
  #define DLLEXP_OABASICS  __declspec(dllimport)
  #define EXTERN_OABASICS  extern
#endif

#ifdef MAKE_DLL_CONFIGS
  #define DLLEXP_CONFIGS  __declspec(dllexport)
  #define EXTERN_CONFIGS
#else
  #define DLLEXP_CONFIGS  __declspec(dllimport)
  #define EXTERN_CONFIGS  extern
#endif

#ifdef MAKE_DLL_DATAPOINT
  #define DLLEXP_DATAPOINT  __declspec(dllexport)
  #define EXTERN_DATAPOINT
#else
  #define DLLEXP_DATAPOINT  __declspec(dllimport)
  #define EXTERN_DATAPOINT  extern
#endif

#ifdef MAKE_DLL_MESSAGES
  #define DLLEXP_MESSAGES  __declspec(dllexport)
  #define EXTERN_MESSAGES
#else
  #define DLLEXP_MESSAGES  __declspec(dllimport)
  #define EXTERN_MESSAGES  extern
#endif

#ifdef MAKE_DLL_MANAGER
  #define DLLEXP_MANAGER  __declspec(dllexport)
  #define EXTERN_MANAGER
#else
  #define DLLEXP_MANAGER  __declspec(dllimport)
  #define EXTERN_MANAGER  extern
#endif

#ifdef MAKE_DLL_CTRL
  #define DLLEXP_CTRL  __declspec(dllexport)
  #define EXTERN_CTRL
#else
  #define DLLEXP_CTRL  __declspec(dllimport)
  #define EXTERN_CTRL  extern
#endif

#endif
