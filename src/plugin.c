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

#include <gtk/gtk.h>
#include <glib.h>
#include <sharing-plugin-interface.h>
#include <sharing-transfer.h>
#include <conicconnection.h>
#include <osso-log.h>
#include <hildon/hildon.h>

#include "send.h"
#include "validate.h"


static SharingPluginInterfaceEditAccountResult
edit_account(SharingAccount* account, 
			 GtkWindow* parent,
			 gboolean setup);

/**
 * sharing_plugin_interface_init:
 * @dead_mans_switch: What?
 *
 * Initialize interface
 *
 * Returns: 0
 */
guint sharing_plugin_interface_init (gboolean* dead_mans_switch)
{
    ULOG_DEBUG_L("sharing_manager_plugin_interface_init");
    return 0;
}

/**
 * sharing_plugin_interface_uninit:
 * @dead_mans_switch: What?
 *
 * Uninitialize interface
 *
 * Returns: 0
 */
guint sharing_plugin_interface_uninit (gboolean* dead_mans_switch)
{
    ULOG_DEBUG_L("sharing_manager_plugin_interface_uninit");
    return 0;
}

/**
 * sharing_plugin_interface_send:
 * @transfer: Transfer to be send
 * @con: Connection used
 * @dead_mans_switch: 
 *
 * Send interface.
 *
 * Returns: Result of send
 */
SharingPluginInterfaceSendResult sharing_plugin_interface_send
    (SharingTransfer* transfer, ConIcConnection* con,
    gboolean* dead_mans_switch)
{
    ULOG_DEBUG_L ("sharing_plugin_interface_send");
    SharingPluginInterfaceSendResult ret_val = SHARING_SEND_ERROR_UNKNOWN;
    ret_val = send (transfer, con, dead_mans_switch);
    return ret_val;
}

/**
 * sharing_plugin_interface_account_setup:
 * @transfer: #SharingTransfer send
 * @service: #SharingService
 * @worked_on: Connection used
 * @osso_context_t: Osso context
 *
 * Send interface
 *
 * Returns: Result of account setup
 */
SharingPluginInterfaceAccountSetupResult sharing_plugin_interface_account_setup
    (GtkWindow* parent, SharingService* service, SharingAccount** worked_on,
    osso_context_t* osso)
{
    ULOG_DEBUG_L ("sharing_plugin_interface_account_setup");
    return edit_account (*worked_on, parent, TRUE);
}

/**
 * sharing_manager_plugin_interface_account_validate:
 * @account: Account tested
 * @con: Connection used to test account
 * @dead_mans_switch: 
 *
 * Validates account information.
 *
 * Returns: Result of account validation
 */
SharingPluginInterfaceAccountValidateResult
sharing_plugin_interface_account_validate (SharingAccount* account, 
    ConIcConnection* con, gboolean *cont, gboolean* dead_mans_switch)
{
    ULOG_DEBUG_L ("sharing_plugin_interface_account_validate");
    SharingPluginInterfaceAccountValidateResult ret_val = 0;
    ret_val = validate (account, con, cont, dead_mans_switch);
    return ret_val;
}

/**
 * sharing_plugin_interface_edit_account:
 * @account: Account tested
 * @con: Connection used to test account
 * @dead_mans_switch: 
 *
 * Edit account plugin implementation.
 *
 * Returns: Result of account edit
 */
SharingPluginInterfaceEditAccountResult
    sharing_plugin_interface_edit_account (GtkWindow* parent,
    SharingAccount* account, ConIcConnection* con, gboolean* dead_mans_switch)
{
    ULOG_DEBUG_L ("sharing_plugin_interface_edit_account");
    return edit_account (account, parent, FALSE);
}


static void
gui_add_item(GtkWidget* table,
			 guint row,
			 gchar* id,
			 const gchar* label,
			 const gchar* placeholder,
			 gboolean invis,
			 SharingAccount* a,
			 GHashTable* h)
{
  GtkWidget* wlabel = gtk_label_new (label);

  gtk_table_attach (GTK_TABLE (table), wlabel, 0, 1, row, row+1,
					GTK_FILL, GTK_FILL|GTK_EXPAND, HILDON_MARGIN_DOUBLE, 0);
  
  GtkWidget* wentry = hildon_entry_new (HILDON_SIZE_AUTO);
  hildon_entry_set_placeholder (HILDON_ENTRY (wentry), placeholder);

  if (invis) {
	
	hildon_gtk_entry_set_input_mode(GTK_ENTRY (wentry),
									HILDON_GTK_INPUT_MODE_FULL |
									HILDON_GTK_INPUT_MODE_INVISIBLE);
  }

  gtk_table_attach_defaults (GTK_TABLE (table), wentry, 1, 2, row, row+1);
  
  g_hash_table_insert (h, id, wentry);
  
  gchar* old = sharing_account_get_param (a, id);

  if (old) {
	gtk_entry_set_text (GTK_ENTRY (wentry), old);
	g_free (old);
  }

}



