  Node - basic json node, can get type string, int, double, bool, dictionary, array, null. Function Is... checks node type. Function As... checks node type and, if used function with a required type, then return node's value.
  Load - function, which load json file from std::istream.
  Print - function, which print json file. Take Document and std::ostream.
  ParsingError - exception thrown in case of parsing error
