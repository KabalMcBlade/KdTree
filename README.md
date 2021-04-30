# KdTree

A K-d Tree with a fast search range implementation

This implementation is aimed into the searching domain, focus more on search than insert.
For instance can insert 1 million of elements in less than 20 seconds (in release) and can search points around a coordinated (search range), defined by a unit lenght (for instance 8 meters by default test) across such 1 million of elements in less than 0.02 ms! (in release)

It is a simple header files since the KdTree implementation is a template implementation in less of 350 lines of code.
You can define dimension of the tree, 2d space (x, y) or 3d space (x, y, z) and you can define the type (any integral and floating types) and a payload to be used to store the element.


### Note

The project needs c++17 but is not mandatory foir the KdTree.h file which is the K-d Tree implementation