check_PROGRAMS = \
	test-str \
	test-settings

TESTS = $(check_PROGRAMS)

test_str_SOURCES = test-str.cc
test_str_LDADD = ../src/util/str.o

test_settings_SOURCES = test-settings.cc
test_settings_LDADD = ../src/settings.o
test_settings_LDFLAGS = $(config_LIBS)
test_settings_CFLAGS = $(config_CFLAGS)
