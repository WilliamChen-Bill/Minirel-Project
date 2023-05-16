# Minirel Database Management System Project

## Project Overview

This project aims to guide you through the intricacies of building a database management system (DBMS). It encompasses the creation of a functional single-user DBMS capable of processing simple SQL queries. The objective is to learn how a DBMS is structured and the operations that occur within it when queries are executed.

The project includes a parser (the topmost layer) and a disk I/O layer (the lowest layer). The parser processes SQL queries and calls the relevant functions in the lower layers to execute relational operations. The disk I/O layer handles the reading and writing of pages from and to the disk, which in this case, is the UNIX file system.

## Project Stage 3 - The Buffer Manager

The third part of the project involves the development of a buffer manager for the DBMS, termed "Minirel". The buffer manager controls which database pages are memory resident at any given time, efficiently managing the limited memory resources available compared to the size of the database on disk.

The buffer manager uses the clock algorithm for buffer replacement, a strategy that approximates the least recently used (LRU) behavior, but with a significantly faster execution speed.

You will also work with three C++ classes: `BufMgr`, `BufDesc`, and `BufHashTbl` to create the structure of the buffer manager.

### BufHashTbl Class

The `BufHashTbl` class is a hash table that maps file and page numbers to buffer pool frames, implemented using chained bucket hashing.

### BufDesc Class

The `BufDesc` class is responsible for keeping track of the state of each frame in the buffer pool, including information about the page, the frame, pin count, and dirty and valid flags.

### BufMgr Class

The `BufMgr` class is the core of the buffer manager. It involves methods such as `allocBuf()`, `readPage()`, `unPinPage()`, `allocPage()`, and `flushFile()` to manage buffer frames and pages.

### Contributing to the Project

This project offers a solid foundation for anyone interested in learning the architecture of a DBMS. It is a great opportunity to not only learn the theoretical aspects but also get hands-on experience in building a database system. Contribute to this project to improve your understanding of DBMS and to make a positive impact on your resume for recruiters.
