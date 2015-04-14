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
#include <sharing-service-option.h>
#include "update.h"

/*/
#ifdef ULOG_DEBUG
#undef ULOG_DEBUG
#endif
#ifdef ULOG_DEBUG_L
#undef ULOG_DEBUG_L
#endif


#define ULOG_DEBUG(FRMT) {FILE *f = fopen("/tmp/gallery-sharing.send.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__); fclose(f);}
#define ULOG_DEBUG_L(FRMT, ...) {FILE *f = fopen("/tmp/gallery-sharing.send.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__, __VA_ARGS__); fclose(f);}
//*/


GSList* lookup_albums(SharingHTTP *http, gchar *items_url, JsonArray *urls, GSList *albums);

/**
 * SharingPluginInterfaceUpdateOptions:
 * @account: #SharingAccount that will have the updated options
 * @con: a ConIc connection
 * @dead_mans_switch: while in this function, this switch should be set to
 * %FALSE at least every 30 seconds
 * @cb_func: a callback function that is called when the update UI flow
 * is complete
 * @cb_data: data passed to the callback function
 *
 * Update options (for example, albums) set on a #SharingAccount.
 *
 * Returns: #TRUE if start was OK, #FALSE otherwise. The callback is called in
 * the #TRUE case only
 */
gboolean
sharing_plugin_interface_update_options (SharingAccount *account,
                                         ConIcConnection *con,
                                         gboolean *cont,
                                         gboolean *dead_mans_switch,
                                         UpdateOptionsCallback cb_func,
                                         gpointer cb_data)
{
  SharingPluginInterfaceUpdateOptionsResult result;
  
  //fetch list of albums
  SharingHTTP * http = sharing_http_new();
  sharing_http_set_connection(http, con);

  //test if the album exists and we can connect with the api key given
  gchar* api_key = sharing_account_get_param(account,"api_key");
  sharing_http_add_req_header(http, "X-Gallery-Request-Key", api_key);
  g_free(api_key);
  sharing_http_add_req_header(http, "X-Gallery-Request-Method", "get");
    
  gchar* url_base = sharing_account_get_param(account,"url");

  gchar* items_url = g_strconcat(url_base,"/rest/items",NULL);
  gchar* root_url =  g_strconcat(url_base,"/rest/item/1",NULL);

  JsonArray* urls = json_array_new();
  json_array_add_string_element(urls, root_url);
  //scan albums, creating options list
  GSList* list = lookup_albums(http, items_url, urls, NULL);

  json_array_unref(urls);
  g_free(url_base);
  g_free(items_url);
  g_free(root_url);

  if (list == NULL)
    goto out;

  //set options
  sharing_account_set_option_values(account, "album", list);
  result=SHARING_UPDATE_OPTIONS_SUCCESS;
  sharing_service_option_values_free(list);
  
 out:
  sharing_http_unref (http); //safe?
  //call calback to say we're done (why ??)
  cb_func(result, cb_data);
  
  //do something else?
  
  return result == SHARING_UPDATE_OPTIONS_SUCCESS;
}

GSList* lookup_albums(SharingHTTP *http, gchar *items_url, JsonArray *urls, GSList *albums)
{
  JsonGenerator *generator = json_generator_new();
  JsonNode *node = json_node_new(JSON_NODE_ARRAY);
  json_node_set_array(node, urls);
  json_generator_set_root(generator, node);
  gchar* json_urls = json_generator_to_data(generator, NULL);
  gchar* urls_enc = g_uri_escape_string(json_urls, NULL, TRUE);
  gchar* url_param = g_strconcat(items_url, "?type=album&urls=", urls_enc, NULL);

  SharingHTTPRunResponse res;
  res = sharing_http_run (http, url_param);

  json_node_free(node);
  g_object_unref(generator);
  g_free(json_urls);
  g_free(urls_enc);
  g_free(url_param);

  if (res != SHARING_HTTP_RUNRES_SUCCESS) {
    ULOG_ERR_L ("Couldn't get stuff from service\n");
    goto err_out;
  }
  const char* response = sharing_http_get_res_body(http, NULL);
  JsonParser* parser = json_parser_new();
  GError* error = NULL;
  if (!json_parser_load_from_data(parser, response, -1, &error)) {
    ULOG_ERR_L("Json parsing failed: %s", error->message);
    g_error_free(error);
    goto err_out;
  }
  JsonNode* root = json_parser_get_root(parser);
  JsonArray* array = json_node_get_array(root);

  for (int i=0; i<json_array_get_length(array); i++) {
    JsonNode *n = json_array_get_element(array, i);
    JsonObject *obj = json_node_get_object(n);
    JsonObject *entity = json_object_get_object_member(obj, "entity");

    const gchar* name = json_object_get_string_member(entity, "title");
    const gchar* id = json_object_get_string_member(entity, "id");
    const gchar* description = json_object_get_string_member(entity, "description");

    SharingServiceOptionValue *option =
      sharing_service_option_value_new(id, name, description);
    albums=g_slist_append(albums, option);

    JsonArray *members = json_object_get_array_member(obj, "members");
    if (json_array_get_length(members) > 0) {
      albums=lookup_albums(http, items_url, members, albums);
    }
  }
  g_object_unref(parser);

  return albums;
 err_out:
  //result=SHARING_PLUGIN_INTERFACE_UPDATE_OPTIONS_RESULT_ERROR;
  sharing_service_option_values_free(albums);
  return NULL;
}
