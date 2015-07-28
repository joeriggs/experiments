Hex/Decimal Math Calculator
===========================

This is a C implementation of a math calculator.  The calculator currently provides a simple text-based user interface.  You can use it as-is, or you can build a more sophisticated user interface to sit on top of it.

Test Platforms
--------------

The calculator has been tested on the following platforms:
* CentOS 5.6 (32-bit) - gcc version 4.1.2
* CentOS 6.6 (32-bit) - gcc version 4.4.7
* CentOS 7.1 (64-bit) - gcc version 4.8.3

How to Build
------------

If you want to build the calculator, simply type "make".  This will compile all of the source modules and produce a binary named **calculator**.

There are 3 switches that you can add to the "make" command.  They are:

* **TEST=1** - This switch will direct the Makefile to create a test program named **test**.  The test program will run through all of the unit tests that are contained at the bottom of each source code file.  Each file contains a function called **module**_test(), where **module** is the name of the source file.  For example, calculator.c contains a function called **calculator_test()**.  If you run "make TEST=1", you will run all of the tests.  The program is designed to exit immediately if one of the tests fails.  It will then exit with a return code of 1.  A successful test run will exit with a return code of 0.

* **DEBUG=1** - This switch will direct the Makefile to create a **calculator** or **test** program with internal debug turned on.  This will buy you 2 things:

  * It will build with optimization turned off and debug symbols turned on.  This allows you to debug the program with gdb.

  * If will turn on console debug info.  There is a lot of console debug information in the source code files.  It is useful for debugging problems.  Note that bcd.c contains per-method debug info that is controlled via a series of **BCD_DBG_...** switches at the top of the module.

* **clean** - Run this command if you want to clean up after a build.  This is particularly useful if you want to switch back and forth between running the real **calculator** program and running the **test** program.  For example:

  * To switch from **calculator** to **test**, you run "make clean" followed by "make TEST=1".

  * To switch from **test** to **calculator**, you run "make TEST=1 clean" followed by "make".

Class Hierarchy
---------------

The class hierarchy looks like this:

* **ui** provides the text-based user interface.  If you want to replace the text-based user interface with something more sophisticated, then you will want to replace ui() with your own ui().

* **raw_stdin** provides an interface between ui() and the console device, allowing the user to have a better interactive interface.  If you want to replace the text-based user interface with something more sophisticated, then you can remove this class.

* **calculator** is the engine.  It parses the user input and drives all
    calculator operation.

  * **list** is a doubly-linked list.  It is used to store the user-input infix equation and is also used to store the internally-generated postfix equation while it is being created and prior to being executed.

  * **operand** is used to store each operand.  It contains members that know how to manipulate the numeric operands.

    * **fp_exp** is used to perform exponent operations.  Decimal floating point exponentiation is fairly complex, so that functionality is encapsulated in a separate class in order to avoid making the BCD class overly complicated.

    * **bcd** provides a binary coded decimal (bcd) data representation that allows the calculator to do decimal math without running into problems caused by IEEE 754 float or double issues.

    * **hex** provides a hexadecimal data representation that allows the calculator to do hex math and bit manipulation operations.

  * **operator** is used to store each operator.  It contains members that know how to execute the steps necessary to perform the operator.  The calculator supports unary and binary operators.

  * **stack** is used when converting the user-input infix equation to postfix, and also while processing the postfix equation.

