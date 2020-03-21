# Fluent Bit Plugin

The following repository provides the structure and build-system helpers to develop C dynamic plugins for [Fluent Bit](https://fluentbit.io) like _inputs_, _filters_ and _outputs_.

>  Fluent Bit API development/usage is out of the scope of this article.

## Requirements

- [Fluent Bit](https://fluentbit.io) Source code, version >= 1.2
- C compiler: GCC or Clang
- CMake3

## Getting Started

In the following steps we will build the example plugin provided called __out_stdout2__. As a first step get into the _build/_ directory:

```bash
$ cd build/
```

Now we will provide CMake (our build system) two important variables:

- FLB\_SOURCE: absolute path to source code of Fluent Bit.
- PLUGIN\_NAME: _directory_ name of the project that we aim to build. Note that any plugin name must have it proper prefix as the example mentioned above.

Assuming that Fluent Bit source code is located at /tmp/fluent-bit and we will build _out\_stdout2_, we will run CMake with the following options:

```bash
$ cmake -DFLB_SOURCE=/tmp/fluent-bit -DPLUGIN_NAME=out_stdout2 ../
```

then type 'make' to build the plugin:

```
$ make
Scanning dependencies of target flb-out_stdout2
[ 50%] Building C object out_stdout2/CMakeFiles/flb-out_stdout2.dir/stdout2.c.o
[100%] Linking C shared library ../flb-out_stdout2.so
[100%] Built target flb-out_stdout2
```

If you query the content of the current directory you will find the new shared library created:

```
$ ls -l *.so
-rwxr-xr-x 1 edsiper edsiper 17288 jun 20 16:33 flb-out_stdout2.so
```

that __.so__ file is our dynamic plugin that now can be loaded from Fluent Bit through the [plugins configuration](https://github.com/fluent/fluent-bit/blob/master/conf/plugins.conf) file.

To load the external plugin add Plugin File in your Service section of your configuration file.

```
[SERVICE]

Plugins_File   plugins.conf
```

Also, add name of external plugin in output or input section of your configuration file. For example :

```
[OUTPUT]

Name   stdout2
```


## License

This program is under the terms of the [Apache License v2.0](http://www.apache.org/licenses/LICENSE-2.0).
