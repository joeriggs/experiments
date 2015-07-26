Hex/Decimal Math Calculator
===========================

This is a C implementation of a math calculator.  The calculator currently provides a simple text-based user interface.  You can use it as-is, or you can build a more sophisticated user interface to sit on top of it.

Test Platforms
--------------

The calculator has been tested on the following platforms:
* CentOS 5.6 (32-bit) - gcc version 4.1.2
* CentOS 6.6 (32-bit) - gcc version 4.4.7
* CentOS 7.1 (64-bit) - gcc version 4.8.3

Class Hierarchy
---------------

The class hierarchy looks like this:

`   -----------------------------------------------------------------------`
`  |                                  ui()                                 |`
`   -----------------------------------------------------------------------`
`   -------------  --------------------------------------------------------`
`  |  raw_stdin  ||                       calculator                       |`
`   -------------  --------------------------------------------------------`
`                  --------  ------------------  ------------  ------------`
`                 |  list  ||      operand     ||  operator  ||    stack   |`
`                  --------  ------------------  ------------  ------------`
`                            ------   ---   ---`
`                           |fp_exp| | b | |hex|`
`                            ------  | c |  ---`
`                            --------  d |`
`                           |     bcd    |`
`                            ------------`

* **ui()** provides the text-based user interface.  If you want to replace the text-based user interface with something more sophisticated, then you will want to replace ui() with your own ui().

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

