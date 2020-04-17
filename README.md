# GStreams
A stream library for C++. It uses a stream protocol, that is driven by generators. That means that a generator pushes the data down the pipeline.

## Fast forward documentation
A Stream Pipeline consists of at least a generator and an acceptor:

	generator | acceptor;

In between processors can be inserted, that modify the stream:

	generator | processor0 | processor1 | acceptor;

There are 3 CRTP-Class Templates that classify the kind of stream-node:


	GSGenerator<GeneratorType>;
	GSProcessor<ProcessorType>;
	GSAcceptor<AcceptorType>;
	
	
All three subclasses can be combined with `operator|`, (Generator, Acceptor and Processor are for exposition only):

	Generator | Acceptor  -> void		(1)
	Generator | Processor -> Generator	(2)
	Processor | Processor -> Processor  (3)
	Processor | Acceptor  -> Acceptor   (4)
	
(1) The generator is executed, so all data is streamed to the acceptor, this has *side effects* on the acceptor.
(2) Creates a generator, that generates the elements as the left generator and then applies the transformations of the right processor
(3) Creates a processor, that first applies the transformations of the left processor and then the transformations of the right processor
(4) Creates an acceptor, that first applies the transformations of the left processor and then pushes the data to the right acceptor

By this rules the following expressions are semantically equivalent:

	(generator | processor) | acceptor
	 generator | (processor | acceptor)

### Utility functions
There are some utility functions that help creating and managing GStreams.


	gsGenerate(<Function> function) -> Generator                       (1)
	gsYieldFrom(<Container> const& container) -> Generator             (2)
	gsYieldFromCopy(<Container> container) -> Generator                (3)
	gsYieldFrom(<Iterator> begin, <Iterator> end) -> Generator         (4)

	gsProcess(<Function> function) -> Processor                        (5)
	gsMap(<Function> function) -> Processor                            (6)
	gsFilter(<Function> function) -> Processor                         (7)
	
	gsAccept(<Function> function) -> Acceptor                          (8)
	gsInsertBack(<Container>& container) -> Acceptor                   (9)


(1) When streaming: Calls function with the parameter yield, that accepts a value as its input
(2) When streaming: Yields each element from a container, keeps a reference to the container
(3) When streaming: Yields each element from a container, the container gets copied/moved
(3) When streaming: Yields each element in the Range [begin, end)
	
(5) Creates a Processor, that applies function to each element with a function yield, that passes the element further into the pipeline; `function(value, yield);`
(6) Creates a Processor, that maps `function` to each streamed element; `yield(function(value));`
(7) Creates a Processor, that passes each streamed element, only if `function` returns true; `if(function(value)) yield(value);`
	
(8) Creates an acceptor, that calls `function` with each element; `function(value);`
(9) Creates an acceptor, that inserts elements to the end of container with `container.push_back(value);`

	
	
#### Tip
If you need to keep a reference to a function, you could use `std::ref`; otherwise it is always copied/moved, when `operator|` is used
	






/***
### Custom behavior
If you need custom behavior, you can always implement the nodes yourself:

#### Generator
A generator is implemented as follows:

```cpp
class MyGenerator : public GSGenerator<MyGenerator>
{
public:
		template<typename Acceptor>
	void generate(Acceptor& acceptor);
};
```
	
#### Acceptor
An acceptor is implemented as follows:

```cpp
class MyAcceptor : public GSAcceptor<MyAcceptor>
{
public:
		template<typename T>
	void accept(T&& value);
}
```

#### Processor
A processor is implemented as follows:

```cpp
class MyProcessor : public GSProcessor<MyProcessor>
{
public:
		template<typename T, typename Acceptor>
	void process(T&& value, Acceptor& acceptor);
}
```

***/