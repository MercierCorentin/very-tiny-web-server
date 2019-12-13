
# Very tiny Web Server

## Introduction

A really lightweight web server for static HTML/CSS/JS pages. This web server was a school project developped in TCP/IP Networkprogramming class at the Jade Hochschule, Wilhelmshaven, Lower-Saxony, Germany. It accepts only GET requests.

The creation of this web server allowed me to work from the transport layer of the OSI-model to the application layer. It helps to have a better understading of all these layers. It was a really nice experience.

## Compilation and installation

```bash
    gcc -o main functions.c main.c
```

If you have a web server such as Apache or Nginx running on your device, stop the corresponding service. And clear port 80. Then you can run the web server with super-admin privileges by simply executing the compilation's output.