static gboolean
gui_read_item (GHashTable* h, const gchar* id, SharingAccount* a)
{
  GtkWidget* wentry = g_hash_table_lookup (h, id);
  if (!wentry) return FALSE;
  
  gchar* old = sharing_account_get_param (a, id);
  const gchar* new = gtk_entry_get_text (GTK_ENTRY (wentry));
  
  gboolean changed = FALSE;
  
  if (!old || g_strcmp0 (old, new) != 0)
	{
	  sharing_account_set_param (a, id, new);
	  changed = TRUE;
	}
  /* Make the account name shown under service name */
  if (changed && g_strcmp0(id, "name") == 0) {
	  sharing_account_set_username(a, new);
  }
  g_free (old);
  return changed;
}

static gboolean
gui_read(GHashTable* h, SharingAccount* a)
{
  gboolean command_updated = FALSE;
  gboolean name_updated = FALSE;
  command_updated = gui_read_item (h, "command_line", a);
  name_updated = gui_read_item (h, "name", a);
  /* Must separate the evaluation of two gui_read_item, or 1 of 2 params
   * won't be saved */
  return command_updated || name_updated;
}


static SharingPluginInterfaceEditAccountResult
edit_account(SharingAccount* account, GtkWindow* parent, gboolean setup)
{
  GHashTable* h = g_hash_table_new (g_str_hash, g_str_equal);  
  GtkWidget* dlg = 0;
  GtkWidget* dlg_content;
  GtkWidget* table;
  GtkWidget* vbox;
  GtkWidget* label;

  ULOG_DEBUG_L ("edit_account");


  if (setup) {
	dlg = gtk_dialog_new_with_buttons ("Account setup - CLI", parent,
									   GTK_DIALOG_MODAL |
									   GTK_DIALOG_DESTROY_WITH_PARENT,
									   GTK_STOCK_OK, GTK_RESPONSE_YES,
									   NULL);
  } else {
	dlg = gtk_dialog_new_with_buttons ("Edit account - CLI", parent,
									   GTK_DIALOG_MODAL |
									   GTK_DIALOG_DESTROY_WITH_PARENT,
									   GTK_STOCK_SAVE, GTK_RESPONSE_YES,
									   GTK_STOCK_DELETE, GTK_RESPONSE_NO,
									   NULL);
  }

  dlg_content = gtk_dialog_get_content_area (GTK_DIALOG (dlg));
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add (GTK_CONTAINER (dlg_content), vbox);


  /* Add info label */
  
  label = gtk_label_new("You must use 2x \"%s\", one for src filename, one for dst filename.");
  gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_single_line_mode(GTK_LABEL(label), FALSE);

  gtk_box_pack_end(GTK_BOX(vbox), label, FALSE, FALSE, 0);


  /* Add account info */
  table = gtk_table_new (4, 2, FALSE);

  gtk_box_pack_end(GTK_BOX(vbox), table, TRUE, TRUE, 0);
  
  gui_add_item(table, 1,
			   "command_line", "CLI command",
			   "Command to execute",
			   FALSE, account, h);
  gui_add_item(table, 0,
			   "name", "Name",
			   "Account name",
			   FALSE, account, h);
  
  gtk_widget_show_all (GTK_WIDGET (dlg));
  gint result = gtk_dialog_run (GTK_DIALOG (dlg));
  
  gboolean changed = FALSE;
  if (result == GTK_RESPONSE_YES)
	changed = gui_read (h, account);
  
  gtk_widget_destroy (dlg);
  g_hash_table_unref (h);
  
  if (result == GTK_RESPONSE_YES && (changed || setup))
	return SHARING_EDIT_ACCOUNT_SUCCESS;
  else if (result == GTK_RESPONSE_YES) /* !changed in edit */
	return SHARING_EDIT_ACCOUNT_NOT_STARTED;
  else if (result == GTK_RESPONSE_NO)
	return SHARING_EDIT_ACCOUNT_DELETE;
  else
	return SHARING_EDIT_ACCOUNT_CANCELLED;
}
