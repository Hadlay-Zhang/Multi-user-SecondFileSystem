# Multi-User-SecondFileSystem

![](https://img.shields.io/badge/Source-TJCourse 10043602-red) ![GitHub repo size](https://img.shields.io/github/repo-size/Hadley-Zhang/Multi-user-SecondFileSystem)  ![GitHub stars](https://img.shields.io/github/stars/Hadley-Zhang/Multi-user-SecondFileSystem?color=yellow)  ![GitHub forks](https://img.shields.io/github/forks/Hadley-Zhang/Multi-user-SecondFileSystem?color=green&label=Fork) 



This repo is a Multi-user Second File System for CS 10043602, Tongji University based on following techniques:

- UNIX V6++ operating system by Department of Computer Science and Technology, Tongji University

* linux socket
* pthread

## Description

- include: .h files
- obj: .o files
- src: .cpp codes
- client: client codes
- cpout.txt, Mina.jpg: test examples for API **cpin, cpout**

## Environment

- Ubuntu 22.04
- gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0


## Usage

- To start file system(Server):

  cd to ./SecondFileSystem

```shell
make clean
make all

./MyFileSystem
```

- To start a new Client:

  cd to ./SecondFileSystem/client

```shell
make clean
make all

./client localhost 1234
```

## API instructions

* open a file:

  **open [filename] [mode]**

* close the file:

  **close [fd]**

* read a file:

  **read [fd] [length]**

* write to a file:

  **write [fd] [contents]**

* move file pointer:

  **seek [fd] [offset] [whence]**

* create a new file:

  **touch [filename] [mode=0777]**

* delete a file or directory:

  **rm [filename]**

* show files and directories in current working path:

  **ls**

* create a new directory:

  **mkdir [dirname]**

* change current working path:

  **cd [dirname]**

* show the contents of a file:

  **cat [filename]**

* copy an outer file to current path:

  **cpin [out_filename] [filename]**

* copy a file in current path outwards:

  **cpout [filename] [out_filename]**

* see instructions for all APIs:

  **help [funcname]**

* quit: 

  **exit**

## Reference

[fffqh/MultiUser-secondFileSystem: 同济大学操作系统课程设计：多用户二级文件系统](https://github.com/fffqh/MultiUser-secondFileSystem)

[CindyAndRick/SecondFileSystem: 同济大学操作系统课设 多用户二级文件系统](https://github.com/CindyAndRick/SecondFileSystem)