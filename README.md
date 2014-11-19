#Dropbox-C

##Introdution

This repository contains a C language library, under [MIT License](https://github.com/Dwii/Dropbox-C/blob/master/LICENSE), that provides access to the [Dropbox Core API](https://www.dropbox.com/developers/core/docs).

##Requirements

* [cURL library](http://curl.haxx.se/libcurl/)
* [OAuth library](http://liboauth.sourceforge.net/)
* [Jansson library](http://www.digip.org/jansson/)

##Features and examples
Almost all [Dropbox Core API](https://www.dropbox.com/developers/core/docs) methods are available with their arguments. Their JSON answers are translated into C structures.

An example is provided [here](https://github.com/Dwii/Dropbox-C/blob/master/Dropbox/example/example.c). But to briefly illustrate its usage, here is what a call to the [file_put method](https://www.dropbox.com/developers/core/docs#files_put) looks like:

```c
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
