Gallery3 Sharing Plugin
=======================

A [Maemo](http://maemo.org/) libsharing plug-in for sharing photos to a [Gallery3](http://galleryproject.org/) server using Gallery3's REST API. Implements all the necessary parts of the sharing API to allow setting up an account and then photos can be uploaded from within the image viewer.

Account Setup
=============

To setup a new account, enter Settings then Sharing Accounts. Then press New and select Gallery3 for the account type. Then you'll be prompted for:
 * Your username
 * The server's address
 * Your user's REST API key
Then it will verify the information by testing its ability to connect and you're ready to go.

Usage
=====

After you account is setup, select a photo to share and hit the share button select "Share via Service" and choose the Gallery3 service. Then you can provide a title or description if you want and choose other options like which album it's uploaded to. Once you press Share it will be uploaded in the background and the sharing icon in the task are will reflect its status (but that's mostly the libsharing internals, this plug-in only does the actual transfer.)
