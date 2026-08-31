
#define WINDOWS

//#define STEAMWORKS

#define USE_FMOD

#define EDITOR_EXE_NAME "editor"

#define BASE_DATA_DIR "./"

#ifndef NDEBUG
#define DEBUG_ACHIEVEMENTS
#define DEBUG_EVENT_TIMERS
#endif

#ifndef EDITOR
#ifdef WINDOWS
#define USE_NVAPI // Video settings option for NVIDIA OpenGL compatibility stuttering fix
#if defined(STEAMWORKS) || defined(USE_EOS)
#define USE_DISCORD
#endif
#endif
#endif

#ifdef BARONY_DRM_FREE
#ifdef USE_DISCORD
#undef USE_DISCORD
#endif
#ifdef STEAMWORKS
#undef STEAMWORKS
#endif // STEAMWORKS

#endif
