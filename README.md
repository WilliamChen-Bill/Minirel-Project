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

# Project: Minirel HeapFile Manager

## Overview
The Minirel HeapFile Manager is a robust database file manager system developed as part of the CS 564 curriculum. This system is designed to manage Heap Files, providing a scanning mechanism that facilitates searching heap files for records adhering to a specified filter or search predicate. The distinct feature of Heap Files in comparison to DB layer files is their logical ordering on pages via a linked list, as opposed to the DB layer's physical ordering.

## Key Components

### 1. FileHdrPage Class
The FileHdrPage class is responsible for implementing a heap file using a linked list of pages. Each heap file comprises one instance of the FileHdrPage class and one or more data pages. This class also includes two critical functions, `createHeapFile()` and `destroyHeapFile()`, which create an empty heap file and delete a heap file, respectively.

### 2. HeapFile Class
The HeapFile class provides a mechanism for managing heap files, including the ability to add and delete records. The class also enables scanning all records in a file. A HeapFile class instance loads the heap file, reads the file header page, and the first data page into the buffer pool.

### 3. HeapFileScan Class
The HeapFileScan class, derived from the HeapFile class, provides the functionality to retrieve all records from a HeapFile, retrieve records matching a specific predicate, and delete records in a file. This class can have multiple instances simultaneously operating on the same file.

### 4. InsertFileScan Class
The InsertFileScan class, derived from the HeapFile class, offers the capability to insert records into a file.

## Conclusion
The Minirel HeapFile Manager is a comprehensive project that showcases proficiency in file management systems. It is designed to simulate real-world scenarios and challenges, making it an excellent portfolio piece for potential employers or collaborators. If you're interested in database management, file manipulation, or just want to appreciate a well-crafted project, take a closer look at the code!
