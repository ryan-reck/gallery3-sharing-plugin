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

#include <stdio.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <osso-log.h>
#include <sharing-http.h>
#include "send.h"
#include "common.h"

//*/
#ifdef ULOG_DEBUG
#undef ULOG_DEBUG
#endif
#ifdef ULOG_DEBUG_L
#undef ULOG_DEBUG_L
#endif


#define ULOG_DEBUG(FRMT) {FILE *f = fopen("/tmp/gallery-sharing.send.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__); fclose(f);}
#define ULOG_DEBUG_L(FRMT, ...) {FILE *f = fopen("/tmp/gallery-sharing.send.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__, __VA_ARGS__); fclose(f);}
//*/

typedef struct {
  guint64 total_size;
  gboolean* dead_mans_switch;
  SharingTransfer* transfer;
} callback_data;

gboolean callback_function(SharingHTTP* http, guint64 bytes_sent, gpointer user_data) {
  ULOG_DEBUG_L("in callback, bytes_sent: %llu", bytes_sent);
  callback_data* data = (callback_data*)user_data;
  *(data->dead_mans_switch)=FALSE;
  float percent = bytes_sent / (float) data->total_size;
  sharing_transfer_set_progress(data->transfer, percent);
  return TRUE; //true to continue
}


/**
 * send:
 * @account: #SharingTransfer to be send
 * @con: Connection used
 * @dead_mans_switch: Turn to %FALSE at least every 30 seconds.
 *
 * Sends #SharingTransfer to service.
 *
 * Returns: #SharingPluginInterfaceSendResult
 */
SharingPluginInterfaceSendResult sharing_plugin_interface_send (SharingTransfer* transfer,
    ConIcConnection* con, gboolean* dead_mans_switch)
{
  ULOG_DEBUG("send called");
  *dead_mans_switch=FALSE;

  callback_data data;
  data.dead_mans_switch=dead_mans_switch;
  data.transfer=transfer;
  
  SharingPluginInterfaceSendResult ret = SHARING_SEND_SUCCESS;

    SharingEntry *entry = sharing_transfer_get_entry( transfer );
    data.total_size = sharing_entry_get_size(entry);
    
    SharingAccount* account = sharing_entry_get_account(entry);
    
    SharingHTTP * http = sharing_http_new ();
    //sharing_http_set_connection(http, con);
    //sharing_http_set_timeouts(http, 60 /*connecting*/, 60 /*connection*/);
    
    //get api_key and other parameters
    gchar* api_key = sharing_account_get_param(account,"api_key");
    sharing_http_add_req_header(http, "X-Gallery-Request-Key", api_key);
    ULOG_DEBUG_L("api_key: %s", api_key);
    g_free(api_key);
    sharing_http_add_req_header(http, "X-Gallery-Request-Method", "POST");
    
    gchar* host = sharing_account_get_param(account,"host");
    gchar* port = sharing_account_get_param(account,"port");
    const gchar* album = sharing_entry_get_option(entry,"album");
    gboolean free_port = TRUE;
    // editting might not set the port to 80 if it's not editted
    /*if (port == NULL || g_strcmp0(port, "") == 0) {
      free_port=FALSE;
      port="80";
    }*/

    gchar* url = g_strconcat("http://",host,":",port,"/index.php/rest/item/",album,NULL);
    ULOG_DEBUG_L("url: %s", url);
    
    g_free(host);
    if (free_port)
      g_free(port);

    //    sharing_http_set_progress_callback(http, callback_function, &data); //??
    
    JsonGenerator *generator = json_generator_new();
    SharingHTTPRunResponse res;

    for (GSList* p = sharing_entry_get_media (entry); p != NULL; p = g_slist_next(p)) {
      *dead_mans_switch=FALSE;
      SharingEntryMedia* media = p->data;
      /* Process media */

      if (sharing_entry_media_get_sent (media)) {
        //already sent? weird...
        ULOG_DEBUG("not sending already sent file.");
        continue;
      }
 
      gchar* name=sharing_entry_media_get_title(media);
      gchar* filename=sharing_entry_media_get_filename(media);
      const gchar* description=sharing_entry_media_get_desc(media);
      gchar* mime=sharing_entry_media_get_mime(media);

      // determine actual type
      const gchar* item_type;
      if (g_str_has_prefix(mime, "image")) {
        item_type="photo";
      }
      else if (g_str_has_prefix(mime, "video")) {
        item_type="movie";
      }
      else {
        //error out unknown type
        ULOG_DEBUG_L("unknown mimetype: %s",mime);
        res = SHARING_HTTP_RUNRES_INVALID_PARAMETERS;
        ret = SHARING_SEND_ERROR_FILE_FORMAT;
        goto outro;
      }

      JsonObject *jobj = json_object_new();
      json_object_set_string_member(jobj, "type", item_type);
      if (name)
        json_object_set_string_member(jobj, "name", name);
      if (description)
        json_object_set_string_member(jobj, "description", description);
      JsonNode *node = json_node_new(JSON_NODE_OBJECT);
      json_node_set_object(node, jobj);
      json_generator_set_root(generator, node);
      gchar* json_entity = json_generator_to_data(generator, NULL);
      /*/
      gchar* json_entity= g_strconcat("{\"name\":\"",name,"\",\"type\":\"",item_type,"\",\"description\":\"",description,"\"}",NULL);
      //*/
      ULOG_DEBUG_L("entity json: %s",json_entity);
      sharing_http_add_req_multipart_data(http,
                                          "entity",
                                          json_entity,
                                          -1,
                                          "application/json");
      sharing_http_add_req_multipart_file_with_filename(http,
                                                        "file", //part_name
                                                        filename,
                                                        mime,
                                                        name);

      /* Post media */
      res = sharing_http_run (http, url);

      ULOG_DEBUG_L("response code: %d", sharing_http_get_res_code(http));
      json_object_unref(jobj);
      g_free(json_entity);
      json_node_free(node);

    outro:
      g_free(name);
      g_free(filename);
      //g_free(description);
      g_free(mime);
      
      /* Process post result */
      if (res == SHARING_HTTP_RUNRES_SUCCESS) {
        /* If success mark media as sent */
        sharing_entry_media_set_sent (media, TRUE);
        /* And mark process to your internal data structure */
        //my_send_task->upload_done += sharing_entry_media_get_size (media);
      } else {
        /* We have sent the file in last sharing-manager call */
        //my_send_task->upload_done += sharing_entry_media_get_size (media);
        ULOG_DEBUG_L("send failed: %d", res);
        if (ret == SHARING_SEND_SUCCESS) { //don't overwrite other errors...
          switch(res) {
            //case SHARING_HTTP_RUNRES_SUCCESS:
          case SHARING_HTTP_RUNRES_CONNECTION_PROBLEM:
            ret = SHARING_SEND_ERROR_CONNECTION;
            break;
          case SHARING_HTTP_RUNRES_CANCELLED:
            ret = SHARING_SEND_CANCELLED;
            break;
          case SHARING_HTTP_RUNRES_INVALID_PARAMETERS:
          case SHARING_HTTP_RUNRES_ALREADY_RUNNING:
          case SHARING_HTTP_RUNRES_UNKNOWN_FAILURE:
            ret = SHARING_SEND_ERROR_UNKNOWN;
            break;
          }
        }
      }
      if(!sharing_transfer_continue(transfer)) {
        ret= SHARING_SEND_CANCELLED;
        break;
      }
      sharing_http_clear_multiparts(http);
    }
    g_object_unref(generator);
    g_free(url);
    //g_free(album);//??
    sharing_http_unref (http); 
    ULOG_DEBUG("bye");
    return ret;
}

