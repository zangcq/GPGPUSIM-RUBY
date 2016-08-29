# GPGPUSIM-RUBY

This is the source code for GPGPU-Sim v3.1.2 with Ruby memory system which was used in the HPCA 2013 work "Cache Coherence for GPU Architectures" by Singh et al.


*****************
 BUILDING
*****************

The process for compiling this version is identical to the process of compiling regular GPGPU-Sim. Read the README file for GPGPU-Sim v3.1.2 (file called README in the same directory as this file) for instructions on how to compile and run GPGPU-Sim. By default, the makefile builds the following Ruby protocols (see the HPCA 2013 paper for more details on these protocols).

non_coherent_g          - No coherence. Also use for No L1s
MESI_CMP_directory_g    - MESI
VI_wt_g                 - GPU-VI, inclusive protocol
VI_wt_ni_g              - GPU-VIni, non-inclusive protocol
ideal_VI_wt_g           - Idealized write-through protocol
cachelease3_g           - TC-Weak
cachelease2_g           - TC-Strong
cachelease2uo_g         - TC-Strong without optimizations proposed in HPCA 2013 paper


*****************
 RUNNING
*****************

The process for running this version is almost identical to the process for running regular GPGPU-Sim, so first read the README file. This version requires pointing the runtime library path to the version of Ruby protocol that should be used. Run the following commands in a bash shell to do that.


export RUBY_PROTOCOL_NAME=non_coherent_g
export LD_LIBRARY_PATH=$GPGPUSIM_ROOT/lib/$GPGPUSIM_CONFIG/ruby/$RUBY_PROTOCOL_NAME/:$LD_LIBRARY_PATH


This above command specifies that the "No coherence" protocol should be used. Modify RUBY_PROTOCOL_NAME with whichever protocol you wish to simulate and run the above commands before running GPGPU-Sim.

The "configs" directory includes configuration files for some of the protocols evaluated in the HPCA 2013 paper.
This version has only been tested with CUDA Toolkit v4.0.


------------------------------------------------------------------------------------------------------------

Welcome to GPGPU-Sim, a cycle-level simulator modeling contemporary graphics
processing units (GPUs) running GPU computing workloads written in CUDA or
OpenCL.  

This version of GPGPU-Sim has been tested with CUDA version 2.3, 3.1 and 4.0.

Please see the copyright notice in the file COPYRIGHT distributed with this
release in the same directory as this file.

If you use this simulator in your research please cite:

Ali Bakhoda, George Yuan, Wilson W. L. Fung, Henry Wong, Tor M. Aamodt,
Analyzing CUDA Workloads Using a Detailed GPU Simulator, in IEEE International
Symposium on Performance Analysis of Systems and Software (ISPASS), Boston, MA,
April 19-21, 2009.

This file contains instructions on installing, building and running GPGPU-Sim.
Detailed documentation on what GPGPU-Sim models, how to configure it, and a
guide to the source code can be found here: <http://gpgpu-sim.org/manual/>. 
Instructions for building doxygen source code documentation are included below.

If you have questions, please sign up for the google groups page (see
gpgpu-sim.org), but note that use of this simulator does not imply any level of
support.  Questions answered on a best effort basis.

To submit a bug report, go here: http://www.gpgpu-sim.org/bugs/

See Section 2 "INSTALLING, BUILDING and RUNNING GPGPU-Sim" below to get started.


1. CONTRIBUTIONS and HISTORY

GPGPU-Sim was created by Tor Aamodt's research group at the University of
British Columbia.  Many students have contributed including: Wilson W.L. Fung,
Ali Bakhoda, George Yuan, Ivan Sham, Henry Wong, Henry Tran, Andrew Turner,
Aaron Ariel, Inderpret Singh, Tim Rogers, Jimmy Kwa, Andrew Boktor, Ayub Gubran
Tayler Hetherington and others.

GPGPU-Sim models the features of a modern graphics processor that are relevant
to non-graphics applications.  The first version of GPGPU-Sim was used in a
MICRO'07 paper and follow-on ACM TACO paper on dynamic warp formation. That
version of GPGPU-Sim used the SimpleScalar PISA instruction set for functional
simulation, and various configuration files to provide a programming model
close to CUDA.  Creating benchmarks for the original GPGPU-Sim simulator was a
very time consuming process.  This motivated the development an interface for
directly running CUDA applications to leverage the growing number of
applications being developed to use CUDA.  We subsequently added support for
OpenCL and removed all SimpleScalar code.

