Medusa Engine - Copyright (c) 1999 - 2009 Palestar Inc
See license.txt for licensing information.

*** Contents:
* readme.txt - This file that you are currently reading.
* license.txt - The license information for the "Medusa Engine".
* Medusa.sln - Visual Studio Solution to compile the base binaries.
* Audio/ - Audio System abstraction.
* AudioDS/ - DirectSound implementation of the audio system.
* Bin/ - All executables & DLL's are compiled into this directory.
* Debug/ - Debugging support.
* Display/ - Rendering abstraction.
* DisplayD3D/ - Direct3D implementation.
* DisplayGL/ - OpenGL implementation (not completed).
* Docs/ - Documentation.
* Draw/ - Drawing code.
* Factory/ - Object factory system.
* File/ - File access code.
* GCQ/ - GameCQ client-side code.
* GUI3D/ - GUI code.
* Math/ - Vector, Matrix, and Quanterinion classes.
* Movie/ - Movie player abstraction.
* MovieDS/ - DirectShow implementation.
* Network/ - Networking code for sockets, client, and server classes.
* Reflection/ - C++ reflection system.
* Render3D/ - high-level 3D system (i.e. meshes, particle systems)
* Resource/ - Resource/Asset management.
* SelfUpdate/ - Library for self updating applications.
* Standard/ - Common code.
* System/ - Platform abstraction.
* SystemWin/ - Windows implementation.
* ThirdParty/ - ThirdParty libraries & code.
* Tools/ - Game editing tools.
* UnitTest/ - UnitTest console application for testing various systems.
* World/ - World management including client & server implementations.


*** Requirements:
* This engine currently requires Visual Studio 2003 to compile. 

*** Quick Start:

* Open the Medusa.sln found in the root directory, this solution will compile the core medusa files. 
* Open Tools/Tools.sln next and build, this will build the base "Resourcer" tool. 
* Once "Resourcer" is built when you run it for the first time, it will prompt you for the "Port" files. "Ports" are short for importer
exporter. 