CLUSTERFRAC DISTRIBUTED COMPUTING LIBRARY

Ashley Flynn - Academy of Interactive Entertainment - 2018


*** This file is vest viewed with Notepad++ or any viewer with text auto-wrap.


OVERVIEW

	ClusterFrac is a C++ library that facilitates the distributed computing of parallelisable tasks. The module allows any parallelisable task to be broken down into discreet work packages which can then be shared among multiple computers on a network.

	ClusterFrac is for use with the Windows operating system and relies on standard libraries included with Visual Studio 2017.
	

FILES

	ClusterFrac Library
	
		/DISTRIBUTABLE/ClusterFrac Library Distributable/
	
		The ClusterFrac library in DLL and LIB formats, along with header files and other depndencies.
		
	Benchmark
	
		/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Host - Benchmark/
	
		A benchmarking host application that implements the ClusterFrac Host and tests all functionality by distributing 100 CPU-intensive test calculations among all connected clients.
		
	CFMandelbrot
	
		/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Host - CFMandelbrot/
		
		A test application that implements the ClusterFrac Host and renders the Mandelbrot set to screen, allowing the user to navigate the set in real time. This demonstrates the functionality by dividing hundreds of thousdands of calculations among all connected clients each frame.
		
	ClusterFrac Client
	
		/DISTRIBUTABLE/(DEMOS - Static or Dynamic)/ClusterFrac Client/
		
		The standard client implementation which demonstrates the ClusterFrac Client functionality. The client can connect to a ClusterFrac host such as those demonstrated in the Benchmark and CFMandelbrot applications.
		

COMMAND LINE OPTIONS

	The host and client applications have command line options as follows.
	
	HOST
	
		This applies to Benchmark.exe and CFMandelbrot.exe.
	
		MyApplication.exe (Port Number) (Concurrency) (Compression on/off)
		
			Port Number - A port number in the range 1025 - 65535. Default is 5000.
			
			Concurrency - Number of threads to use for host based calculations. Default is to use max threads available.
			
			Compression on/off - Set to compression_on to enable network compression. Default is compression_off.
			
		Eg: Benchmark.exe 5000 4
			
	CLIENT
	
		This aplies to Client.exe.
		
		Client.exe (Host IP Address) (Port Number) (Concurrency) (Compression on/off)
		
			Host IP Address - The IP address of the host in IPv4 format like 10.10.0.126.
		
			Port Number - A port number in the range 1025 - 65535. Default is 5000.
			
			Concurrency - Number of threads to use for host based calculations. Default is to use max threads available.
			
			Compression on/off - Set to compression_on to enable network compression. Default is compression_off.
	
		Eg: Client.exe 10.10.0.126 5000 2