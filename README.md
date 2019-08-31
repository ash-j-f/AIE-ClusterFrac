![ClusterFrac Logo](ClusterFracIcon_smaller.png)

# ClusterFrac C++ Distributed Computing Library

Copyright (c) Ashley Flynn - Academy of Interactive Entertainment - 2018 - https://ajflynn.io/

Released under the GPL v3 license - See the LICENSE file for more details


## Overview

ClusterFrac is a C++ library as a static LIB and a dynamic DLL that facilitates the distributed computing of parallelisable tasks. The module allows any parallelisable task to be broken down into discreet work packages which are then processed by multiple CPU cores and shared among multiple computers on a network.

ClusterFrac is for use with the Windows operating system and relies on standard libraries included with Visual Studio 2017.
	
This project demonstrates the following complex systems:
	
* Cluster computing including the parallelisation and distribution of tasks.
* Multithreaded processing.
* Thread-safe objects utilising mutexes.
* Exception handling in a multithreaded environment.
* Networking.
* Data compression.
* The mathematics of the Mandelbrot set, comprised of complex numbers which involve real and imaginary components https://en.wikipedia.org/wiki/Mandelbrot_set
	
This project also includes:
	
* Library provided as LIB and DLL.
* User interaction in Benchmark and CFMandelbrot.
* Saving and loading of data between sessions, with data serialised in binary format and stored on disk.
	
	
## Files

### Project Brief

/ClusterFrac Project Brief.pdf

### ClusterFrac Library
	
/DISTRIBUTABLE/ClusterFrac Library Distributable/
	
The ClusterFrac library in DLL and LIB formats, along with header files and other depndencies.
		
### Benchmark
	
/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Host - Benchmark/
	
A benchmarking host application that implements the ClusterFrac Host and tests all functionality by distributing 100 CPU-intensive test calculations among all connected clients.
		
Follow prompts in the console output for user interaction keys.
		
Statistical data about previous benchmark test runs is saved and loaded each session.
		
### CFMandelbrot
	
/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Host - CFMandelbrot/
		
A test application that implements the ClusterFrac Host and renders the Mandelbrot set to screen, allowing the user to navigate the set in real time. This demonstrates the functionality by dividing hundreds of thousdands of calculations among all connected clients each frame.
		
Follow prompts on the graphical display for user interaction keys. Esc to quit.
		
Zoom and pan position data is saved and loaded each session.
		
### ClusterFrac Client
	
/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Client/
		
The standard client implementation which demonstrates the ClusterFrac Client functionality. The client can connect to a ClusterFrac host such as those demonstrated in the Benchmark and CFMandelbrot applications.
		

## Command Line Options

The host and client applications have command line options as follows.
	
### HOST
	
This applies to Benchmark.exe and CFMandelbrot.exe.
	
```MyApplication.exe (Port Number) (Concurrency) (Compression on/off)```
		
* Port Number - A port number in the range 1025 - 65535. Default is 5000.
			
* Concurrency - Number of threads to use for host based calculations. Default is to use max threads available.
			
* Compression on/off - Set to compression_on to enable network compression. Default is compression_off.
			
Eg: Benchmark.exe 5000 4
			
### CLIENT
	
This aplies to Client.exe.
		
```Client.exe (Host IP Address) (Port Number) (Concurrency) (Compression on/off)```
		
* Host IP Address - The IP address of the host in IPv4 format like 10.10.0.126.
		
* Port Number - A port number in the range 1025 - 65535. Default is 5000.
			
* Concurrency - Number of threads to use for client based calculations. Default is to use max threads available.
			
* Compression on/off - Set to compression_on to enable network compression. Default is compression_off.
	
Eg: Client.exe 10.10.0.126 5000 2
		
	
## How to use ClusterFrac

ClusterFrac relies on SFML to provide the networking layer, and ZLIB to provide data compression. These libraries may need to be included in your application if you extend the functionality of ClusterFrac classes. Copies of these libraries can be found in /DISTRIBUTABLE/ClusterFrac Library Distributable/
	
An application using ClusterFrac would typically implement the ClusterFrac Host class, which manages distribution of tasks to all connected clients.
	
See the Benchmark and CFMandelbrot applications for examples of using the Host class.
	
Each client node will need to run an executable implementing the Client class. See the ClusterFrac Client application for an example of using the Client class.
	
Tasks are defined in custom classes which extend the ClusterFrac Task and Result base classes. Tasks must be parallelisable and divisible into sub components to be suitable for cluster computing. See MandelbrotTask, MandelbrotResult, BenchmarkTask and BenchmarkResult classes in the example host applications.
	
The Client must be compiled with your custom Task and Result classes for it to be able to accept and process custom tasks from a host.
	
Custom classes must be registered on the Host and Client using the registerTaskType and registerResultType functions that are members of the Host and Client classes.
	
During execution, ClusterFrac will write status and error information to the console.
