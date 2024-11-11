#include <stdio.h>

#include <functional>
#include <iostream>

// Define two simple functions with the same signature
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int main() {
// Declare a function pointer with the same signature as the functions
    int (*operation)(int, int);

    // Assign the function `add` to the function pointer
    operation = add;
    printf("Add: %d\n", operation(3, 4));  // Output: Add: 7

    // Reassign the function pointer to point to `multiply`
    operation = multiply;
    printf("Multiply: %d\n", operation(3, 4));  // Output: Multiply: 12

    // Use std::function to store a function with the signature int(int, int)
    std::function<int(int, int)> func = multiply;

    // Call through std::function
    std::cout << "Result: " << func(3, 4) << std::endl;  // Output: Result: 12
    return 0;
}
