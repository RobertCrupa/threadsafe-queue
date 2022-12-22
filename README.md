# Thread-Safe Queue

For a detailed explanation on how the queue works [see my blog post](https://techventure.tech/how-to-build-a-thread-safe-queue-using-c/). 

## Building

### Preliminary 

To start, the repository has to be cloned:

```bash
$ git clone https://github.com/RobertCrupa/threadsafe-queue.git
$ cd threadsafe-queue
```

### Building and running the test cases

To create build folder and run tests:

```bash
$ cmake -S . -B build
$ cmake --build build
$ cd build && ./queue_test
```