The interconnection network is simulated using the booksim simulator developed
by Bill Dally's research group at Stanford.

To produce output that is compatible with the output from running the same CUDA
program on the GPU, we have implemented several PTX instructions using the CUDA
Math library (part of the CUDA toolkit). Code to interface with the CUDA Math
library is contained in cuda-math.h, which also includes several structures
derived from vector_types.h (one of the CUDA header files).

See file CHANGES for updates in this and earlier versions.


2. INSTALLING, BUILDING and RUNNING GPGPU-Sim

Assuming all dependencies required by GPGPU-Sim are installed on your system,
to build GPGPU-Sim all you need to do is add the following line to your
~/.bashrc file (assuming the CUDA Toolkit was installed in /usr/local/cuda):

	export CUDA_INSTALL_PATH=/usr/local/cuda

then type

	source ~/.bashrc
	source setup_environment
	make

If the above fails, see "Step 1" and "Step 2" below. 

If the above worked, see "Step 3" below, which explains how to run a CUDA
benchmark on GPGPU-Sim.

Step 1: Dependencies
====================

GPGPU-Sim was developed on SUSE  Linux (this release was tested with SUSE
version 11.3) and has been used on several other Linux platforms (both 32-bit
and 64-bit systems).  In principle, GPGPU-Sim should work with any linux
distribution as long as the following software dependencies are satisfied.

Download and install the CUDA Toolkit. It is recommended to use version 3.1 for
normal PTX simulation and version 4.0 for cuobjdump support and/or to use
PTXPlus (Harware instruction set support). Note that it is possible to have
multiple versions of the CUDA toolkit installed on a single system -- just
install them in different directories and set your CUDA_INSTALL_PATH
environment variable to point to the version you want to use.

[Optional] If you want to run OpenCL on the simulator, download and install
NVIDIA's OpenCL driver from <http://developer.nvidia.com/opencl>.  Update your
PATH and LD_LIBRARY_PATH as indicated by the NVIDIA install scripts. Note that
you will need to use the lib64 directory if you are using a 64bit machine.  We
have tested OpenCL on GPGPU-Sim using NVIDIA driver version 256.40
<http://developer.download.nvidia.com/compute/cuda/3_1/drivers/devdriver_3.1_linux_64_256.40.run>
Note the most recent version of the NVIDIA driver produces PTX that is
incompatible with this version of GPGPU-Sim.

GPGPU-Sim dependencies:
* gcc
* g++
* make
* makedepend
* xutils
* bison
* flex
* zlib
* CUDA Toolkit
	
GPGPU-Sim documentation dependencies:
* doxygen
* graphvi

AerialVision dependencies:
* python-pmw
* python-ply
* python-numpy
* libpng12-dev
* python-matplotlib

We used gcc/g++ version 4.5.1, bison version 2.4.1, and flex version 2.5.35.

If you are using Ubuntu, the following commands will install all required
dependencies besides the CUDA Toolkit.

GPGPU-Sim dependencies:
"sudo apt-get install build-essential xutils-dev bison zlib1g-dev flex
libglu1-mesa-dev"

GPGPU-Sim documentation dependencies:
"sudo apt-get install doxygen graphviz"

AerialVision dependencies:
"sudo apt-get install python-pmw python-ply python-numpy libpng12-dev
python-matplotlib"

CUDA SDK dependencies:
"sudo apt-get install libxi-dev libxmu-dev libglut3-dev"

Finally, ensure CUDA_INSTALL_PATH is set to the location where you installed
the CUDA Toolkit (e.g., /usr/local/cuda) and that $CUDA_INSTALL_PATH/bin is in
your PATH.  You probably want to modify your .bashrc file to incude the
following (this assumes the CUDA Toolkit was installed in /usr/local/cuda):

	export CUDA_INSTALL_PATH=/usr/local/cuda
	export PATH=$CUDA_INSTALL_PATH/bin


Step 2: Build
=============

To build the simulator, you first need to configure how you want it to be
built. From the root directory of the simulator, do the following:

cd v3.x
source setup_environment <build_type>

replace <build_type> with debug or release. Use release if you need faster
simulation and debug if you need to run the simulator in gdb. If nothing is
specified, release will be used by default.  

Now you are ready to build the simulator, just run

make

After make is done, the simulator would be ready to use. To clean the build,
run

make clean

