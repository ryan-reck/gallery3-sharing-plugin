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
#include <sharing-account.h>
#include <sharing-http.h>
#include <osso-log.h>
#include <conicconnection.h>
#include "validate.h"
#include "common.h"


/*/
#ifdef ULOG_DEBUG
#undef ULOG_DEBUG
#endif
#ifdef ULOG_DEBUG_L
#undef ULOG_DEBUG_L
#endif


#define ULOG_DEBUG(FRMT) {FILE *f = fopen("/tmp/gallery-sharing.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__); fclose(f);}
#define ULOG_DEBUG_L(FRMT, ...) {FILE *f = fopen("/tmp/gallery-sharing.log", "a"); fprintf(f, "%s:%d: " FRMT "\n", __FILE__, __LINE__, __VA_ARGS__); fclose(f);}
//*/

/**
 * test:
 * @account: #SharingAccount to be tested
 * @con: Connection used
 * @dead_mans_switch: Turn to %FALSE at least every 30 seconds.
 *
 * Test if #SharingAccount is valid.
 *
 * Returns: #SharingPluginInterfaceTestAccountResult
 */
SharingPluginInterfaceAccountValidateResult validate (SharingAccount* account,
    ConIcConnection* con, gboolean *cont, gboolean* dead_mans_switch)
{
    SharingPluginInterfaceAccountValidateResult ret =
        SHARING_ACCOUNT_VALIDATE_SUCCESS;

    SharingHTTP * http = sharing_http_new ();
    sharing_http_set_connection(http, con);

    //test if the rest api works and we can connect with the api key given
    gchar* api_key = sharing_account_get_param(account,"api_key");
    sharing_http_add_req_header(http, "X-Gallery-Request-Key", api_key);
    g_free(api_key);
    sharing_http_add_req_header(http, "X-Gallery-Request-Method", "get");

    gchar* address = sharing_account_get_param(account,"address");
    gchar* url_base;

    int attempt=0,attempts=2; //a shitty way to rollup the 2/4 attempts in a loop
    char* protocol = "";
    
    if (! g_strrstr(address,"://")) {
      protocol = "http://";
      attempts*=2; //try http and https variants
    }

    do {
      if (attempt == 2) {
        protocol="https://";
      }
      if (attempt & 1) {
        url_base = g_strconcat(protocol,address,NULL);
      }
      else {
        url_base = g_strconcat(protocol,address,"/index.php",NULL);
      }
      gchar* url = g_strconcat(url_base,"/rest/item/1",NULL);
      SharingHTTPRunResponse res = sharing_http_run (http, url);
      g_free(url);
      if (res == SHARING_HTTP_RUNRES_SUCCESS) {
        const char* response = sharing_http_get_res_body(http, NULL);
        if (response[0] == '{') {
          ret = SHARING_ACCOUNT_VALIDATE_SUCCESS;
          //store the url that worked
          sharing_account_set_param(account,"url",url_base);
          g_free(url_base);
          goto cleanup;
        }
      }
      g_free(url_base);
    } while (attempt++ < attempts);

    ULOG_ERR_L ("Couldn't get stuff from service\n");
    ret = SHARING_ACCOUNT_VALIDATE_FAILED;

 cleanup:
    g_free(address);
    sharing_http_unref (http); 

    return ret;
}

