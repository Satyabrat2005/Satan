// Import Demo — load code from another file
summon "Import System Demo";

import "examples/math_utils.satan";

summon greet("Satan User");
summon "2 + 3 = " + str(add(2, 3));
summon "5! = " + str(factorial(5));
summon "10! = " + str(factorial(10));
