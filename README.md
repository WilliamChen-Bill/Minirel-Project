# Minirel Database Management System Project

## Project Overview

In this project series guides you through the complexities of building a functional, single-user database management system (DBMS) capable of processing simple SQL queries. The overarching objective is to provide a comprehensive understanding of DBMS structures and the operations that occur within them during query execution.

### Stage 3 - The Buffer Manager

Here, we lay the foundations of our DBMS with two primary components: the parser and the disk I/O layer. The parser handles SQL queries, invoking relevant functions in lower layers for performing relational operations. The disk I/O layer is responsible for the essential task of reading from and writing to the UNIX file system.

### Stage 4 - Minirel HeapFile Manager

The fourth stage introduces the robust Minirel HeapFile Manager. This stage encapsulates the management of Heap Files, offering a scanning mechanism for searching records based on specified predicates. The distinguishing feature of Heap Files is their logical ordering on pages using a linked list, a contrast to the physical ordering in DB layer files.

### Stage 6 - Minirel Query and Update Operators

In the final stage, we delve into the creation and implementation of query and update operators for Minirel databases. This stage encapsulates the implementation of crucial operators such as selection, projection, insertion, and deletion. A provided parser, integral to this project, parses SQL-like commands and triggers appropriate backend calls.

## Key Components & Features

### Project Stage 3 - The Buffer Manager

#### BufHashTbl Class

The `BufHashTbl` class is a hash table that maps file and page numbers to buffer pool frames, implemented using chained bucket hashing.

#### BufDesc Class

The `BufDesc` class is responsible for keeping track of the state of each frame in the buffer pool, including information about the page, the frame, pin count, and dirty and valid flags.

##### BufMgr Class

The `BufMgr` class is the core of the buffer manager. It involves methods such as `allocBuf()`, `readPage()`, `unPinPage()`, `allocPage()`, and `flushFile()` to manage buffer frames and pages.

### Project Stage 4 - Minirel HeapFile Manager

#### 1. FileHdrPage Class
The FileHdrPage class is responsible for implementing a heap file using a linked list of pages. Each heap file comprises one instance of the FileHdrPage class and one or more data pages. This class also includes two critical functions, `createHeapFile()` and `destroyHeapFile()`, which create an empty heap file and delete a heap file, respectively.

#### 2. HeapFile Class
The HeapFile class provides a mechanism for managing heap files, including the ability to add and delete records. The class also enables scanning all records in a file. A HeapFile class instance loads the heap file, reads the file header page, and the first data page into the buffer pool.

#### 3. HeapFileScan Class
The HeapFileScan class, derived from the HeapFile class, provides the functionality to retrieve all records from a HeapFile, retrieve records matching a specific predicate, and delete records in a file. This class can have multiple instances simultaneously operating on the same file.

#### 4. InsertFileScan Class
The InsertFileScan class, derived from the HeapFile class, offers the capability to insert records into a file.

### Project Stage 6 - Minirel Query and Update Operators

#### Minirel SQL DML Commands

Minirel supports a simplified variant of SQL query language. The syntax structure is outlined in a pseudo context-free grammar, a continuation from project's Part 5. It includes both query and update statements.

#### Query Structure and Formatting

Queries can either be displayed on-screen or stored in a relation. To simplify the on-screen display, the Minirel query interpreter generates a temporary relation which is subsequently printed and deleted. The project also includes specifications for attributes in target lists, target table lists, and qualifications.

#### Update Operations

The project implements update operations in the form of insert or delete actions. The Minirel does not support modification of the value of an existing tuple in a relation.

#### Implementing the Relational Operators

The project involves the implementation of several routines including QU_Select, QU_Join, QU_Delete, and QU_Insert. These are executed by the parser in response to various user-submitted SQL statements.



