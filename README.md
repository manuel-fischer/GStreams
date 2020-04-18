# GStreams
A stream library for C++. It uses a stream protocol, that is driven by generators. That means that a generator pushes the data down the pipeline.

## Dependencies?
Only the standard library of C++, so this little library can almost be called stand alone. Only utility function templates like `move`, `forward`, `addressof` and `for_each` are used. Note that there is some compatibility with containers like `std::vector` and `std::string`. Eg. these can be used with `gsInsertBack`, which calls `container.push_back`.

## Fast forward documentation
A Stream Pipeline consists of at least two nodes; a generator and an acceptor:

	generator | acceptor;

In between processors can be inserted, that modify the stream:

	generator | processor0 | processor1 | acceptor;

There are 3 CRTP-Class Templates that classify the kind of stream-node:


	GSGenerator<GeneratorType>;
	GSProcessor<ProcessorType>;
	GSAcceptor<AcceptorType>;
	
	
All three subclasses can be combined with `operator|`:

	Generator | Acceptor  -> void       (1)
	Generator | Processor -> Generator  (2)
	Processor | Processor -> Processor  (3)
	Processor | Acceptor  -> Acceptor   (4)
	
1) The generator is executed, so all data is streamed to the acceptor, this has *side effects* on the acceptor.  
2) Creates a generator, that generates the elements as the left generator and then applies the transformations of the right processor  
3) Creates a processor, that first applies the transformations of the left processor and then the transformations of the right processor  
4) Creates an acceptor, that first applies the transformations of the left processor and then pushes the data to the right acceptor  

By this rules the following expressions are semantically equivalent:

	(generator | processor) | acceptor
	 generator | (processor | acceptor)

### Utility functions
There are some utility functions that help creating and managing GStreams.


	gsGenerate(function(yield))
	gsYieldFrom(container)
	gsYieldFromCopy(container)
	gsYieldFrom(container)
	
	gsProcess(function(value, yield))
	gsMap(function(value))
	gsFilter(function(value))

	gsAccept(function(value))
	gsInsertBack(container)

#### Generator

	gsGenerate(<Function> function) -> Generator                (1)
	gsYieldFrom(<Container> const& container) -> Generator      (2)
	gsYieldFromCopy(<Container> container) -> Generator         (3)
	gsYieldFrom(<Iterator> begin, <Iterator> end) -> Generator  (4)

1) When streaming: Calls function with the parameter `yield`, that accepts a value as its input  
2) When streaming: Yields each element from a container, keeps a reference to the container  
3) When streaming: Yields each element from a container, the container gets copied/moved. Use this when you are returning nodes from a function.  
4) When streaming: Yields each element in the Range `[begin, end)`  
	
#### Processor

	gsProcess(<Function> function) -> Processor                 (5)
	gsMap(<Function> function) -> Processor                     (6)
	gsFilter(<Function> function) -> Processor                  (7)
	
5) Creates a Processor, that applies function to each element with a function yield, that passes the element further into the pipeline; `function(value, yield);`  
6) Creates a Processor, that maps `function` to each streamed element; `yield(function(value));`  
7) Creates a Processor, that passes each streamed element, only if `function` returns true; `if(function(value)) yield(value);`  

#### Acceptor

	gsAccept(<Function> function) -> Acceptor                   (8)
	gsInsertBack(<Container>& container) -> Acceptor            (9)
	
8) Creates an acceptor, that calls `function` with each element; `function(value);`  
9) Creates an acceptor, that inserts elements to the end of container with `container.push_back(value);`  


###	gsRef

	gsRef(<Generator> generator) -> Generator
	gsRef(<Processor> processor) -> Processor
	gsRef(<Acceptor> acceptor) -> Acceptor

This is a Helper function that creates a reference wrapper to a stream node. This allows nodes to be used multiple times without copying.

	
#### Tip
If you need to keep a reference to a function, you could use `std::ref`; otherwise it is always copied/moved, when `operator|` is used.
	

## Technical Information
GStreams is heavily typed, it uses generic lambdas under the hood. To enable the user to return stream nodes from a function, merged nodes own subnodes.

By using templates everywhere, elements of different types can be passed around

The Library is not contained in a namespace, but instead each identifier is prefixed with `gs` or `GS`.
	
	
## Examples

### Hello World

This example is the Hello world example to this library.
This example creates a stream acceptor `cat`, that outputs each element to `std::cout`.
Note that the lambda function has an empty capture, this means that copying the node gets optimized to nothing.

```cpp
#include <gstream.hpp>
#include <vector>
#include <iostream>

int main()
{
	auto cat = gsAccept([](auto& value) { std::cout << value; });
	
	std::vector<const char*> strings { "Hello", " ", "World", "\n" };
	gsYieldFrom(strings) | cat;
}
```
	

### Squares

This example outputs the first 10 squares, by using a node that generates numbers, a node that squares numbers and a node that outputs each number.
Then it outputs all the odd ones.

```cpp
#include <gstream.hpp>
#include <iostream>

inline auto make_counter(int first, int last)
{
	return [first, last](auto yield)
	{
		for(int i = first; i < last; ++i) yield(i);
	}
}

int main()
{
	auto numbers = gsGenerate(make_counter(1, 11));
	auto square  = gsMap([](int x) { return x*x; });
	auto output  = gsAccept([](int y) { std::cout << y << '\n'; });
	
	numbers | square | output;
	
	numbers | gsFilter([](int x) { x%2 == 1; }) | square | output;
}
```

	
### Poly-typed

This example show the ability to stream elements of different type, and that the number of elements at the generator node does not need to be equal to the number of elements at the acceptor node.


```cpp
#include <gstream.hpp>
#include <utility> // forward
#include <iostream>

int main()
{
	auto generate = gsGenerate(
		[](yield)
		{
			yield(42);
			yield(3.14);
			yield("Hello");
		}
	);
	
	auto twice = gsMap(
		[]<template Value, template Yield>(Value&& value, Yield yield)
		{
			yield(value);
			yield(std::forward<Value>(value));
		}
	);
	
	auto output = gsAccept(
		[](auto& value)
		{
			std::cout << value << '\n';
		}
	);

	
	generate | twice | output;
}
```
	
	
	
## Possible Future changes
- The merged nodes (returned by the `operator|` overloads) might use references to the nodes instead of owning the nodes.
	- This could introduce a new mechanism to bake streams, that can be returned from functions.
- Improved function signature checking -> better compiler error messages
- `yieldFrom` with `std::tuples` and `std::array`