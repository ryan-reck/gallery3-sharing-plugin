/*
 * This file is part of sharing-plugin-template
 *
 * Copyright (C) 2015-2016 Ryan Reck
 * Copyright (C) 2008-2009 Nokia Corporation. All rights reserved.
 *
 * This maemo code example is licensed under a MIT-style license,
 * that can be found in the file called "COPYING" in the root
 * directory.
 *
 */

#ifndef _UPDATE_H_
#define _UPDATE_H_

#include <glib.h>
#include <sharing-transfer.h>
#include <sharing-entry.h>
#include <conicconnection.h>
#include <sharing-plugin-interface.h>

G_BEGIN_DECLS

gboolean
sharing_plugin_interface_update_options (SharingAccount *account,
                                         ConIcConnection *con,
                                         gboolean *cont,
                                         gboolean *dead_mans_switch,
                                         UpdateOptionsCallback cb_func,
                                         gpointer cb_data);


G_END_DECLS

#endif // _UPDATE_H_

