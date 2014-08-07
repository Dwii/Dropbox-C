#Dropbox-C

##Introdution

This repository contains a C language library, under MIT License, that provides access to the Dropbox Core API.

##Requirements

* [cURL library](http://curl.haxx.se/libcurl/)
* [OAuth library](http://liboauth.sourceforge.net/)
* [Jansson library](http://www.digip.org/jansson/)

##Features and examples
Almost all [Dropbox Core API](https://www.dropbox.com/developers/core/docs) methods are available with all their arguments. Their JSON answers are translated to C structures.

A example is provided [here](https://github.com/Dwii/Dropbox-C/blob/master/Dropbox/example/example.c). But to shortly illustrate it's usage, there's what a call to the [file_put method](https://www.dropbox.com/developers/core/docs#files_put) looks like:

```
  FILE *file = fopen("/tmp/hello.txt", "r"); 
  drbPutFile(cli, NULL,
             DRBOPT_ROOT, DRBVAL_ROOT_AUTO,
             DRBOPT_PATH, "/hello.txt",
             DRBOPT_IO_DATA, file,
             DRBOPT_IO_FUNC, fread,
             DRBOPT_END);
  fclose(file);
```

##Known Issues
This library is in BETA and still have some issues in multi-threaded programs.
