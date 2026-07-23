---
applyTo: "**/Makefile.am,**/configure.ac,**/*.ac,**/*.mk"
---

# Build System Standards (Autotools) for Cellular Manager

## Autotools Best Practices

### configure.ac
- Check for required headers and functions
- Provide clear error messages for missing modem SDK dependencies
- Support cross-compilation for ARM/MIPS targets
- Allow feature toggles for optional modem backends

```autoconf
# GOOD: Check for required features
AC_CHECK_HEADERS([pthread.h], [],
    [AC_MSG_ERROR([pthread.h is required])])

AC_CHECK_LIB([pthread], [pthread_create], [],
    [AC_MSG_ERROR([pthread library is required])])

# GOOD: Optional modem SDK with clear naming
AC_ARG_ENABLE([qmi],
    AS_HELP_STRING([--enable-qmi], [Enable QMI modem support via libqmi]),
    [enable_qmi=$enableval],
    [enable_qmi=no])

AM_CONDITIONAL([WITH_QMI_SUPPORT], [test "x$enable_qmi" = "xyes"])

AC_ARG_ENABLE([gtest],
    AS_HELP_STRING([--enable-gtest], [Enable Google Test support]),
    [enable_gtest=$enableval],
    [enable_gtest=no])

AM_CONDITIONAL([WITH_GTEST_SUPPORT], [test "x$enable_gtest" = "xyes"])
```

### Makefile.am
- Use non-recursive makefiles when possible
- Minimize intermediate libraries
- Support parallel builds
- Link only what's needed

```makefile
# GOOD: Minimal linking
bin_PROGRAMS = CellularManager

CellularManager_SOURCES = \
    cellularmgr_main.c \
    cellularmgr_sm.c \
    cellularmgr_cellular_apis.c

CellularManager_CFLAGS = -DFEATURE_SUPPORT_RDKLOG

CellularManager_LDADD = \
    -lpthread

# GOOD: Conditional compilation
if WITH_QMI_SUPPORT
CellularManager_SOURCES += cellular_hal_qmi_apis.c
CellularManager_LDADD += -lqmi-glib
endif

if WITH_GTEST_SUPPORT
SUBDIRS += test
endif
```

## Cross-Compilation Support

### Platform Detection
```autoconf
# Support different target platforms
case "$host" in
    *-linux*)
        AC_DEFINE([PLATFORM_LINUX], [1], [Linux platform])
        ;;
    arm*|*-arm*)
        AC_DEFINE([PLATFORM_ARM], [1], [ARM platform])
        ;;
    mips*|*-mips*)
        AC_DEFINE([PLATFORM_MIPS], [1], [MIPS platform])
        ;;
esac
```

### Compiler Flags
```makefile
# Platform-specific optimizations
if TARGET_ARM
AM_CFLAGS += -march=armv7-a
endif

# Debug vs Release
if DEBUG_BUILD
AM_CFLAGS += -g -O0 -DDEBUG
else
AM_CFLAGS += -O2 -DNDEBUG
endif
```

## Dependency Management

### Package Config
```autoconf
# Use pkg-config for external dependencies
PKG_CHECK_MODULES([DBUS], [dbus-1 >= 1.6], [], [AC_MSG_WARN([dbus-1 not found])])
AC_SUBST([DBUS_CFLAGS])
AC_SUBST([DBUS_LIBS])
```

### Header Organization
```makefile
# Include paths
AM_CPPFLAGS = -I$(top_srcdir)/source/CellularManager \
              -I$(top_srcdir)/source/TR-181 \
              $(DBUS_CFLAGS)
```

## Testing Integration

```makefile
# Test targets
check-local:
	@echo "Running memory leak tests..."
	@for test in $(TESTS); do \
		valgrind --leak-check=full \
		         --error-exitcode=1 \
		         ./$$test || exit 1; \
	done

# Code coverage
if ENABLE_COVERAGE
AM_CFLAGS += --coverage
AM_LDFLAGS += --coverage
endif

coverage: check
	$(LCOV) --capture --directory . --output-file coverage.info
	$(GENHTML) coverage.info --output-directory coverage
```

## Build Validation

- Verify `autoreconf -i`, `./configure`, and `make` on a clean workspace
- Confirm test targets remain callable from CI workflows
- Verify cross-compilation with `--host=arm-linux-gnueabihf`
