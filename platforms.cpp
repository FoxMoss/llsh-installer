#include <filesystem>
#include <string>

// https://stackoverflow.com/a/66249936
const std::string get_build() {  // Get current architecture, detectx nearly
                                 // every architecture. Coded by Freak
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
  return "x86_32";
#elif defined(__ARM_ARCH_2__)
  return "arm2";
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
  return "arm3";
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
  return "arm4t";
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
  return "arm5"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
  return "arm6t2";
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) ||  \
    defined(__ARM_ARCH_6ZK__)
  return "arm6";
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || \
    defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) ||  \
    defined(__ARM_ARCH_7S__)
  return "arm7";
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || \
    defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "arm7a";
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || \
    defined(__ARM_ARCH_7S__)
  return "arm7r";
#elif defined(__ARM_ARCH_7M__)
  return "arm7m";
#elif defined(__ARM_ARCH_7S__)
  return "arm7s";
#elif defined(__aarch64__) || defined(_M_ARM64)
  return "arm64";
#elif defined(mips) || defined(__mips__) || defined(__mips)
  return "mips";
#elif defined(__sh__)
  return "superh";
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || \
    defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) ||           \
    defined(_ARCH_PPC)
  return "powerpc";
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
  return "powerpc64";
#elif defined(__sparc__) || defined(__sparc)
  return "sparc";
#elif defined(__m68k__)
  return "m68k";
#else
  return "unknown";
#endif
}

const std::string get_os() {
#ifdef _WIN32
  return "win32";
#elif _WIN64
  return "win64";
#elif __APPLE__ || __MACH__
  return "macos";
#elif __linux__
  return "linux";
#elif __FreeBSD__
  return "freebsd";
#else
  return "unknown";
#endif
}

const std::string get_abi() {
#if __linux__
  // modern libc exists we can link with!
  if (std::filesystem::exists("/lib64/libc.so.6") ||
      std::filesystem::exists("/lib/libc.so.6") ||
      std::filesystem::exists("/lib/x86_64-linux-gnu/libc.so.6")) {
    return "gnu";
  }

  return "musl";
#else
  return "unknown";
#endif
}