To build the doxygen generated documentations, run

make docs

to clean the docs run

make cleandocs

The documentation resides at v3.x/doc/doxygen/html.

Step 3: Run
============

Copy the contents of v3.x/configs/QuadroFX5800/ or v3.x/configs/Fermi/ to your
application's working directory.  These files configure the microarchitecture
models to resemble the respective GPGPU architectures.

To use ptxplus (native ISA) change the following options in the configuration
file to "1" (Note: you need CUDA version 4.0) as follows:

-gpgpu_ptx_use_cuobjdump 1
-gpgpu_ptx_convert_to_ptxplus 1

Now To run a CUDA application on the simulator, simply execute

source setup_environment <build_type>

Use the same <build_type> you used while building the simulator. Then just
launch the executable as you would if it was to run on the hardware. By
running "source setup_environment <build_type>" you change your LD_LIBRARY_PATH
to point to GPGPU-Sim's instead of CUDA or OpenCL runtime so that you do NOT
need to re-compile your application simply to run it on GPGPU-Sim.

To revert back to running on the hardware, remove GPGPU-Sim from your
LD_LIBRARY_PATH environment variable.

Running OpenCL applications is identical to running CUDA applications. However,
OpenCL applications need to communicate with the NVIDIA driver in order to
build OpenCL at runtime. GPGPU-Sim supports offloading this compilation to a
remote machine. The hostname of this machine can be specified using the
environment variable OPENCL_REMOTE_GPU_HOST. This variable should also be set
through the setup_environment script. If you are offloading to a remote machine,
you might want to setup passwordless ssh login to that machine in order to
avoid having too retype your password for every execution of an OpenCL
application.

If you need to run the set of applications in the NVIDIA CUDA SDK code
samples then you will need to download, install and build the SDK.

The CUDA applications from the ISPASS 2009 paper mentioned above are
distributed separately on the git server under the directory
ispass2009-benchmarks. The README.ISPASS-2009 file distributed with the
benchmarks now contains updated instructions for running the benchmarks on
GPGPU-Sim v3.x.


3.  (OPTIONAL) Updating GPGPU-Sim (ADVANCED USERS ONLY)

If you have made modifications to the simulator and wish to incorporate new
features/bugfixes from subsequent releases the following instructions may help.
They are meant only as a starting point and only recommended for users
comfortable with using source control who have experience modifying and
debugging GPGPU-Sim.

WARNING: Before following the procedure below, back up your modifications to
GPGPU-Sim. The following procedure may cause you to lose all your changes.  In
general, merging code changes can require manual intervention and even in the
case where a merge proceeds automatically it may introduce errors.  If many
edits have been made the merge process can be a painful manual process.  Hence,
you will almost certainly want to have a copy of your code as it existed before
you followed the procedure below in case you need to start over again.  You
will need to consult the documentation for git in addition to these
instructions in the case of any complications.

STOP.  BACK UP YOUR CHANGES BEFORE PROCEEDING. YOU HAVE BEEN WARNED. TWICE.

To update GPGPU-Sim you need git to be installed on your system.  Below we
assume that you ran the following command to get the source code of GPGPU-Sim:

git clone git://dev.ece.ubc.ca/gpgpu-sim

Since running the above command you have made local changes and we have
published changes to GPGPU-Sim on the above git server. You have looked at the
changes we made, looking at both the new CHANGES file and probably even the
source code differences.  You decide you want to incorporate our changes into
your modified version of GPGPU-Sim.  

Before updating your source code, we recommend you remove any object files:

make clean

Then, run the following command in the root directory of GPGPU-Sim:

git pull

While git is pulling the latest changes, conflicts might arise due to changes
that you made that conflict with the latest updates. In this case, you need to
resolved those conflicts manually. You can either edit the conflicting files
directly using your favorite text editor, or you can use the following command
to open a graphical merge tool to do the merge:

git mergetool

Now you should test that the merged version "works".  This means following the
steps for building GPGPU-Sim in the *new* README file (not this version) since
they may have changed. Assuming the code compiles without errors/warnings the
next step is to do some regression testing.  At UBC we have an extensive set of
regression tests we run against our internal development branch when we make
changes.  In the future we may make this set of regression tests publically
available. For now, you will want to compile the merged code and re-run all of
the applications you care about (implying these applications worked for you
before you did the merge). You want to do this before making further changes to
identify any compile time or runtime errors that occur due to the code merging
process. 
