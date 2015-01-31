/*
 * This file is part of sharing-plugin-template
 *
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

    /* Correct fields must be added to http request before sending */

    //test if the album exists and we can connect with the api key given
    gchar* api_key = sharing_account_get_param(account,"api_key");
    sharing_http_add_req_header(http, "X-Gallery-Request-Key", api_key);
    g_free(api_key);
    sharing_http_add_req_header(http, "X-Gallery-Request-Method", "get");
    
    gchar* host = sharing_account_get_param(account,"host");
    gchar* port = sharing_account_get_param(account,"port");
    gchar* album = sharing_account_get_param(account,"album");

    gchar* url = g_strconcat("http://",host,":",port,"/index.php/rest/items/",album,"?type=album");
    g_free(host);
    g_free(port);
    g_free(album);

    SharingHTTPRunResponse res;
    res = sharing_http_run (http, url);
    g_free(url);
    if (res == SHARING_HTTP_RUNRES_SUCCESS) {
      ULOG_DEBUG_L ("Got response (%d): %s\n", sharing_http_get_res_code (http),
		    sharing_http_get_res_body (http, NULL));
    } else {
      ULOG_ERR_L ("Couldn't get stuff from service\n");
      ret = SHARING_ACCOUNT_VALIDATE_FAILED;
    }
    sharing_http_unref (http); 
    

    return ret;
}

