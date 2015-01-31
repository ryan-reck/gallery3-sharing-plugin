#
# This file is part of sharing-plugin-template
#
# Copyright (C) 2008-2009 Nokia Corporation. All rights reserved.
#
# This maemo code example is licensed under a MIT-style license,
# that can be found in the file called "COPYING" in the root
# directory.
#

plugins_LTLIBRARIES = gallery3sharing.la

gallery3sharing_la_SOURCES = plugin.c common.c send.c validate.c

gallery3sharing_la_CFLAGS = $(PACKAGE_CFLAGS) $(DEBUG_CFLAGS)
gallery3sharing_la_LDFLAGS = -module -avoid-version -export-dynamic
gallery3sharing_la_LIBADD = $(PACKAGE_LIBS)

AM_CPPFLAGS=-I/usr/source/hildon-1

MAINTAINERCLEANFILES = Makefile.in


